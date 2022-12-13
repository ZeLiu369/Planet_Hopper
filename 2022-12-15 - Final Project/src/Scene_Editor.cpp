///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - final
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_Editor.cpp
// 
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): Nathan
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/

#include "Scene_Editor.h"
#include "Scene_Menu.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <cmath>

Scene_Editor::Scene_Editor(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
}

void Scene_Editor::init()
{
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");      // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");    // Toggle drawing (C)ollision Boxes
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");         // Toggle drawing (G)rid
    registerAction(sf::Keyboard::F, "TOGGLE_CAMERA");       // Toggle drawing (F)amera
    registerAction(sf::Keyboard::BackSpace, "DELETE_MODE"); // Toggle (Delete) mode
    registerAction(sf::Keyboard::H, "HELP");                // Toggle text

    registerAction(sf::Keyboard::P, "PLACE_POINT");
    registerAction(sf::Keyboard::O, "CLEAR_POINTS");

    registerAction(sf::Keyboard::Num8, "MUSIC");
    registerAction(sf::Keyboard::Num9, "BACKGROUND");
    registerAction(sf::Keyboard::Num0, "DARK");
    registerAction(sf::Keyboard::Q, "ENTITY_MENU");
    registerAction(sf::Keyboard::Tab, "SAVE/LOAD_MENU");
    registerAction(sf::Keyboard::M, "MUTE");

    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::LShift, "FAST");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("ChunkFive"));

    m_controlText.setCharacterSize(24);
    m_controlText.setFont(m_game->assets().getFont("ChunkFive"));

    m_selectionText.setCharacterSize(48);
    m_selectionText.setFont(m_game->assets().getFont("ChunkFive"));

    m_buttonText.setCharacterSize(24);
    m_buttonText.setFont(m_game->assets().getFont("ChunkFive"));

    m_modText.setCharacterSize(48);
    m_modText.setFont(m_game->assets().getFont("ChunkFive"));

    setUpSounds();

    fillAssetList();
    loadBlankLevel();
}

Vec2 Scene_Editor::windowToWorld(const Vec2& window) const
{
    auto view = m_game->window().getView();

    float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
    float wy = view.getCenter().y - (m_game->window().getSize().y / 2);

    return { window.x + wx, window.y + wy };
}

Vec2 Scene_Editor::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    Vec2 size = (entity->hasComponent<CBoundingBox>() ? 
        entity->getComponent<CBoundingBox>().size : 
        entity->getComponent<CAnimation>().animation.getSize());

    return Vec2(
        (gridX * m_gridSize.x) + size.x / 2,
        (m_BOUNDARYPOS.y - gridY) * m_gridSize.y - (size.y / 2)
    );
}

Vec2 Scene_Editor::midPixelToGrid(std::shared_ptr<Entity> entity)
{
    Vec2 size = (entity->hasComponent<CBoundingBox>() ?
        entity->getComponent<CBoundingBox>().size :
        entity->getComponent<CAnimation>().animation.getSize());

    Vec2 pos = entity->getComponent<CTransform>().pos;

    return Vec2(
        (pos.x - (size.x / 2)) / m_gridSize.x,
        m_BOUNDARYPOS.y - ((pos.y + (size.y / 2)) / m_gridSize.y)
    );
}

Vec2 Scene_Editor::midPixelToGrid(std::shared_ptr<Entity> entity, Vec2& pos)
{
    Vec2 size = (entity->hasComponent<CBoundingBox>() ?
        entity->getComponent<CBoundingBox>().size :
        entity->getComponent<CAnimation>().animation.getSize());

    return Vec2(
        (pos.x - (size.x / 2)) / m_gridSize.x,
        m_BOUNDARYPOS.y - ((pos.y + (size.y / 2)) / m_gridSize.y)
    );
}

// makes sure entity is alligned to grid
// won't place if a certain entity is overlaping another or if out of bounds
bool Scene_Editor::snapToGrid(std::shared_ptr<Entity> entity)
{
    Vec2& ePos = entity->getComponent<CTransform>().pos;
    Vec2 snap = gridToMidPixel(floor(ePos.x / m_gridSize.x), (m_BOUNDARYPOS.y - 1) - floor(ePos.y / m_gridSize.y), entity);
    ePos = snap;

    for (auto& e : m_entityManager.getEntities())
    {
        if (e == entity || e->tag() == "camera") continue;

        if (entity->tag() == "dec" && e->tag() == "dec" && ePos == e->getComponent<CTransform>().pos) return false;

        Vec2 o = Physics::GetOverlap(entity, e);
        if (o.x > 0 && o.y > 0) return false;

    }
    

    Vec2 absolutePos = { m_BOUNDARYPOS.x * m_gridSize.x , m_BOUNDARYPOS.y * m_gridSize.y };
    Vec2 absoluteNeg = { m_BOUNDARYNEG.x * m_gridSize.x , m_BOUNDARYNEG.y * m_gridSize.y };

    // out of bounds
    if (snap.x < absoluteNeg.x || snap.x > absolutePos.x ||
        snap.y < absoluteNeg.y || snap.y > absolutePos.y)
    {
        return false;
    }

    return true;
}
bool Scene_Editor::snapToGrid(std::shared_ptr<Entity> entity, Vec2& point)
{
    Vec2& ePos = entity->getComponent<CTransform>().pos;
    Vec2 pos = ePos;

    Vec2 snap = gridToMidPixel(floor(point.x / m_gridSize.x), (m_BOUNDARYPOS.y - 1) - floor(point.y / m_gridSize.y), entity);
    point = snap;
    ePos = point;

    for (auto& e : m_entityManager.getEntities())
    {
        if (e == entity || e->tag() != "tile") continue;

        Vec2 o = Physics::GetOverlap(entity, e);
        if (o.x > 0 && o.y > 0)
        {
            ePos = pos;
            return false;
        }
    }


    Vec2 absolutePos = { m_BOUNDARYPOS.x * m_gridSize.x , m_BOUNDARYPOS.y * m_gridSize.y };
    Vec2 absoluteNeg = { m_BOUNDARYNEG.x * m_gridSize.x , m_BOUNDARYNEG.y * m_gridSize.y };

    // out of bounds
    if (snap.x < absoluteNeg.x || snap.x > absolutePos.x ||
        snap.y < absoluteNeg.y || snap.y > absolutePos.y)
    {
        ePos = pos;
        return false;
    }

    ePos = pos;
    return true;
}

void Scene_Editor::setUpSounds()
{
    // music
    for (std::string m : m_levelAssetList["Music"])
    {
        m_game->assets().getMusic(m).setLoop(true);
    }
}

void Scene_Editor::fillAssetList()
{
    std::vector<std::string> BLACK_LIST =
    {
        "Inventory", "Sky", "Stars", "Hill", "Land", "Craters", "SkyObj", "Rock", "Space",
        "Hill2", "Land2", "Craters2", "Sky2", "SkyObj2", "Rock2", "Gauge"
    };

    std::string ASSET_FILE = "assets.txt";

    std::ifstream fin(ASSET_FILE);
    std::string temp;

    while (fin >> temp)
    {
        if (temp == "Animation")
        {
            fin >> temp;
            if (std::find(BLACK_LIST.begin(), BLACK_LIST.end(), temp) != BLACK_LIST.end())
            {
                fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            else
            {
                m_aniAssets.push_back(temp);
            }
            fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// remove floating point annoyance in txt
std::string Scene_Editor::formatFloat(float f)
{
    // round to 2 decimals
    f = std::ceil(f * 100.0) / 100.0;

    // convert to string
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << f;
    std::string str = ss.str();

    // remove any trailing 0's
    str = str.substr(0, str.find_last_not_of('0') + 1);
    if (str.find('.') == str.size() - 1)
    {
        str = str.substr(0, str.size() - 1);
    }
    return(str);
}

void Scene_Editor::loadBlankLevel()
{
    m_entityManager = EntityManager();

    m_game->assets().getMusic("MusicTitle").stop();

    LevelConfig& lc = m_levelConfig;
    lc.MUSIC = "Play";
    lc.DARK = false;
    lc.BACKGROUND = "None";
    lc.NAME = "level1.txt";

    PlayerConfig& pc = m_playerConfig;
    pc.X = 0; pc.Y = 0; pc.CX = 46; pc.CY = 70; pc.SPEED = 5; pc.JUMP = -20; pc.MAXSPEED = 20; pc.GRAVITY = 0.75;

    spawnPlayer();
    spawnCamera();
    m_game->playMusic(m_levelConfig.MUSIC);
}

void Scene_Editor::loadLevel(const std::string& filename)
{
    // reset the entity manager every time we load a level
    m_selected = NULL;
    m_entityManager = EntityManager();

    m_levelConfig.MUSIC = "Play";
    m_levelConfig.DARK = false;
    m_levelConfig.BACKGROUND = "None";
    m_levelConfig.NAME = filename.substr(13);
    std::ifstream fin(filename);
    std::string temp;

    while (fin >> temp)
    {
        if (temp == "Lighting")
        {
            fin >> temp;
            if (temp == "Night")
            {
                m_levelConfig.DARK = true;
            }
        }
        else if (temp == "BackgroundType")
        {
            fin >> m_levelConfig.BACKGROUND;
        }
        else if (temp == "Music")
        {
            fin >> m_levelConfig.MUSIC;
        }

        else if (temp == "Dec" || temp == "Tile")
        {
            // tile and decs
            std::string type = temp;
            std::string texture;
            float x, y;
            fin >> texture >> x >> y;

            auto tile = m_entityManager.addEntity(type == "Dec" ? "dec" : "tile");
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
            tile->addComponent<CTransform>(gridToMidPixel(x, y, tile));

            if (type == "Tile")
            {
                int m, v, damage;
                float speed;
                fin >> m >> v >> damage >> speed;
     
                tile->addComponent<CBoundingBox>(tile->getComponent<CAnimation>().animation.getSize(),m,v);

                tile->addComponent<CDamage>(damage);

                if (speed > 0)
                {
                    int points;
                    fin >> points;

                    std::vector<Vec2> pos;

                    for (int i = 0; i < points; i++)
                    {
                        int pointX, pointY;
                        fin >> pointX >> pointY;
                        pos.push_back(gridToMidPixel(pointX, pointY, tile));
                    }

                    tile->addComponent<CPatrol>(pos, speed);
                }

            }

            tile->addComponent<CDraggable>();
        }

        // items, player, and npc
        else if (temp == "Item")
        {
            std::string tileName;
            float x, y;
            fin >> tileName >> x >> y;

            auto item = m_entityManager.addEntity("item");

            item->addComponent<CAnimation>(m_game->assets().getAnimation(tileName), true);
            item->addComponent<CTransform>(gridToMidPixel(x, y, item));
            item->addComponent<CBoundingBox>(m_game->assets().getAnimation(tileName).getSize());
            item->addComponent<CDraggable>();
        }
        else if (temp == "Npc")
        {
            std::string ani;
            float x, y;
            int health, damage;
            float scale, speed, jump, gravity;
            fin >> ani >> x >> y >> scale >> health >> damage >> jump >> gravity >> speed;

            auto npc = m_entityManager.addEntity("npc");

            npc->addComponent<CAnimation>(m_game->assets().getAnimation(ani), true);
            npc->addComponent<CTransform>(gridToMidPixel(x, y, npc));
            npc->addComponent<CBoundingBox>(m_game->assets().getAnimation(ani).getSize());

            npc->getComponent<CTransform>().scale.x = scale;

            npc->addComponent<CHealth>(health, health);
            npc->addComponent<CDamage>(damage);
            npc->addComponent<CGravity>(gravity);

            fin >> temp;

            if (temp == "Follow")
            {
                npc->addComponent<CFollowPlayer>(gridToMidPixel(x, y, npc), speed);
            }
            else if (temp == "Patrol")
            {
                int points;
                fin >> points;

                std::vector<Vec2> pos;

                for (int i = 0; i < points; i++)
                {
                    int pointX, pointY;
                    fin >> pointX >> pointY;
                    pos.push_back(gridToMidPixel(pointX, pointY, npc));
                }

                npc->addComponent<CPatrol>(pos, speed);
            }

            npc->addComponent<CDraggable>();
        }
        else if (temp == "Player")
        {
            PlayerConfig& pc = m_playerConfig;
            fin >> pc.X >> pc.Y >> pc.CX >> pc.CY >> pc.SPEED
                >> pc.JUMP >> pc.MAXSPEED >> pc.GRAVITY;
        }
        else
        {
            fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    fin.close();
    spawnPlayer();
    spawnCamera();

    if (!m_mute)
    {
        m_game->playMusic(m_levelConfig.MUSIC);
    }
    std::cout << m_levelConfig.NAME + " loaded!\n";
}

void Scene_Editor::saveLevel()
{
    LevelConfig& lc = m_levelConfig;

    std::ofstream fout("customLevels/" + lc.NAME);
    std::string saveLine = "";

    // World Properties
    // Lighting
    saveLine = "Lighting " + std::string(lc.DARK ? "Night" : "Day");
    fout << saveLine << std::endl;

    // Music
    saveLine = "Music " + lc.MUSIC;
    fout << saveLine << std::endl;

    // Background
    saveLine = "BackgroundType " + lc.BACKGROUND;
    fout << saveLine << std::endl;

    if (lc.BACKGROUND == m_levelAssetList["Background"][0])
    {
        saveLine = "Background Sky     0 1.25 1.5  640 384" + std::string("\n") +
            "Background Stars   0 1.25 1.65 640 384" + std::string("\n") +
            "Background SkyObj  1 1    1    640 200 0.85" + std::string("\n") +
            "Background Hill    1 1.25 1.75 640 554 0.50" + std::string("\n") +
            "Background Rock    1 1    1    640 584 0.10" + std::string("\n") +
            "Background Land    1 1.25 1    640 717 0.02" + std::string("\n") +
            "Background Craters 1 1    1    640 717 0.01";
        fout << saveLine << std::endl;
    }
    else if (lc.BACKGROUND == m_levelAssetList["Background"][1])
    {
        saveLine = "Background Sky2     0 1.25 1.5  640 384" + std::string("\n") +
            "Background Stars   0 1.25 1.65 640 384" + std::string("\n") +
            "Background SkyObj2  1 1    1    640 200 0.85" + std::string("\n") +
            "Background Hill2    1 1.25 1.75 640 554 0.50" + std::string("\n") +
            "Background Rock2    1 1    1    640 584 0.10" + std::string("\n") +
            "Background Land2    1 1.25 1    640 717 0.02" + std::string("\n") +
            "Background Craters2 1 1    1    640 717 0.01";
        fout << saveLine << std::endl;
    }
    else if (lc.BACKGROUND == m_levelAssetList["Background"][2])
    {
        saveLine = "Background Sky     0 1.25 1.5  640 384" + std::string("\n") +
            "Background Stars   0 1.25 1.65 640 384" + std::string("\n") +
            "Background SkyObj2  1 1    1    640 200 0.85" + std::string("\n") +
            "Background Hill2    1 1.25 1.75 640 554 0.50" + std::string("\n") +
            "Background Rock    1 1    1    640 584 0.10" + std::string("\n") +
            "Background Land2    1 1.25 1    640 717 0.02" + std::string("\n") +
            "Background Craters2 1 1    1    640 717 0.01";
        fout << saveLine << std::endl;
    }

    // Player
    PlayerConfig& pc = m_playerConfig;
    Vec2 grid = midPixelToGrid(m_player);

    pc.X = grid.x; pc.Y = grid.y;
    pc.CX = m_player->getComponent<CBoundingBox>().size.x; pc.CY = m_player->getComponent<CBoundingBox>().size.y;
    pc.GRAVITY = m_player->getComponent<CGravity>().gravity;
    pc.JUMP = m_player->getComponent<CJump>().jump;

    // I made the player pos int because they like to scoot around when saving/loading
    saveLine = "Player " + std::to_string((int)pc.X) + " " + std::to_string((int)pc.Y) +
        " " + formatFloat(pc.CX) + " " + formatFloat(pc.CY) + " " +
        formatFloat(pc.SPEED) + " " + formatFloat(pc.JUMP) + " " + formatFloat(pc.MAXSPEED) +
        " " + formatFloat(pc.GRAVITY);

    fout << saveLine << std::endl;

    // level content

    for (auto& e : m_entityManager.getEntities())
    {
        if (!e->hasComponent<CTransform>() || !e->hasComponent<CAnimation>() || 
             e->tag() == "camera" || e->tag() == "button" || e->tag() == "player" || e->tag() == "dec") continue;

        CTransform& t = e->getComponent<CTransform>();
        CAnimation& a = e->getComponent<CAnimation>();
        
        char u[1];
        u[0] = e->tag()[0];
        u[0] = toupper(u[0]);
        std::string tag = u[0] + e->tag().substr(1);
        
        grid = midPixelToGrid(e);

        saveLine = tag + " " + a.animation.getName() + " " + formatFloat(grid.x) + " " + formatFloat(grid.y);

        if (e->tag() == "tile")
        {
            saveLine = saveLine + " " + std::to_string(e->getComponent<CBoundingBox>().blockMove) +
                " " + std::to_string(e->getComponent<CBoundingBox>().blockVision) +
                " " + std::to_string(e->getComponent<CDamage>().damage);

            if (e->hasComponent<CPatrol>())
            {
                saveLine = saveLine + " " + formatFloat(e->getComponent<CPatrol>().speed);
                std::vector<Vec2> points = e->getComponent<CPatrol>().positions;
                saveLine = saveLine + " " + std::to_string(points.size());

                for (Vec2& p : points)
                {
                    Vec2 g = midPixelToGrid(e, p);
                    saveLine = saveLine + " " + formatFloat(g.x) + " " + formatFloat(g.y);
                }
            }
            else
            {
                saveLine = saveLine + " 0";
            }
        }
        else if (e->tag() == "item")
        {

        }
        else if (e->tag() == "npc")
        {
            saveLine = saveLine + " " + formatFloat(t.scale.x);

            saveLine = saveLine + " " + std::to_string(e->getComponent<CHealth>().max) +
                                  " " + std::to_string(e->getComponent<CDamage>().damage);

            saveLine = saveLine + " " + formatFloat(e->getComponent<CJump>().jump) +
                                  " " + formatFloat(e->getComponent<CGravity>().gravity);

            if (e->hasComponent<CFollowPlayer>())
            {
                saveLine = saveLine + " " + formatFloat(e->getComponent<CFollowPlayer>().speed) + " Follow";
            }
            else if (e->hasComponent<CPatrol>())
            {
                std::vector<Vec2> points = e->getComponent<CPatrol>().positions;
                saveLine = saveLine + " " + formatFloat(e->getComponent<CPatrol>().speed) + " Patrol " +
                                            std::to_string(points.size());
                for (Vec2& p : points)
                {
                    Vec2 g = midPixelToGrid(e,p);
                    saveLine = saveLine + " " + formatFloat(g.x) + " " + formatFloat(g.y);
                }
            }
        }
        fout << saveLine << std::endl;
    }

    // dec is saved last so they load in front of tiles
    for (auto& d : m_entityManager.getEntities("dec"))
    {
        CTransform& t = d->getComponent<CTransform>();
        CAnimation& a = d->getComponent<CAnimation>();

        grid = midPixelToGrid(d);

        saveLine = "Dec " + a.animation.getName() + " " + formatFloat(grid.x) + " " + formatFloat(grid.y);
        fout << saveLine << std::endl;
    }

    fout.close();
    m_game->playSound("winSound");
    std::cout << lc.NAME + " saved!\n";
}

void Scene_Editor::spawnPlayer()
{
    PlayerConfig& pc = m_playerConfig;

    // here is a sample player entity which you can use to construct other entities
    auto player = m_entityManager.addEntity("player");
    player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);

    Vec2 spawnPos = gridToMidPixel(pc.X, pc.Y, player);
    player->addComponent<CTransform>(spawnPos);
    player->addComponent<CBoundingBox>(Vec2(pc.CX, pc.CY));
    player->addComponent<CGravity>(pc.GRAVITY);
    player->addComponent<CJump>(pc.JUMP);

    player->addComponent<CDraggable>();
    m_player = player;
}

void Scene_Editor::spawnCamera()
{
    PlayerConfig& pc = m_playerConfig;

    m_camera = m_entityManager.addEntity("camera");
    m_camera->addComponent<CAnimation>(m_game->assets().getAnimation(m_CAMERA_AVATAR), true);

    Vec2 spawnPos = gridToMidPixel(pc.X, pc.Y, m_camera);
    m_camera->addComponent<CTransform>(spawnPos);
    m_camera->addComponent<CBoundingBox>(m_BOUND_BOX);

    m_camera->addComponent<CInput>();
    m_camera->addComponent<CState>("move");
}

bool Scene_Editor::pasteEntity(std::shared_ptr<Entity> e)
{
    auto entity = m_entityManager.addEntity(e->tag());
    entity->addComponent<CTransform>() = e->getComponent<CTransform>();
    entity->addComponent<CDraggable>();

    if (e->hasComponent<CBoundingBox>()) entity->addComponent<CBoundingBox>() = e->getComponent<CBoundingBox>();
    if (e->hasComponent<CAnimation>()) entity->addComponent<CAnimation>() = e->getComponent<CAnimation>();
    if (e->hasComponent<CGravity>()) entity->addComponent<CGravity>() = e->getComponent<CGravity>();
    if (e->hasComponent<CPatrol>()) entity->addComponent<CPatrol>() = e->getComponent<CPatrol>();
    if (e->hasComponent<CFollowPlayer>()) entity->addComponent<CFollowPlayer>() = e->getComponent<CFollowPlayer>();
    if (e->hasComponent<CHealth>()) entity->addComponent<CHealth>() = e->getComponent<CHealth>();
    if (e->hasComponent<CDamage>()) entity->addComponent<CDamage>() = e->getComponent<CDamage>();
    if (e->hasComponent<CJump>()) entity->addComponent<CJump>() = e->getComponent<CJump>();

    entity->getComponent<CTransform>().pos = windowToWorld(m_mPos);
    if (snapToGrid(entity))
    {
        return(true);
    }
    else
    {
        entity->destroy();
        return(false);
    }
}

void Scene_Editor::showEntityPage(int page)
{
    clearMenu();
    Vec2& camPos = m_camera->getComponent<CTransform>().pos;

    Vec2 startCorner = { camPos.x - (width() / 2), camPos.y - (height() / 2) };
    Vec2 endCorner = { startCorner.x + width(), startCorner.y + height() };
    int x = startCorner.x + m_gridSize.x;
    int y = startCorner.y + m_gridSize.y*3;

    for (int i = (112*page); i < (112*page)+113 && i < m_aniAssets.size(); i++)
    {
        auto b = m_entityManager.addEntity("button");
        b->addComponent<CTransform>(Vec2(x, y));
        b->addComponent<CAnimation>(m_game->assets().getAnimation(m_aniAssets[i]), true);
        b->addComponent<CBoundingBox>(Vec2( m_gridSize.x, m_gridSize.y ));

        Vec2 aniSize = b->getComponent<CAnimation>().animation.getSize();
        b->getComponent<CTransform>().scale = Vec2{ m_gridSize.x / aniSize.x, m_gridSize.y / aniSize.y };

        x += m_gridSize.x + 10;
        if (x > endCorner.x - m_gridSize.x) { x = startCorner.x + m_gridSize.x; y += (m_gridSize.y + 10); }
    }

}

void Scene_Editor::clearMenu()
{
    for (auto e : m_entityManager.getEntities("button"))
    {
        e->destroy();
    }
}

void Scene_Editor::showSLMenu()
{
    Vec2& camPos = m_camera->getComponent<CTransform>().pos;

    for (int i = 0; i < 2; i++)
    {
        auto b = m_entityManager.addEntity("button");
        b->addComponent<CTransform>(Vec2(camPos.x + (m_gridSize.x * 2.5 * (i == 0 ? -1 : 1)), camPos.y));
        b->addComponent<CAnimation>(m_game->assets().getAnimation("Plat"), true);
        b->addComponent<CBoundingBox>(Vec2(m_gridSize.x * 2, m_gridSize.y));

        Vec2 aniSize = b->getComponent<CAnimation>().animation.getSize();
        b->getComponent<CTransform>().scale = Vec2{ (m_gridSize.x * 2) / aniSize.x, m_gridSize.y / aniSize.y };

        b->addComponent<CButton>(i == 0 ? "save" : "load");
    }
}

std::shared_ptr<Entity> Scene_Editor::createEntity(std::string animation)
{
    auto e = m_entityManager.addEntity(m_entityTypes[m_menuSelection]);

    e->addComponent<CTransform>(windowToWorld(m_mPos));
    e->addComponent<CAnimation>(m_game->assets().getAnimation(animation), true);
    e->addComponent<CDraggable>();

    if (e->tag() == "tile")
    {
        e->addComponent<CBoundingBox>(e->getComponent<CAnimation>().animation.getSize(),1,1);
        e->addComponent<CDamage>(0);
    }
    else if (e->tag() == "item")
    {
        e->addComponent<CBoundingBox>(e->getComponent<CAnimation>().animation.getSize());
    }
    else if (e->tag() == "npc")
    {
        e->addComponent<CBoundingBox>(e->getComponent<CAnimation>().animation.getSize());
        e->addComponent<CDamage>(1);
        e->addComponent<CHealth>(1,1);
        e->addComponent<CJump>(-4);
        e->addComponent<CGravity>(0.75);
        e->addComponent<CFollowPlayer>(Vec2(0, 0),2);
    }

    return e;
}

void Scene_Editor::changeValue(int& i, int c, int min, int max)
{
    i += c;
    if (i > max) i = min;
    if (i < min) i = max;
}

void Scene_Editor::changeValue(float& i, int c, int min, int max, float g)
{
    i += c*g;
    if (i > max*g) i = min*g;
    if (i < min*g) i = max*g;
}
    

void Scene_Editor::modConfig()
{
    std::string value = m_modTypes[m_selected->tag()][m_pageSelection];
    int change = 0;
    if (m_camera->getComponent<CInput>().left && m_modDecrease)
    {
        m_modDecrease = false;
        change = -1;
    }
    else if (m_camera->getComponent<CInput>().right && m_modIncrease)
    {
        m_modIncrease = false;
        change = 1;
    }
    
    if (change != 0)
    {
        if (value == "Block vision") m_selected->getComponent<CBoundingBox>().blockVision = !m_selected->getComponent<CBoundingBox>().blockVision;
        else if (value == "Block move") m_selected->getComponent<CBoundingBox>().blockMove = !m_selected->getComponent<CBoundingBox>().blockMove;

        else if (value == "Health") changeValue(m_selected->getComponent<CHealth>().max, change, m_minMax[value][0], m_minMax[value][1]);
        else if (value == "Damage") changeValue(m_selected->getComponent<CDamage>().damage, change, m_minMax[value][0], m_minMax[value][1]);

        else if (value == "Jump") changeValue(m_selected->getComponent<CJump>().jump, -change, m_minMax[value][0], m_minMax[value][1], 1.0);
        else if (value == "Gravity") changeValue(m_selected->getComponent<CGravity>().gravity, change, m_minMax[value][0], m_minMax[value][1],0.75);

        else if (value == "Speed")
        {
            if (m_selected->tag() == "player") changeValue(m_playerConfig.SPEED, change, m_minMax[value][0], m_minMax[value][1], 1.0);
            else if (m_selected->tag() == "npc")
            {
                if (m_selected->hasComponent<CPatrol>()) 
                    changeValue(m_selected->getComponent<CPatrol>().speed, change, m_minMax[value][0], m_minMax[value][1], 1.0);
                if (m_selected->hasComponent<CFollowPlayer>()) 
                    changeValue(m_selected->getComponent<CFollowPlayer>().speed, change, m_minMax[value][0], m_minMax[value][1], 1.0);
            }
            else if (m_selected->tag() == "tile")
            {
                if (m_selected->hasComponent<CPatrol>())
                {
                    changeValue(m_selected->getComponent<CPatrol>().speed, change, m_minMax[value][0], m_minMax[value][1], 1.0);
                    if (m_selected->getComponent<CPatrol>().speed == 0) m_selected->removeComponent<CPatrol>();
                }
                else
                {
                    m_selected->addComponent<CPatrol>();
                    m_selected->getComponent<CPatrol>().positions = {};
                    changeValue(m_selected->getComponent<CPatrol>().speed, change, m_minMax[value][0], m_minMax[value][1], 1.0);
                }
            }
        }

        else if (value == "AI")
        {
            float s;
            if (m_selected->hasComponent<CPatrol>())
            {
                float s = m_selected->getComponent<CPatrol>().speed;
                m_selected->removeComponent<CPatrol>();
                m_selected->addComponent<CFollowPlayer>(Vec2(0, 0), s);
            }
            else if (m_selected->hasComponent<CFollowPlayer>())
            {
                float s = m_selected->getComponent<CFollowPlayer>().speed;
                m_selected->removeComponent<CFollowPlayer>();
                m_selected->addComponent<CPatrol>();
                m_selected->getComponent<CPatrol>().speed = s;
            }
        }
    }
}

void Scene_Editor::update()
{
    m_entityManager.update();

    std::string& state = m_camera->getComponent<CState>().state;

    sState();
    if (state != "entity" && state != "sl" && state != "mod") sMovement();
    sCollision();
    sAnimation();
    sRender();

    m_currentFrame++;
}

void Scene_Editor::sState()
{

    CState& state = m_camera->getComponent<CState>();
    CInput& input = m_camera->getComponent<CInput>();

    // can drag if in move state
    if (state.state == "move")
    {
        if (input.click1 && !m_place)
        {
            std::shared_ptr<Entity> target = NULL;

            for (auto e : m_entityManager.getEntities("dec"))
            {
                if (e->hasComponent<CDraggable>() && Physics::IsInside(windowToWorld(m_mPos), e))
                {
                    if (!e->getComponent<CDraggable>().dragging)
                    {
                        target = e; break;
                    }
                }
            }
            if (target == NULL)
            {
                for (auto e : m_entityManager.getEntities())
                {
                    if (e->hasComponent<CDraggable>() && Physics::IsInside(windowToWorld(m_mPos), e))
                    {
                        if (!e->getComponent<CDraggable>().dragging)
                        {
                            target = e; break;
                        }
                    }
                }
            }
            if (target != NULL)
            {
                m_game->playSound("throw");
                target->getComponent<CDraggable>().dragging = true;
                m_selected = target;
                state.state = "drag";
                m_drop = false;
            }
        }
        else if (input.click2 && m_selected != NULL && m_selected->tag() != "player")
        {
            pasteEntity(m_selected);
        }
        else if (input.click3)
        {
            for (auto e : m_entityManager.getEntities())
            {
                if (e->hasComponent<CDraggable>() && Physics::IsInside(windowToWorld(m_mPos), e) &&
                    e->tag() != "dec" && e->tag() != "item")
                {
                    m_selected = e;
                    m_pageSelection = 0;
                    state.state = "mod";
                }
            }
        }
    }
    // can place and change to move during drag
    else if (state.state == "drag")
    {
        if (input.click1 && m_drop)
        {
            if (snapToGrid(m_selected))
            {
                m_selected->getComponent<CDraggable>().dragging = false;
                state.state = "move";
                m_place = true;
            }
        }
    }
    // deletes any non important entities in cursor pos
    else if (state.state == "delete" && input.click2)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CDraggable>() && Physics::IsInside(windowToWorld(m_mPos), e) && e->tag() != "player")
            {
                if (e == m_selected) m_selected = NULL;
                e->destroy();
            }
        }
    }
    // entity menu buttons, makes a new entity if button is clicked
    else if (state.state == "entity" && input.click1)
    {
        bool clicked = false;
        for (auto& e : m_entityManager.getEntities("button"))
        {
            if (Physics::IsInside(windowToWorld(m_mPos), e))
            {
                m_selected = createEntity(e->getComponent<CAnimation>().animation.getName());
                m_selected->getComponent<CDraggable>().dragging = true;
                clicked = true;
                break;
            }
        }
        if (clicked)
        {
            m_menuSelection = 0;
            clearMenu();
            state.state = "drag";
            m_drop = false;
        }
    }
    else if (state.state == "sl" && input.click1)
    {
        std::string choice = "";
        for (auto& e : m_entityManager.getEntities("button"))
        {
            if (Physics::IsInside(windowToWorld(m_mPos), e))
            {
                choice = e->getComponent<CButton>().value;
                break;
            }
        }
        if (choice != "")
        {
            if (choice == "save")
            {
                clearMenu();
                m_levelConfig.NAME = "level" + std::to_string(m_menuSelection + 1) + ".txt";
                saveLevel();
                state.state = "move";
                m_menuSelection = 0;
            }
            else if (choice == "load")
            {
                std::string fileName = ("customLevels/level" + std::to_string(m_menuSelection + 1) + ".txt");
                std::ifstream f(fileName.c_str());
                if (f.good() == true)
                {
                    clearMenu();
                    m_game->assets().getMusic(m_levelConfig.MUSIC).stop();
                    loadLevel(fileName);
                    m_menuSelection = 0;
                }
            }
        }
    }
    else if (state.state == "mod")
    {
        modConfig();
    }
}

void Scene_Editor::sMovement()
{
    CTransform& transform = m_camera->getComponent<CTransform>();
    CInput& input = m_camera->getComponent<CInput>();

    // camera movement \\

    // horizontal movement
    Vec2 dir = Vec2((input.right - input.left), (input.down - input.up));
    //transform.velocity = dir * m_CAMERA_SPEED;

    transform.velocity = dir * (m_CAMERA_SPEED + m_CAMERA_SPEED*m_fast);

    // updates prevPos and current pos
    transform.prevPos = transform.pos;
    transform.pos += transform.velocity;

    // Dragging \\

    // if entity is being dragged, move with mouse
    if (m_selected != NULL && m_selected->getComponent<CDraggable>().dragging)
    {
        m_selected->getComponent<CTransform>().pos = windowToWorld(m_mPos);
    }

}

void Scene_Editor::sCollision()
{

    // boundary collison
    CTransform& transform = m_camera->getComponent<CTransform>();
    CBoundingBox& box = m_camera->getComponent<CBoundingBox>();

    Vec2 absolutePos = { m_BOUNDARYPOS.x * m_gridSize.x , m_BOUNDARYPOS.y * m_gridSize.y };
    Vec2 absoluteNeg = { m_BOUNDARYNEG.x * m_gridSize.x , m_BOUNDARYNEG.y * m_gridSize.y };

    // when the camera travels too far 
    if (transform.pos.x < absoluteNeg.x) transform.pos.x = absolutePos.x - box.halfSize.x;
    if (transform.pos.x > absolutePos.x) transform.pos.x = absoluteNeg.x + box.halfSize.x;

    // when the camerea falls into the sky
    if (transform.pos.y < absoluteNeg.y) transform.pos.y = absolutePos.y - box.halfSize.y;
    if (transform.pos.y > absolutePos.y) transform.pos.y = absoluteNeg.y + box.halfSize.y;
}

void Scene_Editor::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = (m_drawGrid + 1) % 3; }
        else if (action.name() == "TOGGLE_CAMERA") { m_drawCamera = !m_drawCamera; }
        else if (action.name() == "QUIT") { onEnd(); }
        else if (action.name() == "HELP") { m_help = !m_help; }

        else if (action.name() == "UP") 
        { 
            m_camera->getComponent<CInput>().up = true; 
            if (m_camera->getComponent<CState>().state == "entity")
            {
                m_pageSelection--;
                if (m_pageSelection < 0) m_pageSelection = floor((m_aniAssets.size() - 1) / 112);
                showEntityPage(m_pageSelection);
            }
            else if (m_camera->getComponent<CState>().state == "mod")
            {
                m_pageSelection--;
                if (m_pageSelection < 0) m_pageSelection = (m_modTypes[m_selected->tag()].size() - 1);
            }
        }
        else if (action.name() == "DOWN") 
        { 
            m_camera->getComponent<CInput>().down = true;
            if (m_camera->getComponent<CState>().state == "entity")
            {
                m_pageSelection++;
                if (m_pageSelection > (int)floor((m_aniAssets.size() - 1) / 112)) m_pageSelection = 0;
                showEntityPage(m_pageSelection);
            }
            else if (m_camera->getComponent<CState>().state == "mod")
            {
                m_pageSelection++;
                if (m_pageSelection >= m_modTypes[m_selected->tag()].size()) m_pageSelection = 0;
            }
        }
        else if (action.name() == "LEFT") 
        { 
            m_camera->getComponent<CInput>().left = true; 
            if (m_camera->getComponent<CState>().state == "entity")
            {
                m_menuSelection--;
                if (m_menuSelection < 0) m_menuSelection = (m_entityTypes.size() - 1);
            }
            else if (m_camera->getComponent<CState>().state == "sl")
            {
                m_menuSelection--;
                if (m_menuSelection < 0) m_menuSelection = (m_saveLimit - 1);
            }
        }
        else if (action.name() == "RIGHT") 
        { 
            m_camera->getComponent<CInput>().right = true;
            if (m_camera->getComponent<CState>().state == "entity")
            {
                m_menuSelection = (m_menuSelection + 1) % m_entityTypes.size();
            }
            else if (m_camera->getComponent<CState>().state == "sl")
            {
                m_menuSelection = (m_menuSelection + 1) % (m_saveLimit);
            }
        }
        else if (action.name() == "FAST")
        {
            m_fast = true;
        }

        else if (action.name() == "LEFT_CLICK") { m_camera->getComponent<CInput>().click1 = true; }
        else if (action.name() == "RIGHT_CLICK") 
        { 
            m_camera->getComponent<CInput>().click2 = true; 
            if (m_camera->getComponent<CState>().state == "drag" && m_selected->tag() == "npc")
            {
                m_selected->getComponent<CTransform>().scale.x = -m_selected->getComponent<CTransform>().scale.x;
            }
        }
        else if (action.name() == "MIDDLE_CLICK") { m_camera->getComponent<CInput>().click3 = true; }
        else if (action.name() == "MOUSE_MOVE") { m_mPos = action.pos(); }

        else if (action.name() == "DELETE_MODE")
        {
            if (m_camera->getComponent<CState>().state == "move")
            {
                m_game->playSound("death");
                m_camera->getComponent<CState>().state = "delete";
            }
            else if (m_camera->getComponent<CState>().state == "delete")
            {
                m_camera->getComponent<CState>().state = "move";
            }
            else if (m_camera->getComponent<CState>().state == "drag")
            {
                m_game->playSound("death");
                m_camera->getComponent<CState>().state = "delete";
                m_selected->destroy();
                m_selected = NULL;
                m_drop = true;
                m_place = false;
            }
        }

        else if (action.name() == "MUSIC")
        {
            m_game->assets().getMusic(m_levelConfig.MUSIC).stop();
            for (int i = 0; i < m_levelAssetList["Music"].size(); i++)
            {
                if (m_levelAssetList["Music"][i] == m_levelConfig.MUSIC)
                {
                    m_levelConfig.MUSIC = m_levelAssetList["Music"][(i + 1) % m_levelAssetList["Music"].size()];
                    break;
                }
            }

            if (!m_mute)
            {
                m_game->playMusic(m_levelConfig.MUSIC);
            }
        }
        else if (action.name() == "MUTE")
        {
            m_mute = !m_mute;

            if (!m_mute) { m_game->playMusic(m_levelConfig.MUSIC); }
            else m_game->assets().getMusic(m_levelConfig.MUSIC).stop();
        }
        else if (action.name() == "BACKGROUND")
        {
            for (int i = 0; i < m_levelAssetList["Background"].size(); i++)
            {
                if (m_levelAssetList["Background"][i] == m_levelConfig.BACKGROUND)
                {
                    m_levelConfig.BACKGROUND = m_levelAssetList["Background"][(i + 1) % m_levelAssetList["Background"].size()];
                    break;
                }
            }
        }
        else if (action.name() == "DARK")
        {
            m_levelConfig.DARK = !m_levelConfig.DARK;
        }

        else if (action.name() == "ENTITY_MENU")
        {
            if (m_camera->getComponent<CState>().state == "entity" || m_camera->getComponent<CState>().state == "mod")
            {
                m_menuSelection = 0;
                m_camera->getComponent<CState>().state = "move";
                clearMenu();
            }
            else if (m_camera->getComponent<CState>().state == "move")
            {
                m_pageSelection = 0;
                m_camera->getComponent<CState>().state = "entity";
                showEntityPage(0);
            }
        }
        else if (action.name() == "SAVE/LOAD_MENU")
        {
            if (m_camera->getComponent<CState>().state == "sl")
            {
                m_menuSelection = 0;
                m_camera->getComponent<CState>().state = "move";
                clearMenu();
            }
            else if (m_camera->getComponent<CState>().state == "move")
            {
                m_menuSelection = 0;
                m_camera->getComponent<CState>().state = "sl";
                showSLMenu();
            }
        }
        else if (action.name() == "PLACE_POINT")
        {
            if (m_camera->getComponent<CState>().state == "move" && m_selected != NULL && m_selected->hasComponent<CPatrol>() &&
                m_selected->getComponent<CPatrol>().positions.size() < 20 && m_patrol)
            {
                m_patrol = false;
                Vec2 point = {windowToWorld(m_mPos)};
                if (snapToGrid(m_selected, point))
                {
                    m_selected->getComponent<CPatrol>().positions.push_back(point);
                }
            }
        }
        else if (action.name() == "CLEAR_POINTS")
        {
            if (m_selected != NULL && m_selected->hasComponent<CPatrol>() && m_selected->getComponent<CPatrol>().positions.size() > 0)
            {
                m_selected->getComponent<CPatrol>().positions.clear();
            }
        }

    }
    else if (action.type() == "END")
    {
        if (action.name() == "UP") { m_camera->getComponent<CInput>().up = false; }
        else if (action.name() == "DOWN") { m_camera->getComponent<CInput>().down = false; }
        else if (action.name() == "LEFT") 
        { 
            m_camera->getComponent<CInput>().left = false; 
            m_modDecrease = true;
        }
        else if (action.name() == "RIGHT") 
        { 
            m_camera->getComponent<CInput>().right = false; 
            m_modIncrease = true;
        }
        else if (action.name() == "FAST")
        {
            m_fast = false;
        }

        else if (action.name() == "LEFT_CLICK")
        {
            m_camera->getComponent<CInput>().click1 = false;
            m_drop = true;
            m_place = false;
        }
        else if (action.name() == "RIGHT_CLICK")
        {
            m_camera->getComponent<CInput>().click2 = false;
        }
        else if (action.name() == "MIDDLE_CLICK") { m_camera->getComponent<CInput>().click3 = false; }

        else if (action.name() == "PLACE_POINT") m_patrol = true;

    }
}

void Scene_Editor::sAnimation()
{

    // update animation frame for all entities
    for (auto& e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CAnimation>())
        {
            CAnimation& eAni = e->getComponent<CAnimation>();
            eAni.animation.update();
            if (!eAni.repeat && eAni.animation.hasEnded()) e->destroy();
        }
    }
}

void Scene_Editor::onEnd()
{
    m_hasEnded = true;
    m_game->assets().getMusic(m_levelConfig.MUSIC).stop();
    m_game->playMusic("MusicTitle");
    m_game->changeScene("MENU", std::shared_ptr<Scene_Menu>(), true);
}

void Scene_Editor::renderEntity(std::shared_ptr<Entity> e)
{
    auto& transform = e->getComponent<CTransform>();
    auto& ct = m_camera->getComponent<CTransform>();

    if (abs(transform.pos.x - ct.pos.x) > (width() / 2) + m_gridSize.x * 2 ||
        abs(transform.pos.y - ct.pos.y) > (height() / 2) + m_gridSize.y * 2) return;

    if (e->hasComponent<CAnimation>())
    {
        auto& animation = e->getComponent<CAnimation>().animation;
        animation.getSprite().setRotation(transform.angle);
        animation.getSprite().setPosition(transform.pos.x, transform.pos.y);

        if (e->hasComponent<CGravity>() && e->getComponent<CGravity>().gravity < 0) 
             animation.getSprite().setScale(transform.scale.x, -transform.scale.y);
        else animation.getSprite().setScale(transform.scale.x, transform.scale.y);

        m_game->window().draw(animation.getSprite());
    }
}

void Scene_Editor::renderTile(std::shared_ptr<Entity> e)
{
    if (e->tag() != "tile") return;
    auto& transform = e->getComponent<CTransform>();
    auto& ct = m_camera->getComponent<CTransform>();

    if (abs(transform.pos.x - ct.pos.x) > (width() / 2) + m_gridSize.x * 2 ||
        abs(transform.pos.y - ct.pos.y) > (height() / 2) + m_gridSize.y * 2) return;

    if (e->hasComponent<CAnimation>())
    {
        auto& animation = e->getComponent<CAnimation>().animation;
        animation.getSprite().setRotation(transform.angle);
        animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
        animation.getSprite().setScale(transform.scale.x, transform.scale.y);

        if (!e->getComponent<CBoundingBox>().blockVision) animation.getSprite().setColor(sf::Color(255, 255, 255, 150));
        else animation.getSprite().setColor(sf::Color(255, 255, 255, 255));

        m_game->window().draw(animation.getSprite());

        // render indicaters for special tiles
        Vec2& cen = e->getComponent<CTransform>().pos;
        CBoundingBox& b = e->getComponent<CBoundingBox>();
        for (int i = 0; i < 3; i++)
        {
            if (i == 0 && b.blockMove) continue;
            else if (i == 1 && e->getComponent<CDamage>().damage == 0) continue;
            else if (i == 2 && !e->hasComponent<CPatrol>()) continue;

            sf::Color col;
            sf::CircleShape cir;
            cir.setRadius(4);
            cir.setOrigin(4, 4);
            cir.setPosition(cen.x - b.halfSize.x + 4 + i * 10, cen.y + b.halfSize.y - 4);

            if (i == 0) col = sf::Color(0, 255, 0);
            else if (i == 1) col = sf::Color(255, 0, 0);
            else col = sf::Color(0, 0, 255);

            cir.setFillColor(col);
            m_game->window().draw(cir);
        }
    }
}

void Scene_Editor::sRender()
{
    std::string s = m_camera->getComponent<CState>().state;

    // determines colour of background
    std::tuple<int,int,int> rgb;
    if (m_levelConfig.BACKGROUND == "Background1") rgb = { 62, 77, 179 };
    else if (m_levelConfig.BACKGROUND == "Background2") rgb = { 225, 102, 50 };
    else if (m_levelConfig.BACKGROUND == "Background3") rgb = { 230, 226, 87 };
    else rgb = { 148, 148, 148 };
    if (m_levelConfig.DARK)
    {
        std::get<0>(rgb) -= 50;
        std::get<1>(rgb) -= 50;
        std::get<2>(rgb) -= 50;
    }

    m_game->window().clear(sf::Color(std::get<0>(rgb), std::get<1>(rgb), std::get<2>(rgb)));

    // set the viewport of the window to be centered on the player if it's far enough right
    auto& pPos = m_camera->getComponent<CTransform>().pos;

    sf::View view = m_game->window().getView();
    view.setCenter(pPos.x, pPos.y);
    m_game->window().setView(view);

    Vec2 upperLeftCorner = Vec2((m_game->window().getView().getCenter().x - width() / 2),
        (m_game->window().getView().getCenter().y - height() / 2));
    // normal editor drawing
    if (s != "entity" && s != "sl" && s != "mod")
    {
        // draw all Entity textures / animations
        if (m_drawTextures)
        {
            for (auto e : m_entityManager.getEntities("tile"))
            {
                renderTile(e);
            }
            // dec render
            for (auto e : m_entityManager.getEntities("dec"))
            {
                renderEntity(e);
            }
            // item render
            for (auto e : m_entityManager.getEntities("item"))
            {
                renderEntity(e);
            }
            // npc render
            for (auto e : m_entityManager.getEntities("npc"))
            {
                renderEntity(e);
            }
            // player render
            renderEntity(m_player);
            if (m_drawCamera) renderEntity(m_camera);
        }

        // guide text
        std::string control = "";
        if (m_help)
        {
            control = "Toggle: Texture = T | Collision Boxes = C | Camera = F | Grid = G (" + std::to_string(m_drawGrid) + ") | " +
                "Help: H | Quit: Esc\n" +
                // delete mode
                (s == "delete" ? "DELETE MODE ON (Right click to delete | Backspace to toggle off)" :
                    // move mode
                    (s == "move" ? "Menus: Entity = Q | Save/Load = Tab\nDelete Mode (Backspace) | Mod Mode (Mouse3) | Paste Entity (Mouse2) " +
                        // if m_selected has entity
                        (m_selected != NULL ? "Entity: " + m_selected->tag() + " " +
                            m_selected->getComponent<CAnimation>().animation.getName() : "") :
                        "Place Entity (Mouse1) | Delete Mode (Backspace)"));

            if (s == "move" && m_selected != NULL && m_selected->hasComponent<CPatrol>())
            {
                control = control + "\nPlace Point: P | Clear Points: O";
            }
        }
        else
        {
            // Minimal help
            control = "D(" + std::to_string(s == "delete") + ") | E("
                + (m_selected != NULL ? m_selected->tag() + " " + m_selected->getComponent<CAnimation>().animation.getName() : "") + 
                ") | Help: H";
        }

        m_controlText.setString(control);

        m_controlText.setPosition(upperLeftCorner.x, upperLeftCorner.y);
        m_game->window().draw(m_controlText);

        // draw all Entity collision bounding boxes with a rectangleshape
        if (m_drawCollision)
        {
            for (auto e : m_entityManager.getEntities())
            {
                if (e->hasComponent<CBoundingBox>())
                {
                    auto& box = e->getComponent<CBoundingBox>();
                    auto& transform = e->getComponent<CTransform>();
                    auto& ct = m_camera->getComponent<CTransform>();

                    if (abs(transform.pos.x - ct.pos.x) > (width() / 2) + m_gridSize.x * 2 ||
                        abs(transform.pos.y - ct.pos.y) > (height() / 2) + m_gridSize.y * 2) continue;

                    sf::RectangleShape rect;
                    rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
                    rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
                    rect.setPosition(transform.pos.x, transform.pos.y);
                    rect.setFillColor(sf::Color(0, 0, 0, 0));
                    rect.setOutlineColor(sf::Color(255, 255, 255, 255));
                    rect.setOutlineThickness(1);
                    m_game->window().draw(rect);
                }
            }
        }

        // draw the grid so that students can easily debug
        if (m_drawGrid > 0)
        {
            Vec2 absolutePos = { m_BOUNDARYPOS.x * m_gridSize.x , m_BOUNDARYPOS.y * m_gridSize.y };
            Vec2 absoluteNeg = { m_BOUNDARYNEG.x * m_gridSize.x , m_BOUNDARYNEG.y * m_gridSize.y };

            // define boundaries for lines
            float leftX = absoluteNeg.x;
            float rightX = absolutePos.x;
            float upY = absoluteNeg.y;
            float downY = absolutePos.y;

            float halfWidth = width() / 2.0f;
            float halfHeight = height() / 2.0f;

            float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

            for (float x = nextGridX; x < rightX + 1; x += m_gridSize.x)
            {
                drawLine(Vec2(x, upY), Vec2(x, downY));
            }

            for (float y = upY; y < downY + 1; y += m_gridSize.y)
            {
                drawLine(Vec2(leftX, y), Vec2(rightX, y));

                if (y == downY) continue;
                for (float x = nextGridX; x < rightX; x += m_gridSize.x)
                {
                    if (x > pPos.x - halfWidth && x < pPos.x + halfWidth && y > pPos.y - halfHeight && y < pPos.y + halfHeight &&
                        m_drawGrid == 2)
                    {
                        std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
                        std::string yCell = std::to_string(((int)m_BOUNDARYPOS.y - 1) - ((int)y / (int)m_gridSize.y));
                        m_gridText.setString("(" + xCell + "," + yCell + ")");
                        m_gridText.setPosition(x + 3, y + 2);
                        m_game->window().draw(m_gridText);
                    }
                }
            }
        }
        if (m_selected != NULL && m_selected->hasComponent<CPatrol>())
        {
            std::vector<Vec2>& points = m_selected->getComponent<CPatrol>().positions;
            if (!points.empty())
            {
                Vec2 old;
                sf::Color black(0, 0, 0);
                for (int i = 0; i < points.size(); i++)
                {
                    // draw point
                    sf::CircleShape c;
                    c.setRadius(5);
                    c.setFillColor(black);
                    c.setOrigin(5, 5);
                    c.setPosition(points[i].x, points[i].y);
                    m_game->window().draw(c);

                    // draw line
                    if (i != 0)
                    {
                        sf::Vertex line[2] =
                        {
                            sf::Vertex(sf::Vector2f(points[i].x, points[i].y), black),
                            sf::Vertex(sf::Vector2f(old.x, old.y), black)
                        };
                        m_game->window().draw(line, 2, sf::Lines);
                    }

                    // draw extra line at end
                    if (i + 1 == points.size() && i > 1)
                    {
                        sf::Vertex line[2] = 
                        { 
                            sf::Vertex(sf::Vector2f(points[i].x, points[i].y), black), 
                            sf::Vertex(sf::Vector2f(points[0].x, points[0].y), black)
                        };
                        m_game->window().draw(line, 2, sf::Lines);
                    }
                    else old = points[i];
                }
            }
        }
    }
    // menu drawing
    else
    {
        float textWidth;
        if (s == "entity")
        {
            // menu drawing
            m_controlText.setString("A/D to cycle type | W/S to cycle page | Q to return\nPage: " + std::to_string(m_pageSelection) + "/"
                + std::to_string((int)floor((m_aniAssets.size() - 1) / 112)));

            m_controlText.setPosition(upperLeftCorner.x, upperLeftCorner.y);
            m_game->window().draw(m_controlText);

            m_selectionText.setString("< " + m_entityTypes[m_menuSelection] + " >");

            textWidth = m_selectionText.getString().getSize() * m_selectionText.getCharacterSize();
            m_selectionText.setPosition(upperLeftCorner.x + (width() / 2) - (textWidth / 2), upperLeftCorner.y + m_gridSize.y);
            m_game->window().draw(m_selectionText);
        }
        else if (s == "sl")
        {
            m_controlText.setString("A/D to cycle saves | Tab to return");
            m_controlText.setPosition(upperLeftCorner.x, upperLeftCorner.y);
            m_game->window().draw(m_controlText);

            m_selectionText.setString("< level" + std::to_string(m_menuSelection + 1) + " >");

            textWidth = m_selectionText.getString().getSize() * m_selectionText.getCharacterSize();
            m_selectionText.setPosition(upperLeftCorner.x + (width() / 2) - (textWidth / 2),upperLeftCorner.y + m_gridSize.y);
            m_game->window().draw(m_selectionText);
        }
        else if (s == "mod")
        {
            m_controlText.setString("W/S to select values | A/D to change values | Q to return");
            m_controlText.setPosition(upperLeftCorner.x, upperLeftCorner.y);
            m_game->window().draw(m_controlText);

            // image of entity
            auto& transform = m_camera->getComponent<CTransform>();
            auto& animation = m_selected->getComponent<CAnimation>();

            Vec2 aniSize = animation.animation.getSize();

            animation.animation.getSprite().setPosition(transform.pos.x - (m_gridSize.x * 5), transform.pos.y);
            animation.animation.getSprite().setScale((m_gridSize.x * 4) / aniSize.x ,  (m_gridSize.y * 4) / aniSize.y );
            m_game->window().draw(animation.animation.getSprite());

            m_selectionText.setString("\n>");
            m_selectionText.setPosition(upperLeftCorner.x + (width() / 2) - (m_modText.getCharacterSize() + 3), 
                                        upperLeftCorner.y + (height() / 4) + (m_pageSelection * (m_modText.getCharacterSize() + 6)));
            m_game->window().draw(m_selectionText);

            std::vector<std::string>& values = m_modTypes[m_selected->tag()];
            std::string modValues = "";
            for (std::string v : values)
            {
                if (v == "AI") continue;
                modValues = modValues + "\n" + v;
                if (v == "Block vision") modValues = modValues + ": " + std::to_string(m_selected->getComponent<CBoundingBox>().blockVision);
                else if (v == "Block move") modValues = modValues + ": " + std::to_string(m_selected->getComponent<CBoundingBox>().blockMove);
                else if (v == "Health") modValues = modValues + ": " + std::to_string(m_selected->getComponent<CHealth>().max);
                else if (v == "Jump") modValues = modValues + ": " + 
                    (m_selected->getComponent<CJump>().jump != 0 ? formatFloat(-m_selected->getComponent<CJump>().jump) : "0");
                else if (v == "Gravity") modValues = modValues + ": " + formatFloat(m_selected->getComponent<CGravity>().gravity);
                else if (v == "Damage")
                {
                    if (m_selected->hasComponent<CDamage>()) modValues = modValues + ": " + 
                                 std::to_string(m_selected->getComponent<CDamage>().damage);
                    else modValues = modValues + ": 0";
                }
                else if (v == "Speed")
                {
                    if (m_selected->tag() == "player")
                    {
                        modValues = modValues + ": " + formatFloat(m_playerConfig.SPEED);
                    }
                    else if (m_selected->tag() == "tile")
                    {
                        modValues = modValues + ": " + formatFloat(m_selected->getComponent<CPatrol>().speed);
                    }
                    else if (m_selected->tag() == "npc")
                    {
                        if (m_selected->hasComponent<CPatrol>())
                        {
                            modValues = modValues + ": " + formatFloat(m_selected->getComponent<CPatrol>().speed);
                            modValues = modValues + "\n" + values[values.size() - 1] + ": Patrol";
                        }
                        else if (m_selected->hasComponent<CFollowPlayer>())
                        {
                            modValues = modValues + ": " + formatFloat(m_selected->getComponent<CFollowPlayer>().speed);
                            modValues = modValues + "\n" + values[values.size() - 1] + ": Follow";
                        }
                    }
                }
            }
            m_modText.setString(modValues);
            m_modText.setPosition(upperLeftCorner.x + (width() / 2), upperLeftCorner.y + (height() / 4));
            m_game->window().draw(m_modText);
        }

        for (auto e : m_entityManager.getEntities("button"))
        {
            auto& transform = e->getComponent<CTransform>();

            if (e->hasComponent<CAnimation>())
            {
                auto& animation = e->getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(transform.angle);
                animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
                animation.getSprite().setScale(transform.scale.x, transform.scale.y);
                m_game->window().draw(animation.getSprite());
            }
            if (e->hasComponent<CButton>())
            {
                m_buttonText.setString(e->getComponent<CButton>().value);

                textWidth = m_buttonText.getString().getSize() * m_buttonText.getCharacterSize();
                m_buttonText.setPosition(transform.pos.x - (textWidth / 2), transform.pos.y);
                m_game->window().draw(m_buttonText);
            }
        }
    }
    // other controls
    if (m_help)
    {
        m_controlText.setString("Music (8): " + m_levelConfig.MUSIC +
            " | Mute(M) " + std::to_string(m_mute) + " | Background(9) : " + m_levelConfig.BACKGROUND + " | Dark(0) : " + 
            std::to_string(m_levelConfig.DARK) +
            " | Name: " + m_levelConfig.NAME);

        m_controlText.setPosition(upperLeftCorner.x, upperLeftCorner.y + height() - m_controlText.getCharacterSize() - 5);
        m_game->window().draw(m_controlText);
    }
}
