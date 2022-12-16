
#pragma once

#include "Common.h"
#include "Action.h"
#include "EntityManager.h"

#include <memory>

class GameEngine;
                                                     
typedef std::map<int, std::string> ActionMap;
                                                     
class Scene
{

protected: 
    
    GameEngine *    m_game = nullptr;
    EntityManager   m_entityManager;
    ActionMap       m_actionMap;
    bool            m_paused = false;
    bool            m_hasEnded = false;
    size_t          m_currentFrame = 0;
    float           bulletScaler = 1.0f;

    virtual void onEnd() = 0;
    

public:
    Scene();
    Scene(GameEngine * gameEngine);

    virtual void update() = 0;
    virtual void sDoAction(const Action & action) = 0;
    virtual void sRender() = 0;

    virtual void doAction(const Action& action);
    void simulate(const size_t frames);
    void setPaused(bool paused);
    void registerAction(int inputKey, const std::string &actionName);
    void unregisterAction(int inputKey);

    size_t width() const;
    size_t height() const;
    size_t currentFrame() const;

    bool m_optionMenuOpen = false;
    void setOptionMenu(bool open);

    bool hasEnded() const;
    const ActionMap& getActionMap() const;
    void drawLine(const Vec2& p1, const Vec2& p2);
};

