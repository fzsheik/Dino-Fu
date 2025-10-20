

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <memory>  // For shared_ptr



class Player {

    
        public:

            enum class State {
                Normal,
                Dashing,
                Stunned
            };

            enum class AnimationState {
                Idle,
                Running,
                Jumping, 
                Dashing,
                Throwing,
                Dying
                // Add more states as needed (Jumping, Dashing, etc.)
            };

            Player(float x, float y, int id, int controllerId, std::shared_ptr<sf::Texture> texture); // Constructor takes starting position and texture
        
            void handleInput();
            
            
            // We'll need functions to get the player's bounding box for collision later
            

            
            sf::Vector2f getFacingDirection() const;

          
            

             void startThrowCharge();
             bool isChargingThrow() const;
                // This function will calculate the power and return the initial velocity for the boomerang


            void handleThrowInput(bool throwPressed, sf::Vector2f aimDirection); 
            void update(const std::vector<sf::RectangleShape>& tiles);
            void draw(sf::RenderWindow& window);
            void drawAimIndicator(sf::RenderWindow& window);

            sf::FloatRect getBounds() const;
            sf::Vector2f getPosition() const;
                
            int getId() const;
            bool isAlive() const;
            void kill();
            int getControllerId() const;
                
            // This is now a "getter" to signal the main loop
            bool getReadyToThrow() const; 
            sf::Vector2f releaseThrow(); 
            bool isDeathAnimationComplete() const;

           

        private:
            sf::RectangleShape m_shape;
            sf::Vector2f m_position;
            sf::Vector2f m_velocity;
            bool m_isGrounded;

            State m_state;
            sf::Vector2f m_dashDirection;
            float m_dashSpeed;
            sf::Time m_dashDuration;
            sf::Clock m_dashTimer;
            sf::Time m_dashCooldown;
            sf::Clock m_dashCooldownTimer;

            // New variables for charging the boomerang throw
            bool m_isChargingThrow;
            sf::Clock m_throwChargeTimer;

            bool m_readyToThrow; 
            sf::Vector2f m_aimDirection;

            // Aim indicator
            std::shared_ptr<sf::Texture> m_aimArrowTexture;
            std::optional<sf::Sprite> m_aimArrowSprite;
            float m_aimIndicatorDistance;

            int m_id;
            bool m_isAlive;
            bool m_deathAnimationComplete;
            int m_controllerId;

            // Animation members
            std::shared_ptr<sf::Texture> m_texture;
            sf::Sprite m_sprite;
            sf::IntRect m_currentFrame;
            sf::Vector2i m_spriteSize;
            bool m_facingRight;
            float m_animationTime;
            float m_animationSpeed;
            AnimationState m_currentAnimation;
    };


        
#endif