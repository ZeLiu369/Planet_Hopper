///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_Play.h
// 
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/

#pragma once
                                                     
#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"
                                                     
class Scene_Play : public Scene
{
    struct PlayerConfig
    {
        float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
        std::string WEAPON;
    };

protected:

    std::shared_ptr<Entity> m_player;
    std::map<std::string, std::vector<std::shared_ptr<Entity>>> m_backgroundsMap;
    sf::RenderTexture       m_renderTexture;
    sf::Texture             m_lightTexture;
    std::string             m_levelPath;
    PlayerConfig            m_playerConfig;
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    bool                    m_inventory = false;
    bool                    m_drawGrid = false;
    bool                    m_night = false;
    const Vec2              m_gridSize = { 64, 64 };
    sf::Text                m_gridText;
    sf::Text                m_coinText;

    void init(const std::string & levelPath);

    void loadLevel(const std::string & filename);

    void update();
    void onEnd();
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);
    void spawnMoney(std::shared_ptr<Entity> entity);

    sf::Sprite getLightingSprite();

    Vec2 gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);
    
    void sDoAction(const Action& action);

    void sScroll();
    void sCamera();
    void sMovement();
    void sLifespan();
    void sAnimation();
    void sCollision();
    void sRender();

public:

    Scene_Play(GameEngine * gameEngine, const std::string & levelPath);
                                                     
};
                                                     
// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
