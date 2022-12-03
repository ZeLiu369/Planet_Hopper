#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

class Scene_Overworld : public Scene
{
    std::shared_ptr<Entity>     m_player;
    std::shared_ptr<Entity>     planet1, planet2, planet3;
    std::vector<std::string>    m_levelPaths;
    int                         m_level;
    sf::Text                    m_text;
    size_t                      m_selectedMenuIndex = 0;
    bool                        m_drawTextures = true;
    bool                        m_drawCollision = false;
    bool                        m_changeScene = false;
    std::string                 shake;
    sf::Sprite                  background;
    sf::Shader                  *shader, red_shader, grey_shader, fade_shader, shake_shader;
    sf::Clock                   time;
    std::string                 direction = "none";
    int                         offset = 0;
    int                         currPlanet;
    std::vector<sf::VertexArray> points;

    void init(const int& level);
    int  loadShaders();
    void loadMap(const int& levelAvailable);
    void update();
    void onEnd();
    void spawnPlayer();

    void sMovement();
    void sCollision();

public:

    Scene_Overworld(GameEngine* gameEngine, const int& level);

    void sRender();
    void sDoAction(const Action& action);
};
