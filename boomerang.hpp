#ifndef BOOMERANG_HPP
#define BOOMERANG_HPP

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class Boomerang {
public:

    enum class State {
        FlyingOut,
        Hanging,
        Returning,
        Caught
    };

    Boomerang(std::shared_ptr<sf::Texture> texture, sf::Vector2f startPosition, sf::Vector2f direction, int ownerId);

    void update(sf::Vector2f playerPosition, const std::vector<sf::RectangleShape>& tiles);
    void draw(sf::RenderWindow& window);

    sf::Time m_hangDuration;
    sf::Clock m_hangTimer;
    sf::FloatRect getBounds() const;
    State getState() const;
    int getOwnerId() const;
private:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    
    sf::RectangleShape m_shape;
    State m_state;
    int  m_ownerId;
    
    // Sprite and animation
    std::shared_ptr<sf::Texture> m_texture;
    sf::Sprite m_sprite;
    sf::IntRect m_currentFrame;
    sf::Vector2i m_spriteSize;
    float m_rotation;
    float m_rotationSpeed;
};

#endif