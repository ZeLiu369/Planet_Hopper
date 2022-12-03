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

Scene_Editor::Scene_Editor(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void Scene_Editor::init(const std::string& levelPath)
{
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");      // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");    // Toggle drawing (C)ollision Boxes
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");         // Toggle drawing (G)rid
    registerAction(sf::Keyboard::F, "TOGGLE_CAMERA");         // Toggle drawing (F)amera
    registerAction(sf::Keyboard::Delete, "DELETE_MODE");         // Toggle (Delete) mode

    registerAction(sf::Keyboard::Num8, "MUSIC");
    registerAction(sf::Keyboard::Num9, "BACKGROUND");
    registerAction(sf::Keyboard::Num0, "DARK");
    registerAction(sf::Keyboard::Num1, "ENTITY_MENU");

    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Tech"));

    m_controlText.setCharacterSize(24);
    m_controlText.setFont(m_game->assets().getFont("Tech"));

    fillAssetList();
    loadLevel(levelPath);
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

    Vec2 size = entity->getComponent<CAnimation>().animation.getSize();

    return Vec2(
        (gridX * m_gridSize.x) + size.x / 2,
        (m_BOUNDARYPOS.y - gridY) * m_gridSize.y - (size.y / 2)
    );
}

Vec2 Scene_Editor::midPixelToGrid(std::shared_ptr<Entity> entity)
{

    Vec2 size = entity->getComponent<CAnimation>().animation.getSize();
    Vec2 pos = entity->getComponent<CTransform>().pos;

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

    for (auto& e : m_entityManager.getEntities())
    {
        if (e->getComponent<CTransform>().pos == snap)
        {
            return false;
        }
    }
    

    Vec2 absolutePos = { m_BOUNDARYPOS.x * m_gridSize.x , m_BOUNDARYPOS.y * m_gridSize.y };
    Vec2 absoluteNeg = { m_BOUNDARYNEG.x * m_gridSize.x , m_BOUNDARYNEG.y * m_gridSize.y };

    // out of bounds
    if (snap.x < absoluteNeg.x || snap.x > absolutePos.x ||
        snap.y < absoluteNeg.y || snap.y > absolutePos.y)
    {
        return false;
    }

    ePos = snap;
    return true;
}
/*

// dev plan for level editor here

place stuff with menu

save and load menu, using virtual keyboard

modify mode / modifiy entity by middle click, camera locks onto entity, middle click entity to disengage,
list if values to change appears next to entity: 
health, speed, ai (cycle),gravity,jump,maxspeed?,damage, etc all will be increment cycle or increment
extra mode if ai is patrol, when right clicking on enemy, enter patrol mode, 
click on empty spaces to create points, then finish by clicking on enemy
list of entities that can be changed: player, enemy, hazard, moving platform

*/

void Scene_Editor::fillAssetList()
{
    std::vector<std::string> BLACK_LIST =
    {
        "Inventory", "Sky", "Stars", "Hill", "Land", "Craters", "SkyObj", "Rock", "Space"
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

void Scene_Editor::loadLevel(const std::string& filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    m_game->assets().getSound("MusicTitle").stop();

    m_levelConfig.MUSIC = "Play";
    m_levelConfig.DARK = false;
    m_levelConfig.BACKGROUND = "None";

    std::ifstream fin(filename);
    std::string temp;

    while (fin >> temp)
    {
        if (temp != "Item" && temp != "Player")
        {
            std::string type = temp;
            std::string texture;
            if (type == "Lighting")
            {
                std::string time;
                fin >> time;
                if (time == "Night")
                {
                    m_levelConfig.DARK = true;
                }
                continue;
            }
            if (type == "Background")
            {
                fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            float x, y;
            fin >> texture >> x >> y;

            auto tile = m_entityManager.addEntity(type == "Dec" ? "dec" : "tile");
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
            tile->addComponent<CTransform>(gridToMidPixel(x, y, tile));

            if (type == "Tile") tile->addComponent<CBoundingBox>(tile->getComponent<CAnimation>().animation.getSize());
            tile->addComponent<CDraggable>();
        }
        else if (temp == "Item")
        {
            std::string tileName;
            float x, y;
            fin >> tileName >> x >> y;
            Vec2 tSize = m_game->assets().getAnimation(tileName).getSize();

            auto item = m_entityManager.addEntity("item");

            item->addComponent<CAnimation>(m_game->assets().getAnimation(tileName), true);
            item->addComponent<CTransform>(gridToMidPixel(x, y, item));
            item->addComponent<CBoundingBox>(m_game->assets().getAnimation(tileName).getSize());
            item->addComponent<CDraggable>();
        }
        else
        {
            PlayerConfig& pc = m_playerConfig;
            fin >> pc.X >> pc.Y >> pc.CX >> pc.CY >> pc.SPEED
                >> pc.JUMP >> pc.MAXSPEED >> pc.GRAVITY >> pc.WEAPON;
        }
    }
    spawnPlayer();
    spawnCamera();

    m_game->playSound(m_levelConfig.MUSIC);
}

void Scene_Editor::spawnPlayer()
{
    PlayerConfig& pc = m_playerConfig;

    // here is a sample player entity which you can use to construct other entities
    auto& player = m_entityManager.addEntity("player");
    player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);

    Vec2 spawnPos = gridToMidPixel(pc.X, pc.Y, player);
    player->addComponent<CTransform>(spawnPos);
    player->addComponent<CBoundingBox>(Vec2(pc.CX, pc.CY));
    player->addComponent<CDraggable>();
}

void Scene_Editor::spawnCamera()
{
    PlayerConfig& pc = m_playerConfig;

    m_camera = m_entityManager.addEntity("camera");
    m_camera->addComponent<CAnimation>(m_game->assets().getAnimation(m_CAMERA_AVATAR), true);

    Vec2 spawnPos = gridToMidPixel(0, 0, m_camera);
    m_camera->addComponent<CTransform>(spawnPos);
    m_camera->addComponent<CBoundingBox>(m_BOUND_BOX);

    m_camera->addComponent<CInput>();
    m_camera->addComponent<CState>("move");
}

bool Scene_Editor::pasteEntity(std::shared_ptr<Entity> e)
{
    auto& entity = m_entityManager.addEntity(e->tag());
    entity->addComponent<CTransform>(windowToWorld(m_mPos));
    entity->addComponent<CDraggable>();

    if (e->hasComponent<CBoundingBox>()) entity->addComponent<CBoundingBox>() = e->getComponent<CBoundingBox>();
    if (e->hasComponent<CAnimation>()) entity->addComponent<CAnimation>() = e->getComponent<CAnimation>();
    if (e->hasComponent<CGravity>()) entity->addComponent<CGravity>() = e->getComponent<CGravity>();

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

void Scene_Editor::update()
{
    m_entityManager.update();

    sState();
    sMovement();
    sLifespan();
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
            for (auto e : m_entityManager.getEntities())
            {
                if (e->hasComponent<CDraggable>() && Physics::IsInside(windowToWorld(m_mPos), e))
                {
                    if (!e->getComponent<CDraggable>().dragging)
                    {
                        e->getComponent<CDraggable>().dragging = true;
                        m_selected = e;
                        state.state = "drag";
                        m_drop = false;
                        break;
                    }
                }
            }
        }
        else if (input.click2 && m_selected != NULL && m_selected->tag() != "player")
        {
            pasteEntity(m_selected);
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

        // change animation of selected entity based on tag
        else if (input.click2 && m_texture && m_selected->tag() != "player")
        {
            m_texture = false;
            int nextAni = 0;

            for (nextAni = 0; nextAni < m_aniAssets.size(); nextAni++)
            {
                if (m_aniAssets[nextAni] == m_selected->getComponent<CAnimation>().animation.getName())
                {
                    nextAni = (nextAni + 1) % m_aniAssets.size();
                    break;
                }
            }

            m_selected->addComponent<CAnimation>(m_game->assets().getAnimation(m_aniAssets[nextAni] ), true);

            // make sure to change bounding box of some tiles
            if (m_selected->tag() == "tile")
            {
                Vec2 tSize = m_game->assets().getAnimation(m_aniAssets[nextAni]).getSize();
                m_selected->addComponent<CBoundingBox>(tSize);
            }
        }
    }
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
}

void Scene_Editor::sMovement()
{
    CTransform& transform = m_camera->getComponent<CTransform>();
    CInput& input = m_camera->getComponent<CInput>();

    // camera movement \\

    // horizontal movement
    Vec2 dir = Vec2((input.right - input.left), (input.down - input.up));
    transform.velocity = dir * m_CAMERA_SPEED;

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

void Scene_Editor::sLifespan()
{
    for (auto& e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CLifeSpan>())
        {
            CLifeSpan& ls = e->getComponent<CLifeSpan>();
            if (m_currentFrame - ls.frameCreated > ls.lifespan)
            {
                e->destroy();
            }
        }
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

        else if (action.name() == "UP") { m_camera->getComponent<CInput>().up = true; }
        else if (action.name() == "DOWN") { m_camera->getComponent<CInput>().down = true; }
        else if (action.name() == "LEFT") { m_camera->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT") { m_camera->getComponent<CInput>().right = true; }

        else if (action.name() == "LEFT_CLICK") { m_camera->getComponent<CInput>().click1 = true; }
        else if (action.name() == "RIGHT_CLICK") { m_camera->getComponent<CInput>().click2 = true; }
        else if (action.name() == "MIDDLE_CLICK") { m_camera->getComponent<CInput>().click3 = true; }
        else if (action.name() == "MOUSE_MOVE") { m_mPos = action.pos(); }

        else if (action.name() == "DELETE_MODE")
        {
            if (m_camera->getComponent<CState>().state == "move")
            {
                m_camera->getComponent<CState>().state = "delete";
            }
            else if (m_camera->getComponent<CState>().state == "delete")
            {
                m_camera->getComponent<CState>().state = "move";
            }
        }

        else if (action.name() == "MUSIC")
        {
            m_game->assets().getSound(m_levelConfig.MUSIC).stop();
            for (int i = 0; i < m_levelAssetList["Music"].size(); i++)
            {
                if (m_levelAssetList["Music"][i] == m_levelConfig.MUSIC)
                {
                    m_levelConfig.MUSIC = m_levelAssetList["Music"][(i + 1) % m_levelAssetList["Music"].size()];
                    break;
                }
            }
            m_game->playSound(m_levelConfig.MUSIC);
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
            if (m_camera->getComponent<CState>().state == "entity")
            {
                m_camera->getComponent<CState>().state = "move";
            }
            else
            {
                m_camera->getComponent<CState>().state = "entity";
            }
        }

    }
    else if (action.type() == "END")
    {
        if (action.name() == "UP") { m_camera->getComponent<CInput>().up = false; }
        if (action.name() == "DOWN") { m_camera->getComponent<CInput>().down = false; }
        if (action.name() == "LEFT") { m_camera->getComponent<CInput>().left = false; }
        if (action.name() == "RIGHT") { m_camera->getComponent<CInput>().right = false; }

        else if (action.name() == "LEFT_CLICK") 
        {
            m_camera->getComponent<CInput>().click1 = false; 
            m_drop = true;
            m_place = false;
        }
        else if (action.name() == "RIGHT_CLICK") 
        { 
            m_camera->getComponent<CInput>().click2 = false;
            m_texture = true;
        }
        else if (action.name() == "MIDDLE_CLICK") { m_camera->getComponent<CInput>().click3 = false; }

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
    m_game->assets().getSound(m_levelConfig.MUSIC).stop();
    m_game->playSound("MusicTitle");
    m_game->changeScene("MENU", std::shared_ptr<Scene_Menu>(), true);
}

void Scene_Editor::sRender()
{
    // color the background darker so you know that the game is paused

    std::tuple<int,int,int> rgb;
    if (m_levelConfig.BACKGROUND == "Background1") rgb = { 60, 123, 232 };
    else if (m_levelConfig.BACKGROUND == "Background2") rgb = { 158, 132, 179 };
    else if (m_levelConfig.BACKGROUND == "Background3") rgb = { 150, 51, 64 };
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

    // draw all Entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->tag() == "camera" && !m_drawCamera) continue;

            auto& transform = e->getComponent<CTransform>();

            if (e->hasComponent<CAnimation>())
            {
                auto& animation = e->getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(transform.angle);
                animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
                animation.getSprite().setScale(transform.scale.x, transform.scale.y);
                m_game->window().draw(animation.getSprite());
            }
        }
    }

    // Example ui display
    Vec2 upperLeftCorner = Vec2((m_game->window().getView().getCenter().x - width() / 2),
                                (m_game->window().getView().getCenter().y - height() / 2));

    std::string s = m_camera->getComponent<CState>().state;

    m_controlText.setString("Toggle: Texture = T | Collision Boxes = C | Camera = F | Grid = G (" + std::to_string(m_drawGrid) + ")"
    + "\nMenus: Entity = 1"
    + "\n" + (s == "delete" ? "DELETE MODE ON (Right click to delete | DEL to toggle off)" : (s == "move" ? "Delete Mode (DEL)" : "")));
    m_controlText.setPosition(upperLeftCorner.x,upperLeftCorner.y);
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
}
