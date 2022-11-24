///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Final
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_Editor.h
// 
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/

#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

class Scene_Editor : public Scene
{
    struct PlayerConfig
    {
        float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
        std::string WEAPON;
    };

protected:

    std::shared_ptr<Entity> m_camera;
    std::string             m_levelPath;
    PlayerConfig            m_playerConfig;
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    int                     m_drawGrid = 2;
    const Vec2              m_gridSize = { 64, 64 };
    const Vec2              m_BOUNDARYNEG = { 0, 0 };
    const Vec2              m_BOUNDARYPOS = { 200, 50 };
    sf::Text                m_gridText;
    sf::Text                m_exampleText;

    void init(const std::string& levelPath);

    void loadLevel(const std::string& filename);

    void update();
    void onEnd();
    void spawnCamera();

    Vec2 gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);

    void sDoAction(const Action& action);

    void sMovement();
    void sLifespan();
    void sAnimation();
    void sCollision();
    void sRender();

public:

    Scene_Editor(GameEngine* gameEngine, const std::string& levelPath);

};
