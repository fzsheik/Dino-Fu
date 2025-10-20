#include "boomerang.hpp"
#include <cmath>
#include <iostream>

float length(const sf::Vector2f& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

sf::Vector2f normalize(const sf::Vector2f& source) {
    float length = std::sqrt((source.x * source.x) + (source.y * source.y));
    if (length != 0) {
        return sf::Vector2f(source.x / length, source.y / length);
    }
    return source;
}

Boomerang::Boomerang(std::shared_ptr<sf::Texture> texture, sf::Vector2f startPosition, sf::Vector2f initialVelocity, int ownerId) 
    : m_position(startPosition)
    , m_velocity(initialVelocity)
    , m_state(State::FlyingOut)
    , m_ownerId(ownerId)
    , m_texture(texture)
    , m_sprite(*m_texture)
    , m_rotation(0.0f)
    , m_rotationSpeed(720.0f)
{
    // Setup sprite with the provided texture (texture is already loaded by runner.cpp)
    m_sprite.setTextureRect(sf::IntRect({0, 0}, {(int)m_texture->getSize().x, (int)m_texture->getSize().y}));
    
    // Center the origin based on texture size
    sf::Vector2u texSize = m_texture->getSize();
    m_sprite.setOrigin({texSize.x / 2.0f, texSize.y / 2.0f});

    // Scale the sprite
    m_sprite.setScale({2.5f, 2.5f});
    m_sprite.setPosition(m_position);
    
    // Set hitbox to match sprite size (circular collision will be used)
    float spriteSize = std::max(texSize.x, texSize.y) * 2.5f;
    m_shape.setSize({spriteSize, spriteSize});
    m_shape.setFillColor(sf::Color::Magenta);
    m_shape.setOrigin({spriteSize / 2.0f, spriteSize / 2.0f});
    m_shape.setPosition(m_position);

    m_hangDuration = sf::seconds(0.05f);
}

void Boomerang::update(sf::Vector2f playerPosition, const std::vector<sf::RectangleShape>& tiles) {
    switch (m_state) {
        case State::FlyingOut:
        {
            m_velocity *= 0.98f;
            m_position += m_velocity;

            // Circle-based collision detection
            float radius = m_shape.getSize().x / 2.0f * 0.7f; // Use 70% of sprite size for tighter collision
            
            for (const auto& tile : tiles) {
                sf::FloatRect tileBounds = tile.getGlobalBounds();
                
                // Find closest point on the rectangle to the circle center
                float closestX = std::max(tileBounds.position.x, std::min(m_position.x, tileBounds.position.x + tileBounds.size.x));
                float closestY = std::max(tileBounds.position.y, std::min(m_position.y, tileBounds.position.y + tileBounds.size.y));
                
                // Calculate distance from circle center to closest point
                float distanceX = m_position.x - closestX;
                float distanceY = m_position.y - closestY;
                float distanceSquared = distanceX * distanceX + distanceY * distanceY;
                
                // Check if collision occurred
                if (distanceSquared < radius * radius) {
                    float distance = std::sqrt(distanceSquared);
                    
                    // Calculate collision normal
                    sf::Vector2f normal;
                    if (distance > 0.001f) {
                        normal = sf::Vector2f(distanceX / distance, distanceY / distance);
                    } else {
                        // Boomerang center is inside tile, use velocity direction
                        normal = normalize(sf::Vector2f(-m_velocity.x, -m_velocity.y));
                    }
                    
                    // Reflect velocity along the normal with energy loss
                    float dotProduct = m_velocity.x * normal.x + m_velocity.y * normal.y;
                    m_velocity.x = m_velocity.x - 2.0f * dotProduct * normal.x;
                    m_velocity.y = m_velocity.y - 2.0f * dotProduct * normal.y;
                    m_velocity *= 0.85f; // Energy loss on bounce
                    
                    // Push boomerang out of collision
                    float overlap = radius - distance + 0.5f; // Small buffer to prevent re-collision
                    m_position.x += normal.x * overlap;
                    m_position.y += normal.y * overlap;
                    
                    break; // Only handle one collision per frame
                }
            }
            
            if (length(m_velocity) < 1.f) {
                m_state = State::Hanging;
                m_hangTimer.restart();
            }
            break;
        }

        case State::Hanging:
            if (m_hangTimer.getElapsedTime() >= m_hangDuration) {
                m_state = State::Returning;
            }
            break;

        case State::Returning:
            { 
                // Boomerang returns to player
                sf::Vector2f returnDirection = playerPosition - m_position;
                if (length(returnDirection) < 20.f) {
                    m_state = State::Caught;
                    break;
                }
                returnDirection = normalize(returnDirection);
                float returnSpeed = 5.f; 
                m_velocity = returnDirection * returnSpeed;
                m_position += m_velocity;

                // Circle-based collision detection for ricochet
                float radius = m_shape.getSize().x / 2.0f * 0.7f;
                
                for (const auto& tile : tiles) {
                    sf::FloatRect tileBounds = tile.getGlobalBounds();
                    
                    // Find closest point on the rectangle to the circle center
                    float closestX = std::max(tileBounds.position.x, std::min(m_position.x, tileBounds.position.x + tileBounds.size.x));
                    float closestY = std::max(tileBounds.position.y, std::min(m_position.y, tileBounds.position.y + tileBounds.size.y));
                    
                    // Calculate distance from circle center to closest point
                    float distanceX = m_position.x - closestX;
                    float distanceY = m_position.y - closestY;
                    float distanceSquared = distanceX * distanceX + distanceY * distanceY;
                    
                    // Check if collision occurred
                    if (distanceSquared < radius * radius) {
                        float distance = std::sqrt(distanceSquared);
                        
                        // Calculate collision normal
                        sf::Vector2f normal;
                        if (distance > 0.001f) {
                            normal = sf::Vector2f(distanceX / distance, distanceY / distance);
                        } else {
                            normal = normalize(sf::Vector2f(-m_velocity.x, -m_velocity.y));
                        }
                        
                        // Reflect velocity along the normal to prevent tunneling
                        float dotProduct = m_velocity.x * normal.x + m_velocity.y * normal.y;
                        m_velocity.x = m_velocity.x - 2.0f * dotProduct * normal.x;
                        m_velocity.y = m_velocity.y - 2.0f * dotProduct * normal.y;
                        
                        // Push boomerang out of collision
                        float overlap = radius - distance + 0.5f;
                        m_position.x += normal.x * overlap;
                        m_position.y += normal.y * overlap;
                        
                        break; // Only handle one collision per frame
                    }
                }
            }
            break;
        
        case State::Caught:
            break;
    }
    
    m_shape.setPosition(m_position);
    
    // Update rotation based on velocity (faster = faster spin)
    float speed = length(m_velocity);
    m_rotation += m_rotationSpeed * (1.0f / 60.0f); // Assuming 60 FPS
    if (m_rotation >= 360.0f) m_rotation -= 360.0f;
    
    // Temporarily disabled rotation to test visibility
    m_sprite.setRotation(sf::degrees(m_rotation));
    m_sprite.setPosition(m_position);
}

void Boomerang::draw(sf::RenderWindow& window) {
    if (m_state != State::Caught){
        // Always try to draw the sprite if texture is loaded
        if (m_texture->getSize().x > 0) {
            window.draw(m_sprite, sf::RenderStates::Default);
            
            // Debug: draw the circular hitbox
            float radius = m_shape.getSize().x / 2.0f * 0.7f;
            sf::CircleShape debugCircle(radius);
            debugCircle.setOrigin({radius, radius});
            debugCircle.setPosition(m_position);
            debugCircle.setFillColor(sf::Color::Transparent);
            debugCircle.setOutlineColor(sf::Color::Cyan);
            debugCircle.setOutlineThickness(2.0f);
            window.draw(debugCircle);
        } else {
            // Fallback to rectangle if texture not loaded
            window.draw(m_shape);
        }
    }
}

Boomerang::State Boomerang::getState() const {
    return m_state;
}

sf::FloatRect Boomerang::getBounds() const {
    return m_shape.getGlobalBounds();
}
int Boomerang::getOwnerId() const { return m_ownerId; }
