///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_Play.cpp
// 
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): Nathan
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/

#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "Scene_Overworld.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

#include <iostream>
#include <fstream>
#include <sstream>

Scene_Play::Scene_Play(GameEngine * gameEngine, const std::string & levelPath)
    : Scene(gameEngine)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void Scene_Play::init(const std::string & levelPath)
{
    registerAction(sf::Keyboard::P,     "PAUSE");
    registerAction(sf::Keyboard::Escape,"QUIT");
    registerAction(sf::Keyboard::T,     "TOGGLE_TEXTURE");      // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C,     "TOGGLE_COLLISION");    // Toggle drawing (C)ollision Boxes
    registerAction(sf::Keyboard::G,     "TOGGLE_GRID");         // Toggle drawing (G)rid

    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Space, "SHOOT");
    registerAction(sf::Keyboard::M, "MONEY");
                                                     
    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Tech"));

    m_coinText.setCharacterSize(24);
    m_coinText.setFont(m_game->assets().getFont("Tech"));

    loadLevel(levelPath);
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    Vec2 size = entity->getComponent<CAnimation>().animation.getSize();
    auto& eScale = entity->getComponent<CTransform>().scale;

    return Vec2(
        (gridX * m_gridSize.x) + size.x * eScale.x / 2,
        height() - ((gridY * m_gridSize.y) - size.y * eScale.y / 2)
    );
}

void Scene_Play::loadLevel(const std::string & filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    m_game->assets().getSound("OverWorld").stop();
    m_game->playSound("Play");

    std::ifstream fin(filename);
    std::string temp;

    while (fin >> temp)
    {
        if (temp != "Player")
        {
            std::string type = temp;
            std::string texture;
            if (type == "Background")
            {
                int scroll;
                float scaleX, scaleY, x, y, scrollFactor;
                fin >> texture >> scroll >> scaleX >> scaleY >> x >> y;
                if (scroll == 1)
                {
                    fin >> scrollFactor;
                    auto backgroundScroll = m_entityManager.addEntity("scrollbackground");
                    backgroundScroll->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
                    backgroundScroll->addComponent<CTransform>(Vec2(x, y), Vec2(scaleX, scaleY), Vec2(scrollFactor, scrollFactor));

                    auto backgroundScroll2 = m_entityManager.addEntity("scrollbackground");
                    backgroundScroll2->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
                    backgroundScroll2->addComponent<CTransform>(Vec2(x + m_game->window().getSize().x, y), Vec2(scaleX, scaleY), Vec2(scrollFactor, scrollFactor));

                    m_backgroundsMap[texture].push_back(backgroundScroll);
                    m_backgroundsMap[texture].push_back(backgroundScroll2);
                }
                else
                {
                    auto backgroundNoScroll = m_entityManager.addEntity("noscrollbackground");
                    backgroundNoScroll->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
                    backgroundNoScroll->addComponent<CTransform>(Vec2(x, y));
                    backgroundNoScroll->getComponent<CTransform>().scale = { scaleX, scaleY };
                }
                continue;
            }
            float x, y;
            fin >> texture >> x >> y;

            Vec2 tSize = m_game->assets().getAnimation(texture).getSize();
            x = (x * m_gridSize.x) + (tSize.x / 2);
            y = (height()) - ((y * m_gridSize.y) + (tSize.y / 2));

            auto tile = m_entityManager.addEntity(type == "Dec" ? "dec" : "tile");
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

    spawnPlayer();
}

void Scene_Play::spawnPlayer()
{
    PlayerConfig& pc = m_playerConfig;

    // here is a sample player entity which you can use to construct other entities
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);

    Vec2 spawnPos = gridToMidPixel(pc.X, pc.Y, m_player);
    m_player->addComponent<CTransform>(spawnPos);
    m_player->addComponent<CBoundingBox>(Vec2(pc.CX, pc.CY));

    m_player->addComponent<CInput>();
    m_player->addComponent<CGravity>(pc.GRAVITY);
    m_player->addComponent<CState>("air");

    m_player->addComponent<CCoinCounter>();

    // reset background positions
    for (auto bg : m_entityManager.getEntities("scrollbackground"))
    {
        bg->getComponent<CTransform>().pos = bg->getComponent<CTransform>().originalPos;
    }
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{

    Vec2 BULLET_SIZE = Vec2(20, 20);
    int BULLET_LIFETIME = 60;

    PlayerConfig& pc = m_playerConfig;

    CTransform& entityT = entity->getComponent<CTransform>();
    auto bullet = m_entityManager.addEntity("bullet");

    bullet->addComponent<CAnimation>(m_game->assets().getAnimation(pc.WEAPON), true);

    bullet->addComponent<CTransform>
        (
            entityT.pos,
            Vec2(pc.SPEED * entityT.scale.x * 2.5f, 0.0f),
            entityT.scale,
            0.0f
        );

    bullet->addComponent<CBoundingBox>(BULLET_SIZE);

    bullet->addComponent<CLifeSpan>(BULLET_LIFETIME, m_currentFrame);
}

// fires a coin bullet
void Scene_Play::spawnMoney(std::shared_ptr<Entity> entity)
{

    Vec2 BULLET_SIZE = Vec2(45, 45);
    int BULLET_LIFETIME = 120;

    PlayerConfig& pc = m_playerConfig;

    CTransform& entityT = entity->getComponent<CTransform>();
    auto bullet = m_entityManager.addEntity("coinbullet");

    bullet->addComponent<CAnimation>(m_game->assets().getAnimation("Coin"), true);

    bullet->addComponent<CTransform>
        (
            entityT.pos,
            Vec2(pc.SPEED * entityT.scale.x * 3.0f, 0.0f),
            entityT.scale,
            0.0f
            );

    bullet->addComponent<CBoundingBox>(BULLET_SIZE);

    bullet->addComponent<CLifeSpan>(BULLET_LIFETIME, m_currentFrame);
}

void Scene_Play::update()
{
    m_entityManager.update();

    if (!m_paused)
    {
        sMovement();
        sLifespan();
        sCollision();
        sAnimation();
    }
    // sRender() doesn't need to be called here
    //sRender();
    sCamera();
    sScroll();

    m_currentFrame++;
}

void Scene_Play::sMovement()
{
    PlayerConfig& pc = m_playerConfig;

    CTransform& transform = m_player->getComponent<CTransform>();
    CInput& input = m_player->getComponent<CInput>();
    CState& state = m_player->getComponent<CState>();
    CGravity& gravity = m_player->getComponent<CGravity>();
    CCoinCounter& counter = m_player->getComponent<CCoinCounter>();

    // player movement

    // horizontal movement
    int dir = (input.right - input.left);
    transform.velocity.x = dir * pc.SPEED;
    if (dir != 0) transform.scale.x = dir;

    // vertical movement
    if (state.state == "air")
    {
        transform.velocity.y += gravity.gravity;

        // stops jump if player releases UP input
        if (gravity.gravity >= 0)
        {
            if (transform.velocity.y < 0 && !input.up) transform.velocity.y = 0;
        }
        else
        {
            if (transform.velocity.y > 0 && !input.up) transform.velocity.y = 0;
        }
    }
    else
    {
        // player should be on ground
        transform.velocity.y = 0;
        
        // enables player to jump
        if (input.up && input.canJump)
        {
            input.canJump = false;
            transform.velocity.y = gravity.gravity >= 0 ? pc.JUMP : -pc.JUMP;
        }

        if (input.down && input.canJump)
        {
            input.canJump = false;
            gravity.gravity = -gravity.gravity;
        }

    }
    transform.scale.y = gravity.gravity >= 0 ? 1 : -1;

    // player shooting
    if (input.shoot && input.canShoot)
    {
        input.canShoot = false;
        spawnBullet(m_player);
    }

    // if a player has a coin, fire the coin
    if (input.money && input.canShoot && counter.coins > 0)
    {
        counter.coins -= 1;
        input.canShoot = false;
        spawnMoney(m_player);
    }

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

    // bullet movement
    for (auto& b : m_entityManager.getEntities("bullet"))
    {
        CTransform& bt = b->getComponent<CTransform>();
        bt.pos.x += bt.velocity.x;
    }
    for (auto& c : m_entityManager.getEntities("coinbullet"))
    {
        if (c->hasComponent<CBoundingBox>())
        {
            CTransform& ct = c->getComponent<CTransform>();
            ct.pos.x += ct.velocity.x;
        }
    }
    // update background movement
    for (auto bg : m_entityManager.getEntities("scrollbackground"))
    {
        if (m_player->getComponent<CTransform>().pos.x > m_game->window().getSize().x / 2.0f)
        {
            bg->getComponent<CTransform>().velocity.x = transform.velocity.x * bg->getComponent<CTransform>().scrollFactor.x;
        }
        else
        {
            bg->getComponent<CTransform>().velocity.x = 0;
        }
        bg->getComponent<CTransform>().pos.x += bg->getComponent<CTransform>().velocity.x;
    }
}

void Scene_Play::sScroll()
{
    auto& playerTransform = m_player->getComponent<CTransform>();
    for (auto e : m_entityManager.getEntities("scrollbackground"))
    {
        auto& eVelocity = e->getComponent<CTransform>().velocity;
        auto& ePos = e->getComponent<CTransform>().pos;
        auto& eOriginalPos = e->getComponent<CTransform>().originalPos;
        auto& eAnimation = e->getComponent<CAnimation>().animation;
        auto& eScrollFactor = e->getComponent<CTransform>().scrollFactor;
        if (playerTransform.pos.x > m_game->window().getSize().x)
        {
            auto& backgroundOnePos = m_backgroundsMap[eAnimation.getName()][0]->getComponent<CTransform>().pos;
            auto& backgroundTwoPos = m_backgroundsMap[eAnimation.getName()][1]->getComponent<CTransform>().pos;
            if (abs(playerTransform.pos.x - backgroundOnePos.x) <= abs(playerTransform.velocity.x - playerTransform.velocity.x * eScrollFactor.x))
            {
                if (playerTransform.velocity.x > 0)
                {
                    backgroundTwoPos.x = backgroundOnePos.x + m_game->window().getSize().x - eVelocity.x;
                }
                else
                {
                    backgroundTwoPos.x = backgroundOnePos.x - m_game->window().getSize().x - eVelocity.x;
                }
            }
            else if (abs(playerTransform.pos.x - backgroundTwoPos.x) <= abs(playerTransform.velocity.x - playerTransform.velocity.x * eScrollFactor.x))
            {
                if (playerTransform.velocity.x > 0)
                {
                    backgroundOnePos.x = backgroundTwoPos.x + m_game->window().getSize().x - eVelocity.x;
                }
                else
                {
                    backgroundOnePos.x = backgroundTwoPos.x - m_game->window().getSize().x - eVelocity.x;
                }
            }
        }
    }
}
                                                     
void Scene_Play::sLifespan()
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

void Scene_Play::sCollision()
{
    // REMEMBER: SFML's (0,0) position is on the TOP-LEFT corner
    //           This means jumping will have a negative y-component
    //           and gravity will have a positive y-component
    //           Also, something BELOW something else will have a y value GREATER than it
    //           Also, something ABOVE something else will have a y value LESS than it

    m_player->getComponent<CState>().state = "air";
    bool goal = false;
    for (auto& tile : m_entityManager.getEntities("tile"))
    {
        for (auto& e : m_entityManager.getEntities())
        {
            if (e->tag() == "player" || e->tag() == "bullet" || e->tag() == "coinbullet")
            {
                bool destroyBrick = false;
                bool robbery = false;

                Vec2 overlap = Physics::GetOverlap(e, tile);
                if (overlap.x >= 0 && overlap.y >= 0)
                {
                    if (e->tag() == "player")
                    {
                        CTransform& et = e->getComponent<CTransform>();
                        CTransform& tileT = tile->getComponent<CTransform>();
                        Vec2 delta = et.pos - tileT.pos;
                        Vec2 prev = Physics::GetPreviousOverlap(e, tile);

                        if (tile->getComponent<CAnimation>().animation.getName() == "Pole" ||
                            tile->getComponent<CAnimation>().animation.getName() == "PoleTop") goal = true;

                        // collison correction for player
                        if (prev.y > 0)
                        {
                            // prevent background from moving
                            for (auto bg : m_entityManager.getEntities("scrollbackground"))
                            {
                                bg->getComponent<CTransform>().pos.x -= bg->getComponent<CTransform>().velocity.x;
                            }
                            et.pos.x += delta.x > 0 ? overlap.x : -overlap.x;
                        }

                        else if (prev.x > 0)
                        {
                            if (delta.y > 0)
                            {

                                if (m_player->getComponent<CGravity>().gravity >= 0)
                                {
                                    destroyBrick = (tile->getComponent<CAnimation>().animation.getName() == "Brick");
                                    robbery = (tile->getComponent<CAnimation>().animation.getName() == "Question");
                                    et.velocity.y = 0;
                                }
                                else
                                {
                                    m_player->getComponent<CState>().state = "ground";
                                }
                                et.pos.y += overlap.y;

                            }
                            else
                            {
                                if (m_player->getComponent<CGravity>().gravity >= 0)
                                {
                                    m_player->getComponent<CState>().state = "ground";
                                }
                                else
                                {
                                    destroyBrick = (tile->getComponent<CAnimation>().animation.getName() == "Brick");
                                    robbery = (tile->getComponent<CAnimation>().animation.getName() == "Question");
                                    et.velocity.y = 0;
                                }
                                et.pos.y -= overlap.y;
                            }
                        }

                    }
                    else if (overlap.x != 0 && overlap.y != 0)
                    {
                        // bullet collsion resolution
                        if (tile->getComponent<CAnimation>().animation.getName() == "Brick")
                        {
                            destroyBrick = true;
                        }
                        if (e->tag() == "coinbullet")
                        {
                            e->removeComponent<CBoundingBox>();
                            e->removeComponent<CLifeSpan>();
                            e->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
                        }
                        else
                        {
                            e->destroy();
                        }
                    }

                }

                // for when tile brick is marked for explosion
                if (destroyBrick)
                {
                    tile->removeComponent<CBoundingBox>();
                    tile->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
                    continue;
                }
                else if (robbery)
                {
                    // creates coin from question block
                    tile->addComponent<CAnimation>(
                        m_game->assets().getAnimation("Question2"), true);

                    CTransform& qt = tile->getComponent<CTransform>();
                    int sizeY = m_game->assets().getAnimation("Question2").getSize().y;

                    auto coin = m_entityManager.addEntity("coin");
                    coin->addComponent<CTransform>(Vec2(qt.pos.x, qt.pos.y - sizeY));
                    coin->addComponent<CAnimation>(
                        m_game->assets().getAnimation("Coin"), false);

                    // ? block adds one coin
                    m_player->getComponent<CCoinCounter>().coins++;
                }
            }
        }
    }

    // boundary collison
    CTransform& transform = m_player->getComponent<CTransform>();
    CBoundingBox& box = m_player->getComponent<CBoundingBox>();

    // when the player goes to far left
    if (transform.pos.x < box.halfSize.x) transform.pos.x = box.halfSize.x;

    // when the player falls into a pit
    if (transform.pos.y > height() || goal)
    {
        PlayerConfig& pc = m_playerConfig;

        transform.velocity = Vec2(0, 0);

        Vec2 spawnPos = gridToMidPixel(pc.X, pc.Y, m_player);
        m_player->addComponent<CTransform>(spawnPos);

        // reset backgrounds
        for (auto e : m_entityManager.getEntities("scrollbackground"))
        {
            e->getComponent<CTransform>().pos = e->getComponent<CTransform>().originalPos;
        }
    }
}

void Scene_Play::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
             if (action.name() == "TOGGLE_TEXTURE")     { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION")   { m_drawCollision = !m_drawCollision; }
        else if (action.name() == "TOGGLE_GRID")        { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "PAUSE")              { setPaused(!m_paused); }
        else if (action.name() == "QUIT")               { onEnd(); }

        else if (action.name() == "UP")                 { m_player->getComponent<CInput>().up = true; }
        else if (action.name() == "DOWN")               { m_player->getComponent<CInput>().down = true; }
        else if (action.name() == "LEFT")               { m_player->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT")              { m_player->getComponent<CInput>().right = true; }

        else if (action.name() == "SHOOT")              { m_player->getComponent<CInput>().shoot = true; }
        else if (action.name() == "MONEY")              { m_player->getComponent<CInput>().money = true; }
        
    }
    else if (action.type() == "END")
    {
        if (action.name() == "UP")   
        { 
            m_player->getComponent<CInput>().up = false; 
            m_player->getComponent<CInput>().canJump = true;
        }
        if (action.name() == "DOWN") 
        { 
            m_player->getComponent<CInput>().down = false;
            m_player->getComponent<CInput>().canJump = true;
        }
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = false; }
        if (action.name() == "RIGHT"){ m_player->getComponent<CInput>().right = false; }
        if (action.name() == "SHOOT")
        { 
            m_player->getComponent<CInput>().shoot = false;
            m_player->getComponent<CInput>().canShoot = true; 
        }
        if (action.name() == "MONEY")
        {
            m_player->getComponent<CInput>().money = false;
            m_player->getComponent<CInput>().canShoot = true;
        }
        
    }
}
                                                     
void Scene_Play::sAnimation()
{

    // player animations
    CAnimation& pAni = m_player->getComponent<CAnimation>();

    // animations for player on the ground
    if (m_player->getComponent<CState>().state == "ground")
    {
        if (m_player->getComponent<CTransform>().velocity.x != 0)
        {
            // when they are moving
            if (pAni.animation.getName() != "Run")
            {
                m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Run"), true);
            }
        }
        else
        {
            // when they are standing
            if (pAni.animation.getName() != "Stand")
            {
                m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
            }
        }
    }
    // animation when player is in the air
    else
    {
        if (pAni.animation.getName() != "Air")
        {
            m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Air"), true);
        }
    }
    
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

void Scene_Play::onEnd()
{
    m_hasEnded = true;
    m_game->assets().getSound("Play").stop();
    m_game->playSound("OverWorld");
    //m_game->changeScene("OVERWORLD",std::shared_ptr<Scene_Overworld>(), true);
    m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game));
}

void Scene_Play::sCamera()
{
    // set the viewport of the window to be centered on the player if it's far enough right
    auto& pPos = m_player->getComponent<CTransform>().pos;
    float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
    sf::View view = m_game->window().getView();
    view.setCenter(windowCenterX, m_game->window().getSize().y - view.getCenter().y);
    m_game->window().setView(view);

    // always keep no scroll background on screen
    for (auto e : m_entityManager.getEntities("noscrollbackground"))
    {
        auto& ePos = e->getComponent<CTransform>().pos;
        ePos.x = m_game->window().getView().getCenter().x;
    }
}

void Scene_Play::sRender()
{
    // color the background darker so you know that the game is paused
    //if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
    //else { m_game->window().clear(sf::Color(50, 50, 150)); }
    m_game->window().clear();
   
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

    // Coin Counter display
    if (m_player->hasComponent<CCoinCounter>())
    {
        float leftX = m_game->window().getView().getCenter().x - width() / 2;
        std::string coinAmount = std::to_string(m_player->getComponent<CCoinCounter>().coins);

        m_coinText.setString("Coins: " + coinAmount + "\nPress 'M' to shoot money");
        m_coinText.setPosition(leftX, 0);
        m_game->window().draw(m_coinText);
    }
                                                     
    // draw all Entity collision bounding boxes with a rectangleshape
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CBoundingBox>())
            {
                auto & box = e->getComponent<CBoundingBox>();
                auto & transform = e->getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.size.x-1, box.size.y-1));
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
    if (m_drawGrid)
    {
        float leftX = m_game->window().getView().getCenter().x - width() / 2;
        float rightX = leftX + width() + m_gridSize.x;
        float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

        for (float x = nextGridX; x < rightX; x += m_gridSize.x)
        {
            drawLine(Vec2(x, 0), Vec2(x, height()));
        }

        for (float y = 0; y < height(); y += m_gridSize.y)
        {
            drawLine(Vec2(leftX, height() - y), Vec2(rightX, height() - y));

            for (float x = nextGridX; x < rightX; x += m_gridSize.x)
            {
                std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
                std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
                m_gridText.setString("(" + xCell + "," + yCell + ")");
                m_gridText.setPosition(x + 3, height() - y - m_gridSize.y + 2);
                m_game->window().draw(m_gridText);
            }
        }
    }
}
                                                     
                                                     
// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
