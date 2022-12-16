
#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

class Scene_Keybinding : public Scene
{

protected:
    std::string m_title;
    std::vector<std::string> m_menuStrings;
    sf::Text m_menuText;
    sf::Text confirmText;
    size_t m_selectedMenuIndex = 0;
    sf::Clock clock;
    std::string shootKey1;
    std::string moveKey1;

    void init();
    void update();
    void onEnd();
    void sDoAction(const Action &action);

public:
    Scene_Keybinding(GameEngine *gameEngine);
    void sRender();
};
