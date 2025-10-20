#include "player.hpp"
#include <iostream>
#include <map>


Player::Player(float x, float y, int id, int controllerId, std::shared_ptr<sf::Texture> texture)
    : m_position(x, y)
    , m_velocity(0.f, 0.f)
    , m_isChargingThrow(false)
    , m_id(id)
    , m_isAlive(true)
    , m_deathAnimationComplete(false)
    , m_controllerId(controllerId)
    , m_readyToThrow(false)
    , m_aimDirection(1.f, 0.f)
    , m_aimIndicatorDistance(50.0f)
    , m_state(State::Normal)
    , m_dashDirection(1.f, 0.f)
    , m_dashSpeed(25.f)
    , m_dashDuration(sf::seconds(0.15f))
    , m_dashCooldown(sf::seconds(1.0f))
    , m_texture(texture)
    , m_sprite(*m_texture)
    , m_currentFrame({0, 0}, {24, 24})
    , m_spriteSize(24, 24)
    , m_facingRight(true)
    , m_animationTime(0.0f)
    , m_animationSpeed(0.1f)
    , m_currentAnimation(AnimationState::Idle)
    , m_aimArrowTexture(std::make_shared<sf::Texture>())
{
    // Set hitbox to match sprite size (24x24 sprite * 2.0 scale = 48x48)
    float hitboxWidth = m_spriteSize.x * 2.0f;
    float hitboxHeight = m_spriteSize.y * 2.0f;
    m_shape.setSize({hitboxWidth, hitboxHeight});
    
    if (m_id == 0) m_shape.setFillColor(sf::Color::Green);
    if (m_id == 1) m_shape.setFillColor(sf::Color::Blue);
    if (m_id == 2) m_shape.setFillColor(sf::Color::Magenta);
    if (m_id == 4) m_shape.setFillColor(sf::Color::Red);
    
    // Set origin to center of hitbox
    m_shape.setOrigin({hitboxWidth / 2.0f, hitboxHeight / 2.0f});
    m_shape.setPosition(m_position);
    m_isGrounded = false;

    // Set up sprite properties
    m_sprite.setTextureRect(m_currentFrame);
    m_sprite.setScale({2.0f, 2.0f});
    m_sprite.setOrigin({m_spriteSize.x / 2.0f, m_spriteSize.y / 2.0f});
    m_sprite.setPosition(m_position);

    // Load aim arrow texture
    if (!m_aimArrowTexture->loadFromFile("assets/aim_arrow.png")) {
        std::cerr << "Error: Could not load aim arrow texture!" << std::endl;
    } else {
        // Set up aim arrow sprite
        m_aimArrowSprite.emplace(*m_aimArrowTexture);
        sf::Vector2u arrowSize = m_aimArrowTexture->getSize();
        m_aimArrowSprite->setOrigin({arrowSize.x / 2.0f, arrowSize.y / 2.0f});
        m_aimArrowSprite->setScale({2.0f, 2.0f}); // Scale for visibility
        
        // Set color based on player ID
        sf::Color indicatorColor;
        switch (m_id) {
            case 0: indicatorColor = sf::Color::Green; break;
            case 1: indicatorColor = sf::Color::Blue; break;
            case 2: indicatorColor = sf::Color::Magenta; break;
            case 3: indicatorColor = sf::Color::Red; break;
            default: indicatorColor = sf::Color::White;
        }
        indicatorColor.a = 220; // Add some transparency
        m_aimArrowSprite->setColor(indicatorColor);
    }
}

void Player::drawAimIndicator(sf::RenderWindow& window) {
    if (!m_isChargingThrow || !m_isAlive || !m_aimArrowSprite) return;

    // Only show if there's a valid aim direction
    if (m_aimDirection.x == 0 && m_aimDirection.y == 0) return;

    // Calculate the position of the arrow
    float playerRadius = m_shape.getSize().x / 2.0f;
    sf::Vector2f arrowPos = m_position + m_aimDirection * (playerRadius + m_aimIndicatorDistance);
    
    // Calculate rotation angle from aim direction
    float angle = std::atan2(m_aimDirection.y, m_aimDirection.x) * 180.0f / 3.14159f;
    
    // Position and rotate the arrow sprite
    m_aimArrowSprite->setPosition(arrowPos);
    m_aimArrowSprite->setRotation(sf::degrees(angle));
    
    // Draw the arrow
    window.draw(*m_aimArrowSprite);
}

void Player::handleThrowInput(bool throwPressed, sf::Vector2f aimDirection) {
    if (!m_isAlive) return;

    m_readyToThrow = false; // Reset every frame

    // Store the current aim direction
    if (aimDirection.x != 0 || aimDirection.y != 0) {
        m_aimDirection = aimDirection;
    }

    if (throwPressed && !m_isChargingThrow && m_state == State::Normal) {
        // Start charging
        m_isChargingThrow = true;
        m_throwChargeTimer.restart();
        // Stop all movement when charging throw
        m_velocity.x = 0.f;
    }
    
    if (!throwPressed && m_isChargingThrow) {
        // Button was released, signal that we are ready to throw
        m_isChargingThrow = false;
        m_readyToThrow = true;
    }
}

bool Player::getReadyToThrow() const {
    return m_readyToThrow;
}

sf::Vector2f Player::releaseThrow() {
    // If the aim stick is neutral, use the last movement direction as a fallback
    if (m_aimDirection.x == 0 && m_aimDirection.y == 0) {
        m_aimDirection = m_dashDirection;
    }

    float baseSpeed = 30.f; 
    float throwSpeed = baseSpeed;

    return {m_aimDirection.x * throwSpeed, m_aimDirection.y * throwSpeed};
}


int Player::getControllerId() const{
    return m_controllerId;
}

void Player::handleInput() {
    if (!m_isAlive) return; // Don't handle input if dead

    // Define keys based on player ID
    bool isLeft = false;
    bool isRight = false;
    bool isJump = false;
    bool isDash = false;

    if (m_controllerId != -1) { // -1 will be our code for "keyboard"
        // --- Joystick Input ---
        float xAxis = sf::Joystick::getAxisPosition(m_controllerId, sf::Joystick::Axis::X);
        if (xAxis < -50) isLeft = true;
        if (xAxis > 50) isRight = true;
        // PS4/PS5 Cross button is typically button 0 on Mac/Linux
        if (sf::Joystick::isButtonPressed(m_controllerId, 1)) isJump = true; 
        // PS4/PS5 Square button is typically button 3
        if (sf::Joystick::isButtonPressed(m_controllerId, 0)) isDash = true;
    } else {
        // --- Keyboard Input (Player 1 Fallback) ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) isLeft = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) isRight = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) isJump = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) isDash = true;
    }

    if (m_state == State::Normal && !m_isChargingThrow) { 
        if (isLeft) {
            m_velocity.x = -5.f;
            m_dashDirection = {-1.f, 0.f};
        } else if (isRight) {
            m_velocity.x = 5.f;
            m_dashDirection = {1.f, 0.f};
        } else {
            m_velocity.x = 0.f;
        }

        if (isJump&& m_isGrounded) {
            m_velocity.y = -10.f;
            m_isGrounded = false;
        }

        if (isDash && m_dashCooldownTimer.getElapsedTime() >= m_dashCooldown) {
            std::cerr << "Dashing" << std::endl;
            m_state = State::Dashing;
            m_dashTimer.restart(); 
            m_dashCooldownTimer.restart();
        }
    }
    
}

void Player::update(const std::vector<sf::RectangleShape>& tiles) {
    // --- Vertical Physics ---

    // If death animation is complete, stop all updates
    if (m_deathAnimationComplete) return;

    // Define bounds constants before any goto statements
    const float TILEMAP_WIDTH = 800.f;
    const float TILEMAP_HEIGHT = 560.f;
    const float HALF_HITBOX_WIDTH = m_shape.getSize().x / 2.0f;
    const float HALF_HITBOX_HEIGHT = m_shape.getSize().y / 2.0f;

    // If player is dead but animation hasn't completed yet, skip physics but allow animation update
    if (!m_isAlive) {
        // Skip to animation update at the end
        goto update_animation;
    }

    switch (m_state) {
        case State::Normal:
            m_velocity.y += 0.5f; 
            if (m_velocity.y > 15.f) m_velocity.y = 15.f;
            break;
        case State::Dashing:
            m_velocity.y = 0;
            m_velocity.x = m_dashDirection.x * m_dashSpeed;
            if (m_dashTimer.getElapsedTime() >= m_dashDuration) {
                m_state = State::Normal;
                m_velocity.x = 0;
            }
            break;
        case State::Stunned:
            // Handle stunned state
            break;
    }
    m_position.y += m_velocity.y;
    m_shape.setPosition(m_position);
    m_isGrounded = false; // Assume we are in the air until we prove otherwise

    // Check for vertical collisions
    for (const auto& tile : tiles) {
        sf::FloatRect playerBounds = this->getBounds();
        sf::FloatRect tileBounds = tile.getGlobalBounds();

        if (playerBounds.findIntersection(tileBounds)) {
            if (m_velocity.y > 0) { // We were moving DOWN (landing on something)
                m_position.y = tile.getPosition().y - m_shape.getOrigin().y;
                m_velocity.y = 0;
                m_isGrounded = true;
            } else if (m_velocity.y < 0) { // We were moving UP (bumping our head)
                m_position.y = tile.getPosition().y + tile.getSize().y + m_shape.getOrigin().y;
                m_velocity.y = 0; // Bonk head on ceiling, stop rising
            }
        }
    }

    // --- Horizontal Physics ---
    // Don't apply horizontal movement if charging throw
    if (!m_isChargingThrow) {
        m_position.x += m_velocity.x; // Apply horizontal velocity
    }
    m_shape.setPosition(m_position); // Update shape to check for collision

    // Check for horizontal collisions
    for (const auto& tile : tiles) {
        sf::FloatRect playerBounds = this->getBounds();
        sf::FloatRect tileBounds = tile.getGlobalBounds();

        if (playerBounds.findIntersection(tileBounds)) {
            if (m_velocity.x > 0) { // We were moving RIGHT
                m_position.x = tile.getPosition().x - m_shape.getOrigin().x;
            } else if (m_velocity.x < 0) { // We were moving LEFT
                m_position.x = tile.getPosition().x + tile.getSize().x + m_shape.getOrigin().x;
            }
            m_velocity.x = 0; // Stop horizontal velocity on collision
        }
    }

    // --- Bounds checking to keep player within tilemap ---
    // Tilemap is 20 tiles wide × 14 tiles tall, with each tile being 40 pixels
    // So the playable area is 800 pixels wide × 560 pixels tall
    
    // Constrain horizontal position
    if (m_position.x - HALF_HITBOX_WIDTH < 0.f) {
        m_position.x = HALF_HITBOX_WIDTH;
        m_velocity.x = 0;
    } else if (m_position.x + HALF_HITBOX_WIDTH > TILEMAP_WIDTH) {
        m_position.x = TILEMAP_WIDTH - HALF_HITBOX_WIDTH;
        m_velocity.x = 0;
    }

    // Constrain vertical position
    if (m_position.y - HALF_HITBOX_HEIGHT < 0.f) {
        m_position.y = HALF_HITBOX_HEIGHT;
        m_velocity.y = 0;
    } else if (m_position.y + HALF_HITBOX_HEIGHT > TILEMAP_HEIGHT) {
        m_position.y = TILEMAP_HEIGHT - HALF_HITBOX_HEIGHT;
        m_velocity.y = 0;
        m_isGrounded = true; // Consider player grounded if hitting bottom bound
    }

    // Update shape position after bounds checking
    m_shape.setPosition(m_position);

update_animation:
    // Update animation
    m_animationTime += 1.0f / 60.0f; // Using fixed timestep for simplicity

    // Update facing direction based on velocity (only if alive)
    if (m_isAlive) {
        if (m_velocity.x > 0.1f) {
            m_facingRight = true;
        } else if (m_velocity.x < -0.1f) {
            m_facingRight = false;
        }
    }
    
    // Determine animation state based on player state (only if alive)
    if (m_isAlive) {
        switch (m_state) {
            case State::Dashing:
                m_currentAnimation = AnimationState::Dashing;
                break;
            case State::Stunned:
                // Handle stunned state if needed
                break;
            case State::Normal:
            default:
                if (!m_isGrounded) {
                    m_currentAnimation = AnimationState::Jumping;
                } else if (std::abs(m_velocity.x) > 0.1f) {
                    m_currentAnimation = AnimationState::Running;
                } else {
                    m_currentAnimation = AnimationState::Idle;
                }
                break;
        }
    }
    // If dead, keep the Dying animation state (set in kill())

    // Animation frame data: {startFrame, frameCount, frameTime}
    struct AnimationData {
        int startFrame;
        int frameCount;
        float frameTime;
    };

    // Map animation states to their frame data (all frames are in a single row)
    static const std::map<AnimationState, AnimationData> animationData = {
        {AnimationState::Idle,     {0,  4, 0.15f}},   // Frames 0-3: Idle
        {AnimationState::Running,  {4,  6, 0.1f}},    // Frames 4-9: Run
        {AnimationState::Jumping,  {11, 1, 0.1f}},    // Frames 10-11: Jump
        {AnimationState::Dashing,  {22, 1, 0.1f}},    // Frame 22: Dash
        {AnimationState::Dying,    {14, 3, 0.4f}}     // Frame 23: Death
    };
    
    // Update animation
    auto it = animationData.find(m_currentAnimation);
    if (it != animationData.end()) {
        const auto& animData = it->second;
        
        // Update animation timer
        m_animationTime += 1.0f / 60.0f; // Assuming 60 FPS
        
        // Special handling for death animation - play once and don't loop
        if (m_currentAnimation == AnimationState::Dying) {
            // Check if death animation has completed one full cycle
            if (m_animationTime >= animData.frameTime * animData.frameCount) {
                m_deathAnimationComplete = true;
                // Keep the last frame visible
                int frameIndex = animData.frameCount - 1;
                int frameX = (animData.startFrame + frameIndex) * m_spriteSize.x;
                m_currentFrame.position.x = frameX;
                m_currentFrame.position.y = 0;
                m_sprite.setTextureRect(m_currentFrame);
                return; // Stop updating once death animation is done
            }
            // Continue playing death animation if not complete
            int frameIndex = static_cast<int>(m_animationTime / animData.frameTime);
            if (frameIndex >= animData.frameCount) frameIndex = animData.frameCount - 1;
            int frameX = (animData.startFrame + frameIndex) * m_spriteSize.x;
            m_currentFrame.position.x = frameX;
            m_currentFrame.position.y = 0;
            m_sprite.setTextureRect(m_currentFrame);
        } else {
            // Normal animation logic for living player - loop continuously
            int frameIndex = static_cast<int>(m_animationTime / animData.frameTime) % animData.frameCount;
            int frameX = (animData.startFrame + frameIndex) * m_spriteSize.x;
            
            // Update sprite texture rectangle
            m_currentFrame.position.x = frameX;
            m_currentFrame.position.y = 0;
            m_sprite.setTextureRect(m_currentFrame);
            
            // Reset timer if we've completed a full animation cycle
            if (frameIndex == 0 && m_animationTime >= animData.frameTime * animData.frameCount) {
                m_animationTime = 0.0f;
            }
        }
    }
    
    // Update the visual shape's position (for collision)
    m_shape.setPosition(m_position);
}

void Player::draw(sf::RenderWindow& window) {
    // Don't draw if death animation has completed
    if (m_deathAnimationComplete) return;

    // Debug: Draw hitbox behind sprite
    
        sf::RectangleShape debugShape = m_shape;
        debugShape.setFillColor(sf::Color::Transparent);
        debugShape.setOutlineColor(sf::Color::Red);
        debugShape.setOutlineThickness(2.0f);
        window.draw(debugShape);
    

    if(m_texture->getSize().x > 0){
        // Update sprite position
        m_sprite.setPosition(m_position);
        
        // Flip sprite based on facing direction while maintaining scale
        float scaleX = m_facingRight ? 3.f : -3.f;
        m_sprite.setScale({scaleX, 3.f});
        
        // Adjust origin when flipping to keep sprite centered
        if (m_facingRight) {
            m_sprite.setOrigin({m_spriteSize.x / 2.0f, m_spriteSize.y / 2.0f});
        } else {
            m_sprite.setOrigin({m_spriteSize.x / 2.0f, m_spriteSize.y / 2.0f});
        }
        
        window.draw(m_sprite);
    } else {
        // Fallback to drawing the shape if texture failed to load
        if (m_state == State::Normal){
            m_shape.setFillColor(sf::Color::Green);
        } else if (m_state == State::Dashing) {
            m_shape.setFillColor(sf::Color::Yellow);
        }
        window.draw(m_shape);
    }
    
    // Draw the aim indicator when charging a throw
    if (m_isChargingThrow) {
        drawAimIndicator(window);
    }
}

sf::FloatRect Player::getBounds() const {
    return m_shape.getGlobalBounds();
}
sf::Vector2f Player::getPosition() const { return m_position; }
sf::Vector2f Player::getFacingDirection() const { return m_dashDirection; }

int Player::getId() const { return m_id; }
bool Player::isAlive() const { return m_isAlive; }
void Player::kill() { 
    m_isAlive = false;
    m_currentAnimation = AnimationState::Dying;
    m_animationTime = 0.0f; // Reset animation timer to start death animation from beginning
}

void Player::startThrowCharge() {
    if (m_state == State::Normal) {
        m_isChargingThrow = true;
        m_throwChargeTimer.restart();
    }
}

bool Player::isChargingThrow() const {
    return m_isChargingThrow;
}

bool Player::isDeathAnimationComplete() const {
    return m_deathAnimationComplete;
}




    
