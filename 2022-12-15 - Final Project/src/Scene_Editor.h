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

    std::shared_ptr<Entity> m_player;
    std::shared_ptr<Entity> m_camera;
    PlayerConfig            m_playerConfig;
    LevelConfig             m_levelConfig;

    // drawing
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    int                     m_drawGrid = 2;
    bool                    m_drawCamera = true;
    bool                    m_help = true;

    // grid
    const Vec2              m_gridSize = { 64, 64 };
    const Vec2              m_BOUNDARYNEG = { 0, 0 };
    const Vec2              m_BOUNDARYPOS = { 200, 30 };

    // text
    sf::Text                m_gridText;
    sf::Text                m_controlText;
    sf::Text                m_selectionText;
    sf::Text                m_buttonText;

    // camera
    int                     m_CAMERA_SPEED = 5;
    std::string             m_CAMERA_AVATAR = "Bomb";
    Vec2                    m_BOUND_BOX = Vec2(6, 6);

    // mouse
    Vec2                    m_mPos = { 0, 0 };
    bool                    m_drop = false;
    bool                    m_place = false;
    std::shared_ptr<Entity> m_selected = NULL;

    // Patrol point
    bool                    m_patrol;

    // animation list
    std::vector<std::string> m_aniAssets = {};

    // entity types for menu
    std::vector<std::string> m_entityTypes
    {
        "tile", "dec", "item", "npc"
    };

    // menus
    int m_menuSelection = 0;
    int m_pageSelection = 0;
    int m_saveLimit = 10;

    // level properties
    std::map<std::string, std::vector<std::string>>m_levelAssetList =
    {
        {"Music", {"MusicTitle", "Play", "OverWorld"}},
        {"Background", {"Background1", "Background2", "Background3", "None"}}
    };

    void init();

    void fillAssetList();

    std::string formatFloat(float f);

    void loadBlankLevel();
    void loadLevel(const std::string& filename);
    void saveLevel();

    void update();
    void onEnd();

    void spawnPlayer();
    void spawnCamera();

    bool pasteEntity(std::shared_ptr<Entity> entity);

    Vec2 windowToWorld(const Vec2& wpos) const;

    Vec2 gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);
    Vec2 midPixelToGrid(std::shared_ptr<Entity> entity);
    Vec2 midPixelToGrid(std::shared_ptr<Entity> entity, Vec2& pos);

    bool snapToGrid(std::shared_ptr<Entity> entity);
    bool snapToGrid(std::shared_ptr<Entity> entity, Vec2& point);

    void showEntityPage(int page);
    void clearMenu();
    void showSLMenu();

    std::shared_ptr<Entity> createEntity(std::string animation);

    void sDoAction(const Action& action);

    void sState();
    void sMovement();
    void sAnimation();
    void sCollision();
    void sRender();

public:

    Scene_Editor(GameEngine* gameEngine);

};
