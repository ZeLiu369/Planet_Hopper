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

    struct LevelConfig
    {
        bool DARK;
        std::string MUSIC, BACKGROUND, NAME;
    };


protected:

    std::shared_ptr<Entity> m_camera;
    std::string             m_levelPath;
    PlayerConfig            m_playerConfig;
    LevelConfig             m_levelConfig;

    // drawing
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    int                     m_drawGrid = 2;
    bool                    m_drawCamera = true;

    // grid
    const Vec2              m_gridSize = { 64, 64 };
    const Vec2              m_BOUNDARYNEG = { 0, 0 };
    const Vec2              m_BOUNDARYPOS = { 200, 12 };

    // text
    sf::Text                m_gridText;
    sf::Text                m_controlText;

    // canera
    int                     m_CAMERA_SPEED = 5;
    std::string             m_CAMERA_AVATAR = "Coin";
    Vec2                    m_BOUND_BOX = Vec2(4, 4);

    // mouse
    Vec2                    m_mPos = { 0, 0 };
    bool                    m_drop = false;
    bool                    m_place = false;
    bool                    m_texture = true;
    std::shared_ptr<Entity> m_selected = NULL;

    // animation list
    std::vector<std::vector<std::string>>m_animations =
    {
        // Tile List
        {"Ground", "Brick", "Question", "Block", "PipeTall", "Pole","PoleTop"},
        // Dec list
        {"BushBig", "CloudSmall", "CloudBig", "Flag", "Coin", "Pole"},
        // NPC list
        {}
    };

    std::map<std::string, std::vector<std::string>>m_levelAssetList =
    {
        {"Music", {"MusicTitle", "Play", "OverWorld"}},
        {"Background", {"Background1", "Background2", "Background3", "None"}}
    };

    void init(const std::string& levelPath);

    void loadLevel(const std::string& filename);

    void update();
    void onEnd();

    void spawnPlayer();
    void spawnCamera();

    bool pasteEntity(std::shared_ptr<Entity> entity);

    Vec2 windowToWorld(const Vec2& wpos) const;

    Vec2 gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);
    Vec2 midPixelToGrid(std::shared_ptr<Entity> entity);

    bool snapToGrid(std::shared_ptr<Entity> entity);

    void sDoAction(const Action& action);

    void sState();
    void sMovement();
    void sLifespan();
    void sAnimation();
    void sCollision();
    void sRender();

public:

    Scene_Editor(GameEngine* gameEngine, const std::string& levelPath);

};
