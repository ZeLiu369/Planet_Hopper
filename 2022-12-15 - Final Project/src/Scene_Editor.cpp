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
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");      // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");    // Toggle drawing (C)ollision Boxes
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");         // Toggle drawing (G)rid

    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Space, "SHOOT");
    registerAction(sf::Keyboard::M, "MONEY");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Tech"));

    m_exampleText.setCharacterSize(24);
    m_exampleText.setFont(m_game->assets().getFont("Tech"));

    loadLevel(levelPath);
}

Vec2 Scene_Editor::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{

    Vec2 size = entity->getComponent<CAnimation>().animation.getSize();

    return Vec2(
        (gridX * m_gridSize.x) + size.x / 2,
        height() - ((gridY * m_gridSize.y) - size.y / 2)
    );
}

void Scene_Editor::loadLevel(const std::string& filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    std::ifstream fin(filename);
    std::string temp;

    while (fin >> temp)
    {
        if (temp != "Player")
        {

            std::string type = temp;
            std::string texture;
            float x, y;
            fin >> texture >> x >> y;

            Vec2 tSize = m_game->assets().getAnimation(texture).getSize();
            x = (x * m_gridSize.x) + (tSize.x / 2);
            y = (height()) - ((y * m_gridSize.y) + (tSize.y / 2));

            auto& tile = m_entityManager.addEntity(type == "Dec" ? "dec" : "tile");
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
            tile->addComponent<CTransform>(Vec2(x, y));

            if (type == "Tile") tile->addComponent<CBoundingBox>(tSize);
        }
        else
        {
            PlayerConfig& pc = m_playerConfig;
            fin >> pc.X >> pc.Y >> pc.CX >> pc.CY >> pc.SPEED
                >> pc.JUMP >> pc.MAXSPEED >> pc.GRAVITY >> pc.WEAPON;
        }

    }

    spawnCamera();
}

void Scene_Editor::spawnCamera()
{
    std::string CAMERA_AVATAR = "Stand";
    Vec2 BOUND_BOX = Vec2(5, 5);
    PlayerConfig& pc = m_playerConfig;

    m_camera = m_entityManager.addEntity("camera");
    m_camera->addComponent<CAnimation>(m_game->assets().getAnimation(CAMERA_AVATAR), true);

    Vec2 spawnPos = gridToMidPixel(0, 0, m_camera);
    m_camera->addComponent<CTransform>(spawnPos);
    m_camera->addComponent<CBoundingBox>(BOUND_BOX);

    m_camera->addComponent<CInput>();
    m_camera->addComponent<CState>("move");
}

void Scene_Editor::update()
{
    m_entityManager.update();

    if (!m_paused)
    {
        sMovement();
        sLifespan();
        sCollision();
        sAnimation();
    }
    sRender();

    m_currentFrame++;
}

void Scene_Editor::sMovement()
{
    PlayerConfig& pc = m_playerConfig;

    CTransform& transform = m_camera->getComponent<CTransform>();
    CInput& input = m_camera->getComponent<CInput>();
    CState& state = m_camera->getComponent<CState>();

    // player movement

    // horizontal movement
    Vec2 dir = Vec2((input.right - input.left), (input.down - input.up));
    transform.velocity = dir * pc.SPEED;

    // player speed limits
    if (abs(transform.velocity.x) > pc.MAXSPEED)
    {
        transform.velocity.x = transform.velocity.x > 0 ? pc.MAXSPEED : -pc.MAXSPEED;
    }
    if (abs(transform.velocity.y) > pc.MAXSPEED)
    {
        transform.velocity.y = transform.velocity.y > 0 ? pc.MAXSPEED : -pc.MAXSPEED;
    }

    // updates prevPos and current pos
    transform.prevPos = transform.pos;
    transform.pos.x += transform.velocity.x;
    transform.pos.y += transform.velocity.y;
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
    // REMEMBER: SFML's (0,0) position is on the TOP-LEFT corner
    //           This means jumping will have a negative y-component
    //           and gravity will have a positive y-component
    //           Also, something BELOW something else will have a y value GREATER than it
    //           Also, something ABOVE something else will have a y value LESS than it

    // boundary collison
    CTransform& transform = m_camera->getComponent<CTransform>();
    CBoundingBox& box = m_camera->getComponent<CBoundingBox>();

    Vec2 absolutePos = { m_BOUNDARYPOS.x * m_gridSize.x , m_BOUNDARYPOS.y * m_gridSize.y };
    Vec2 absoluteNeg = { m_BOUNDARYNEG.x * m_gridSize.x , m_BOUNDARYNEG.y * m_gridSize.y };

    // when the player goes to far left
    if (transform.pos.x < absoluteNeg.x) transform.pos.x = absolutePos.x - box.halfSize.x;
    if (transform.pos.x > absolutePos.x) transform.pos.x = absoluteNeg.x + box.halfSize.x;

    // when the player falls into a pit
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
        else if (action.name() == "PAUSE") { setPaused(!m_paused); }
        else if (action.name() == "QUIT") { onEnd(); }

        else if (action.name() == "UP") { m_camera->getComponent<CInput>().up = true; }
        else if (action.name() == "DOWN") { m_camera->getComponent<CInput>().down = true; }
        else if (action.name() == "LEFT") { m_camera->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT") { m_camera->getComponent<CInput>().right = true; }

    }
    else if (action.type() == "END")
    {
        if (action.name() == "UP") { m_camera->getComponent<CInput>().up = false; }
        if (action.name() == "DOWN") { m_camera->getComponent<CInput>().down = false; }
        if (action.name() == "LEFT") { m_camera->getComponent<CInput>().left = false; }
        if (action.name() == "RIGHT") { m_camera->getComponent<CInput>().right = false; }

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
    m_game->changeScene("MENU", std::shared_ptr<Scene_Menu>(), true);
}

void Scene_Editor::sRender()
{
    // color the background darker so you know that the game is paused
    if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
    else { m_game->window().clear(sf::Color(50, 50, 150)); }

    // set the viewport of the window to be centered on the player if it's far enough right
    auto& pPos = m_camera->getComponent<CTransform>().pos;

    // Don't need this anymore (maybe)
    //float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
    //float windowCenterY = std::max(m_game->window().getSize().y / 2.0f, pPos.y);

    sf::View view = m_game->window().getView();
    view.setCenter(pPos.x, pPos.y);
    m_game->window().setView(view);

    // draw all Entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities())
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
        }
    }

    // Example ui display
    Vec2 upperLeftCorner = Vec2((m_game->window().getView().getCenter().x - width() / 2),
                                (m_game->window().getView().getCenter().y - height() / 2));

    m_exampleText.setString("EXAMPLE HELP INTERFACE");
    m_exampleText.setPosition(upperLeftCorner.x,upperLeftCorner.y);
    m_game->window().draw(m_exampleText);

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
        float flippedY = downY - pPos.y;

        float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

        for (float x = nextGridX; x < rightX; x += m_gridSize.x)
        {
            drawLine(Vec2(x, 0), Vec2(x, downY));
        }

        for (float y = upY; y < downY; y += m_gridSize.y)
        {
            drawLine(Vec2(leftX, downY - y), Vec2(rightX, downY - y));

            for (float x = nextGridX; x < rightX; x += m_gridSize.x)
            {
                if (x > pPos.x - width() && x < pPos.x + width() && y > flippedY - height() && y < flippedY + height() &&
                    m_drawGrid == 2)
                {
                    std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
                    std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
                    m_gridText.setString("(" + xCell + "," + yCell + ")");
                    m_gridText.setPosition(x + 3, downY - y - m_gridSize.y + 2);
                    m_game->window().draw(m_gridText);
                }
            }
        }
    }
}
