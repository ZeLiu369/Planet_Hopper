
#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

class Scene_OptionMenu : public Scene
{

protected:
    std::string m_title;
    std::vector<std::string> m_menuStrings;
    sf::Text m_menuText;
    sf::Text confirmText;
    size_t m_selectedMenuIndex = 0;
    float sounds_volume;
    float music_volume;
    std::string diff1;
    sf::Clock clock;

    void init();
    void update();
    void onEnd();
    void sDoAction(const Action &action);

public:
    Scene_OptionMenu(GameEngine *gameEngine);
    void sRender();
};

