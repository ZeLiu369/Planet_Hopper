

#include "Scene_Overworld.h"
#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Scene_Editor.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include "Physics.h"

#include <iostream>
#include <fstream>

Scene_Overworld::Scene_Overworld(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
}

void Scene_Overworld::init()
{
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");      // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");    // Toggle drawing (C)ollision Boxes

    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Space, "SELECT");

    m_levelPaths.push_back("level1.txt");
    m_levelPaths.push_back("level2.txt");
    m_levelPaths.push_back("level3.txt");

    loadMap();
}

void Scene_Overworld::loadMap()
{
    m_entityManager = EntityManager();

    std::shared_ptr<Entity> planet1 = m_entityManager.addEntity("planet");
    planet1->addComponent<CAnimation>(m_game->assets().getAnimation("Plan1"), true);
    planet1->addComponent<CTransform>(Vec2(400, 360));

    std::shared_ptr<Entity> planet2 = m_entityManager.addEntity("planet");
    planet2->addComponent<CAnimation>(m_game->assets().getAnimation("Plan2"), true);
    planet2->addComponent<CTransform>(Vec2(500, 360));

    std::shared_ptr<Entity> planet3 = m_entityManager.addEntity("planet");
    planet3->addComponent<CAnimation>(m_game->assets().getAnimation("Plan3"), true);
    planet3->addComponent<CTransform>(Vec2(600, 360));

    spawnPlayer();
}

void Scene_Overworld::spawnPlayer()
{
    m_player = m_entityManager.addEntity("player");

    m_player->addComponent<CTransform>(Vec2(640, 360));
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
    m_player->addComponent<CBoundingBox>(Vec2(48, 48));
    m_player->addComponent<CInput>();
}

void Scene_Overworld::sMovement()
{

    CTransform& transform = m_player->getComponent<CTransform>();
    CInput& input = m_player->getComponent<CInput>();

    // horizontal movement
    int xdir = (input.right - input.left);
    transform.velocity.x = xdir * 5;
    if (xdir != 0) transform.scale.x = xdir;

    // vertical movement
    int ydir = (input.down - input.up);
    transform.velocity.y = ydir * 5;

    // player speed limits
    if (abs(transform.velocity.x) > 20)
    {
        transform.velocity.x = transform.velocity.x > 0 ? 20 : -20;
    }
    if (abs(transform.velocity.y) > 20)
    {
        transform.velocity.y = transform.velocity.y > 0 ? 20 : -20;
    }

    // updates prevPos and current pos
    transform.prevPos = transform.pos;
    transform.pos += transform.velocity;
}

void Scene_Overworld::sAnimation()
{

}

void Scene_Overworld::sCollision()
{
    for (auto p : m_entityManager.getEntities("planet"))
    {
        if      (p->getComponent<CAnimation>().animation.getName() == "Plan1") { m_selectedMenuIndex = 0; }
        else if (p->getComponent<CAnimation>().animation.getName() == "Plan2") { m_selectedMenuIndex = 1; }
        else if (p->getComponent<CAnimation>().animation.getName() == "Plan3") { m_selectedMenuIndex = 2; }

        if (m_changeScene && Physics::IsInside(m_player->getComponent<CTransform>().pos, p))
        {
            m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_levelPaths[m_selectedMenuIndex]));
        }
    }
}

void Scene_Overworld::sCamera()
{
    //sf::View view = m_game->window().getView();

    //int x = 0;
    //int y = 0;
    //int w_posX = m_game->window().getSize().x;
    //int w_posY = m_game->window().getSize().y;
    //Vec2 m_pos = m_player->getComponent<CTransform>().pos;

    //if (m_pos.x < 0) 
    //{
    //    int x = w_posX * (m_pos.x / w_posX);
    //    int y = w_posY * (m_pos.y / w_posY);
    //}
    //else
    //{
    //    int x = w_posX * ((m_pos.x / w_posX) - 1);
    //    int y = w_posY * ((m_pos.y / w_posY) - 1);
    //}
    //
    //view.reset(sf::FloatRect(x, y, w_posX, w_posY));
    //m_game->window().setView(view);
}

void Scene_Overworld::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
        else if (action.name() == "QUIT") { onEnd(); }
        else if (action.name() == "SELECT") { m_changeScene = true; }

        else if (action.name() == "UP") { m_player->getComponent<CInput>().up = true; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = true; }
        else if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = true; }
    }

    else if (action.type() == "END")
    {
        if      (action.name() == "UP")     { m_player->getComponent<CInput>().up = false; }
        else if (action.name() == "DOWN")   { m_player->getComponent<CInput>().down = false; }
        else if (action.name() == "LEFT")   { m_player->getComponent<CInput>().left = false; }
        else if (action.name() == "RIGHT")  { m_player->getComponent<CInput>().right = false; }
    }
}

void Scene_Overworld::update()
{
    m_entityManager.update();

    sMovement();
    sCollision();
    sAnimation();
    sCamera();

    m_currentFrame++;
}

void Scene_Overworld::onEnd()
{
    m_hasEnded = true;
    m_game->changeScene("MENU", nullptr, true);
}

void Scene_Overworld::sRender()
{
    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("images/new/blue.png");
    //background.setTexture(backgroundTexture);

    Vec2 TextureSize(backgroundTexture.getSize().x, backgroundTexture.getSize().y);  //Added to store texture size.
    Vec2 WindowSize(m_game->window().getSize().x, m_game->window().getSize().y);   //Added to store window size.

    float ScaleX = (float)WindowSize.x / TextureSize.x;
    float ScaleY = (float)WindowSize.y / TextureSize.y;     //Calculate scale.

    background.setTexture(backgroundTexture);
    background.setScale(ScaleX, ScaleY);      //Set scale.  
    m_game->window().draw(background);
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
}