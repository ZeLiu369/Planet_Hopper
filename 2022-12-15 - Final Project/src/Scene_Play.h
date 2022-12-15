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
    };

protected:

    std::shared_ptr<Entity> m_player;
    std::shared_ptr<Entity> m_inventoryEntity;
    std::map<std::string, std::vector<std::shared_ptr<Entity>>> m_backgroundsMap;
    sf::RenderTexture       m_renderTexture;
    sf::Texture             m_lightTexture;
    sf::Shader              electric_shader, bright_shader, speed_shader, rainbow_shader, red_shader;
    sf::Clock               time;
    std::string             m_levelPath;
    int                     m_levelEnd;
    int                     m_level;
    int                     m_transition = 0;
    PlayerConfig            m_playerConfig;
    Vec2                    m_prevCameraPos;
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    bool                    m_inventory = false;
    bool                    m_drawGrid = false;
    bool                    m_night = false;
    const Vec2              m_gridSize = { 64, 64 };
    int                     m_countdown = 150;
    int                     m_action = 0;
    sf::Text                m_gridText;
    sf::Text                m_weaponUIText;
    bool goal = false;
    std::string             m_levelMusic;

    std::shared_ptr<Entity> m_touchedPlatform;

    int                     m_invSelect = 0;

    Vec2                    m_mPos;

    void init(const std::string & levelPath);

    void loadLevel(const std::string & filename);

    sf::Text displayText(std::string, float x, float y);
    sf::Sprite displayBox(float x, float y);
    std::shared_ptr<Entity> setupBullet(Vec2 size, Vec2 pos, int lifetime, int dmg, Vec2 speed, std::string name);
    void loadBoss();
    void drawWeapon();
    void drawWeaponDisplay();
    void updateBackgrounds();
    void update();
    void onEnd();
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);

    sf::Sprite getLightingSprite(sf::Sprite gaugeSprite);

    Vec2 windowToWorld(const Vec2& window) const;
    Vec2 gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);
    
    void sDoAction(const Action& action);

    void sInventory(std::string action, std::string name, int index);
    void sAI();
    void sCamera();
    void sClick();
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
