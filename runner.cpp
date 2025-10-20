
#include <iostream>
#include <cstdio>
#include <vector>
#include "player.hpp"
#include "boomerang.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <string>
#include <random>
#include <ctime>

enum class GameState {
    MainMenu,
    Gameplay,
    GameOver
};

template<typename T>
void resetToMainMenu(GameState& gameState, bool& gameOverTriggered, 
                    std::vector<T>& players, 
                    std::vector<Boomerang>& boomerangs,
                    std::vector<int>& joinedControllers,
                    sf::Time& gameOverDelay) {
    gameState = GameState::MainMenu;
    gameOverTriggered = false;
    players.clear();
    boomerangs.clear();
    joinedControllers.clear();
    gameOverDelay = sf::Time::Zero;
}

sf::Vector2f bnormalize(const sf::Vector2f& source) {
    float length = std::sqrt(source.x * source.x + source.y * source.y);
        if (length != 0) return {source.x / length, source.y / length};
        return source;
    }
int main() {
    const unsigned int windowWidth = 800;
    const unsigned int windowHeight = 500;
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "Platformer Sandbox");
    window.setFramerateLimit(60);
    // --- Load Textures ---
    // Load main menu background
    sf::Texture menuBackgroundTexture;
    if(!menuBackgroundTexture.loadFromFile("assets/mc_bg.jpg")){
        std::cerr << "Error: Could not load menu background image!" << std::endl;
    }
    sf::Sprite menuBackgroundSprite(menuBackgroundTexture);
    
    // Scale menu background
    sf::Vector2u menuBgSize = menuBackgroundTexture.getSize();
    float menuScaleX = (float)windowWidth / menuBgSize.x;
    float menuScaleY = (float)windowHeight / menuBgSize.y;
    menuBackgroundSprite.setScale({menuScaleX, menuScaleY});
    
    // Load gameplay backgrounds (array of possible backgrounds)
    std::vector<std::string> gameplayBackgroundFiles = {
        "assets/clouds.jpg",
        "assets/cyberpunk2_bg.jpg",
        "assets/cyberpunk_background.png",
        "assets/minecraft_bg.jpg",
        "assets/minecarft_bg.jpg"
    };
    
    std::vector<sf::Texture> gameplayBackgroundTextures;
    for (const auto& filename : gameplayBackgroundFiles) {
        sf::Texture texture;
        if (texture.loadFromFile(filename)) {
            gameplayBackgroundTextures.push_back(std::move(texture));
        } else {
            std::cerr << "Warning: Could not load background " << filename << std::endl;
        }
    }
    
    // Random number generator for background selection
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> bgDist(0, gameplayBackgroundTextures.size() - 1);
    
    // Sprite for gameplay background (initialize with first texture if available)
    int selectedBackgroundIndex = 0;
    sf::Sprite gameplayBackgroundSprite(gameplayBackgroundTextures.empty() ? menuBackgroundTexture : gameplayBackgroundTextures[0]);

    // Load grass texture
    sf::Texture grassTexture;
    if (!grassTexture.loadFromFile("assets/grass_tile.png")) {
        std::cerr << "Error: Could not load grass texture!" << std::endl;
    }
    //grassTexture.setRepeated(true);
    
    // Create dark overlay for main menu to make UI elements stand out
    sf::RectangleShape menuOverlay;
    menuOverlay.setSize({static_cast<float>(windowWidth), static_cast<float>(windowHeight)});
    menuOverlay.setFillColor(sf::Color(0, 0, 0, 150)); // Semi-transparent black (alpha: 150/255)
    
    //backgroundSprite.setPosition(0, 0);
    // Load boomerang texture once (shared by all boomerangs)
    std::shared_ptr<sf::Texture> boomerangTexture = std::make_shared<sf::Texture>();
    if (!boomerangTexture->loadFromFile("vite_boomerang.png")) {
        std::cerr << "Error: Could not load boomerang texture!" << std::endl;
    }

    // Load dino character textures (one for each player)
    std::vector<std::shared_ptr<sf::Texture>> dinoTextures(4);
    std::string dinoFiles[4] = {
        "assets/DinoSprites - doux.png",
        "assets/DinoSprites - mort.png",
        "assets/DinoSprites - tard.png",
        "assets/DinoSprites - vita.png"
    };
    
    for (int i = 0; i < 4; i++) {
        dinoTextures[i] = std::make_shared<sf::Texture>();
        if (!dinoTextures[i]->loadFromFile(dinoFiles[i])) {
            std::cerr << "Error: Could not load dino texture " << dinoFiles[i] << std::endl;
        }
    }

    // Load p1-p4 indicator textures
    std::vector<std::shared_ptr<sf::Texture>> playerIndicatorTextures(4);
    std::vector<sf::Sprite> playerIndicatorSprites;
    std::vector<sf::Sprite> playerDinoPreviewSprites; // Dino sprites for menu preview
    std::string indicatorFiles[4] = {
        "assets/p1.png",
        "assets/p2.png",
        "assets/p3.png",
        "assets/p4.png"
    };
    
    for (int i = 0; i < 4; i++) {
        playerIndicatorTextures[i] = std::make_shared<sf::Texture>();
        if (!playerIndicatorTextures[i]->loadFromFile(indicatorFiles[i])) {
            std::cerr << "Error: Could not load indicator texture " << indicatorFiles[i] << std::endl;
        } else {
            sf::Sprite sprite(*playerIndicatorTextures[i]);
            // Position indicators at bottom of screen, spread out
            float spacing = windowWidth / 5.0f; // Divide screen into 5 sections for 4 indicators
            float yPosition = windowHeight - 80.0f; // 80 pixels from bottom
            sprite.setPosition({spacing * (i + 1) - playerIndicatorTextures[i]->getSize().x / 2.0f, yPosition});
            sprite.setScale({2.0f, 2.0f}); // Scale up the indicators
            playerIndicatorSprites.push_back(sprite);
            
            // Create dino preview sprite (first frame of animation)
            sf::Sprite dinoPreview(*dinoTextures[i]);
            dinoPreview.setTextureRect(sf::IntRect({0, 0}, {24, 24})); // First frame
            dinoPreview.setScale({3.0f, 3.0f}); // Make it bigger for visibility
            // Position above the indicator
            float dinoX = spacing * (i + 1) - 12.0f * 3.0f; // Center the 24px sprite scaled 3x
            float dinoY = yPosition - 90.0f; // Position above indicator
            dinoPreview.setPosition({dinoX, dinoY});
            playerDinoPreviewSprites.push_back(dinoPreview);
        }
    }
    // Load game over and restart textures
    sf::Texture gameOverTexture;
    if (!gameOverTexture.loadFromFile("assets/gameover.png")) {
        std::cerr << "Error: Could not load game over image!" << std::endl;
    }
    sf::Sprite gameOverSprite(gameOverTexture);
    
    sf::Texture restartTexture;
    if (!restartTexture.loadFromFile("assets/restart.png")) {
        std::cerr << "Error: Could not load restart image!" << std::endl;
    }
    sf::Sprite restartSprite(restartTexture);
    
    // Position game over and restart sprites
    gameOverSprite.setOrigin(sf::Vector2f(gameOverTexture.getSize()) / 2.0f);
    gameOverSprite.setPosition(sf::Vector2f(windowWidth / 2.0f, windowHeight / 2.0f - 50));
    gameOverSprite.setScale({3.0f, 3.0f});
    
    restartSprite.setOrigin(sf::Vector2f(restartTexture.getSize()) / 2.0f);
    restartSprite.setPosition(sf::Vector2f(windowWidth / 2.0f, windowHeight / 2.0f + 100));
    restartSprite.setScale({3.0f, 3.0f});
    
    // Create semi-transparent overlay
    sf::RectangleShape overlay(sf::Vector2f(windowWidth, windowHeight));
    overlay.setFillColor(sf::Color(0, 0, 0, 180)); // Semi-transparent black
    
    sf::Texture titleCardTexture;
    if (!titleCardTexture.loadFromFile("assets/titlecard.png")) {
        std::cerr << "Error: Could not load title card image!" << std::endl;
    }
    sf::Sprite titleCardSprite(titleCardTexture);

    // Position the title card at the top center of the screen
    sf::Vector2u titleSize = titleCardTexture.getSize();
    titleCardSprite.setPosition({(windowWidth - titleSize.x) / 2.0f, 50.0f});


    // Load "Press any button to join" text art
    sf::Texture joinPromptTexture;
    if (!joinPromptTexture.loadFromFile("assets/pabtj.png")) {  // Update with your actual filename
        std::cerr << "Error: Could not load join prompt image!" << std::endl;
    }

    sf::Sprite joinPromptSprite(joinPromptTexture);

    // Position join prompt below the title card with some spacing
    float spacing = 20.0f;  // Space between title and join prompt
    joinPromptSprite.setPosition(
        {(windowWidth - joinPromptTexture.getSize().x) / 2.0f,  // Center horizontally
        titleCardSprite.getPosition().y + titleSize.y + spacing});





            // After loading the title card texture but before setting its position
    titleCardSprite.setScale({2.5f, 2.5f});  // 2x scale - adjust these values as needed

    // After loading the join prompt texture but before setting its position
    joinPromptSprite.setScale({2.0f, 2.0f});  // 2x scale - adjust these values as needed

    // Then update their positions to account for the new size
    titleCardSprite.setPosition({
        (windowWidth - (titleSize.x * titleCardSprite.getScale().x)) / 2.0f, 
        50.0f
    });

// Update join prompt position with scaling in mind
// Position it in the middle area of the screen to ensure visibility
float joinPromptY = 220.0f; // Fixed position to keep it visible above player indicators
joinPromptSprite.setPosition({
    (windowWidth - (joinPromptTexture.getSize().x * joinPromptSprite.getScale().x)) / 2.0f,
    joinPromptY
});

    // --- Tilemap Data ---
    // Here we define our level layout using strings.
    // '#' = a solid tile. '.' = empty space.
    std::vector<std::string> tilemap = {
        "....................",
        "....................",
        "..G.................",
        "..#..........GG..G..",
        "..#.................",
        ".....GGG.......#.....",
        ".......#G...........",
        "..G.........#.......",
        "....................",
        ".....GGGG......G....",
        "....................",
        "GG................GG",
        "##GGGGGGGGGG..GGGG##",
        "############..######"
    };

    // --- Create the visual tiles from the map data ---
    std::vector<sf::RectangleShape> tiles;
    const float TILE_SIZE = 40.f; // Each tile will be 40x40 pixels

    for (int y = 0; y < tilemap.size(); ++y) {
        for (int x = 0; x < tilemap[y].size(); ++x) {
            if (tilemap[y][x] == '#' || tilemap[y][x] == 'G') {
                sf::RectangleShape tile;
                tile.setSize({TILE_SIZE, TILE_SIZE});
                
                if (tilemap[y][x] == 'G') {
                    // Grass tile
                    tile.setTexture(&grassTexture);
                    int tileSize = static_cast<int>(TILE_SIZE);
                    tile.setTextureRect(sf::IntRect(
                        sf::Vector2i(0, 0),  // position
                        sf::Vector2i(tileSize, tileSize)  // size
                    ));
                } else {
                    // Regular solid tile
                    tile.setFillColor(sf::Color(100, 100, 100)); // Dark grey
                }
                
                tile.setPosition({x * TILE_SIZE, y * TILE_SIZE});
                tiles.push_back(tile);
            }
        }
    }

        sf::Vector2f startpositions[] = {
        {100.f, 100.f},
        {700.f, 100.f},
        {200.f, 100.f},
        {600.f, 100.f}
    };
    // --- Game Entities ---
    std::vector<Player> players;
    std::vector<Boomerang> boomerangs;
    std::vector<int> joinedControllers;
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    const sf::Time timePerFrame = sf::seconds(1.f / 60.f);
    sf::Time gameOverDelay = sf::Time::Zero;
    bool gameOverTriggered = false;

    // Start the game in the Main Menu
    GameState gameState = GameState::MainMenu;

    while (window.isOpen()) {
        // Handle events first
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (gameState == GameState::MainMenu) {
                if(event->is<sf::Event::JoystickButtonPressed>()){
                    if(const auto* joybtn = event->getIf<sf::Event::JoystickButtonPressed>()){
                        unsigned int jid = joybtn->joystickId;
                        bool alreadyjoined = (std::find(joinedControllers.begin(), joinedControllers.end(), jid) != joinedControllers.end());

                        if(!alreadyjoined && players.size() < 4){
                            int newplayerid = players.size();
                            // Pass the appropriate dino texture based on player ID
                            players.emplace_back(startpositions[newplayerid].x, startpositions[newplayerid].y, newplayerid, jid, dinoTextures[newplayerid]);
                            joinedControllers.push_back(jid);
                        }
                    }
                }
            }
        }

        // Update game state
        switch (gameState) {
            case GameState::MainMenu:
            {
                // Check for game start
                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) || 
                    (sf::Joystick::isConnected(0) && sf::Joystick::isButtonPressed(0, 7))) // Start button
                    && !players.empty()) {
                    // Select a random background for gameplay
                    if (!gameplayBackgroundTextures.empty()) {
                        selectedBackgroundIndex = bgDist(gen);
                        gameplayBackgroundSprite.setTexture(gameplayBackgroundTextures[selectedBackgroundIndex]);
                        
                        // Scale the gameplay background to fit the window
                        sf::Vector2u bgSize = gameplayBackgroundTextures[selectedBackgroundIndex].getSize();
                        float bgScaleX = (float)windowWidth / bgSize.x;
                        float bgScaleY = (float)windowHeight / bgSize.y;
                        gameplayBackgroundSprite.setScale({bgScaleX, bgScaleY});
                    }
                    gameState = GameState::Gameplay;
                }

                window.clear();
                window.draw(menuBackgroundSprite);
                window.draw(menuOverlay); // Dark overlay to make UI stand out
                window.draw(titleCardSprite);
                window.draw(joinPromptSprite);
                
                // Draw dino previews and p1-p4 indicators for joined players
                for (int i = 0; i < players.size() && i < 4; i++) {
                    window.draw(playerDinoPreviewSprites[i]); // Draw dino sprite above
                    window.draw(playerIndicatorSprites[i]);    // Draw p1-p4 indicator below
                }
                
                window.display();
                break;
            }
            
        

            case GameState::Gameplay:
            {
                // --- Gameplay Logic ---
                for (auto& player : players) {
                    sf::Vector2f aimDir = {0,0};
                    bool throwPressed = false;
                    int controllerId = player.getControllerId();
                    
                    if (controllerId != -1 && sf::Joystick::isConnected(controllerId)) {
                        // DEBUG: Print all axis values to find the right stick mapping
                        ;
                        
                        // Get aim direction from right stick (using Z and R axes)
                        float rightX = sf::Joystick::getAxisPosition(controllerId, sf::Joystick::Axis::Z);
                        float rightY = sf::Joystick::getAxisPosition(controllerId, sf::Joystick::Axis::R);
                        
                        // Apply deadzone but still pass the direction if outside deadzone
                        const float deadZone = 25.0f;
                        if (std::abs(rightX) > deadZone || std::abs(rightY) > deadZone) {
                            // Normalize the direction vector
                            aimDir = bnormalize({rightX, rightY});
                            
                        }
                        // If inside deadzone, aimDir stays {0,0} and player keeps last direction
                        
                        // Check throw button state (R1)
                        throwPressed = sf::Joystick::isButtonPressed(controllerId, 5);
                    }

                    // Handle throw input
                    player.handleThrowInput(throwPressed, aimDir);
                    
                    // Handle player movement
                    player.handleInput();
                    if (player.getReadyToThrow() && boomerangs.size() < 1) {
                        Boomerang boomerang(boomerangTexture, player.getPosition(), player.releaseThrow(), player.getId());
                        boomerangs.push_back(boomerang);
                    }
                }


                for (auto& player : players) {
                    player.handleInput();  // Handle input for each player
                    player.update(tiles);
                }

                for (auto& boomerang : boomerangs) {
                    int ownerId = boomerang.getOwnerId();
                    if (ownerId >= 0 && ownerId < players.size()) {
                        boomerang.update(players[ownerId].getPosition(), tiles); 
                    }
                }

                for (auto& boomerang : boomerangs) {
                    for (auto& player : players) {
                        sf::FloatRect boomBounds = boomerang.getBounds();
                        sf::FloatRect playerBounds = player.getBounds();
                        if(boomBounds.findIntersection(playerBounds) && boomerang.getOwnerId() != player.getId() && player.isAlive()){
                            player.kill();    
                        }
                    }
                }

                boomerangs.erase(
                    std::remove_if(boomerangs.begin(), boomerangs.end(), 
                        [](const Boomerang& b) {
                            return b.getState() == Boomerang::State::Caught;
                        }),
                    boomerangs.end()
                );


                players.erase(
                    std::remove_if(players.begin(), players.end(), 
                        [](const Player& p) {
                            return p.isDeathAnimationComplete() == true;
                        }),
                    players.end()
                );

                if(players.size() == 1 && !gameOverTriggered) {
                    if (gameOverDelay == sf::Time::Zero) {
                        // Start the delay timer when we first detect one player remaining
                        gameOverDelay = clock.getElapsedTime();
                    } else if (clock.getElapsedTime() - gameOverDelay >= sf::seconds(1.0f)) {
                        // After 1 second delay, trigger game over
                        gameState = GameState::GameOver;
                        gameOverTriggered = true;
                    }
                } else if (players.size() > 1) {
                    // Reset the timer if players are revived or added back
                    gameOverDelay = sf::Time::Zero;
                }

                window.clear(sf::Color(50, 50, 150));
                window.draw(gameplayBackgroundSprite);
                for (const auto& tile : tiles) { window.draw(tile); }
                for (auto& player : players) { player.draw(window); }
                for (auto& boomerang : boomerangs) { boomerang.draw(window); }
                window.display();
                break;
            }
            case GameState::GameOver: 
            {
                // Handle input for game over screen
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
                    (sf::Joystick::isConnected(0) && sf::Joystick::isButtonPressed(0, 0))) {  // Any button on any controller
                    
                    std::cout << "Button pressed - restarting" << std::endl;
                    resetToMainMenu(gameState, gameOverTriggered, players, boomerangs, joinedControllers, gameOverDelay);
                }

                // Draw the game state
                window.clear(sf::Color(50, 50, 150));
                window.draw(gameplayBackgroundSprite);
                for (const auto& tile : tiles) { window.draw(tile); }
                for (auto& player : players) { player.draw(window); }
                for (auto& boomerang : boomerangs) { boomerang.draw(window); }
                
                // Draw overlay and game over screen
                window.draw(overlay);
                window.draw(gameOverSprite);
                window.draw(restartSprite);
                
                window.display();
                break;
            }
        }
    }

    return 0;
}

