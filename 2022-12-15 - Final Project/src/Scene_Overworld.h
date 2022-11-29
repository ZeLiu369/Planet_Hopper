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
    std::vector<std::string>    m_levelPaths;
    sf::Text                    m_text;
    size_t                      m_selectedMenuIndex = 0;
    bool                        m_drawTextures = true;
    bool                        m_drawCollision = false;
    bool                        m_changeScene = false;
    sf::Sprite background;

    void init();
    void loadMap();
    void update();
    void onEnd();
    void spawnPlayer();

    void sMovement();
    void sAnimation();
    void sCollision();

public:

    Scene_Overworld(GameEngine* gameEngine);

    void sRender();
    void sDoAction(const Action& action);
};
