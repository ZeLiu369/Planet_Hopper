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
#include "Scene_OptionMenu.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");      // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");    // Toggle drawing (C)ollision Boxes
    registerAction(sf::Keyboard::I, "INVENTORY");               // Toggle drawing (T)extures
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");         // Toggle drawing (G)rid
    registerAction(sf::Keyboard::O, "TOGGLE_OPTION_MENU");        // Toggle option menu
    registerAction(sf::Keyboard::B, "BOSS");        

    registerAction(sf::Keyboard::Num1, "RAYGUN");
    registerAction(sf::Keyboard::Num2, "BOMB");
    registerAction(sf::Keyboard::Num3, "LAUNCHER");

    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Space, "SHOOT");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("ChunkFive"));

    m_weaponUIText.setCharacterSize(14);
    m_weaponUIText.setFont(m_game->assets().getFont("Roboto"));

    loadLevel(levelPath);

    std::string diff = m_game->getDiff();

    if (diff == "EASY")
    {
        bulletScaler = 2.0;
    }

    if (diff == "NORMAL")
    {
        bulletScaler = 1.0;
    }

    if (diff == "HARD")
    {
        bulletScaler = 0.5;
    }
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    Vec2 size = entity->getComponent<CAnimation>().animation.getSize();
    auto& eScale = entity->getComponent<CTransform>().scale;

    float x = (gridX * m_gridSize.x) + (size.x * eScale.x / 2);
    float y = (height()) - ((gridY * m_gridSize.y) + (size.y * eScale.y / 2));

    return Vec2(x, y);
}

void Scene_Play::loadLevel(const std::string& filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    m_game->assets().getMusic("OverWorld").stop();

    std::ifstream fin(filename);
    std::string temp;
    int i = 0;
    std::vector<Vec2> pos;
    int x1, y1;

    m_inventoryEntity = m_entityManager.addEntity("inventory");
    m_inventoryEntity->addComponent<CAnimation>(m_game->assets().getAnimation("Inventory"), true);
    m_inventoryEntity->addComponent<CTransform>(Vec2(m_game->window().getView().getCenter().x - width() / 2, m_game->window().getView().getCenter().y - height() / 2 + 70));
    m_inventoryEntity->addComponent<CInventory>();

    while (fin >> temp)
    {
        std::string texture;
        if (temp == "BackgroundType") { fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
        else if (temp == "Music") { fin >> m_levelMusic; }
        else if (temp == "Dec" || temp == "Tile")
        {
            std::string type = temp;
            float x, y;
            fin >> texture >> x >> y;
            auto tile = m_entityManager.addEntity(type == "Dec" ? "dec" : "tile");
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
            tile->addComponent<CTransform>(gridToMidPixel(x, y, tile));

            if (texture == "Spaceship")
            {
                m_levelEnd = tile->getComponent<CTransform>().pos.x;
            }

            if (type == "Tile") 
            {
                float speed;
                int tileBM, tileBV, dmg;
                bool bm, bv;

                fin >> tileBM >> tileBV >> dmg >> speed;

                switch (tileBM)
                {
                case 0: {bm = false;   break;}
                case 1: {bm = true;    break;}
                }

                switch (tileBV)
                {
                case 0: {bv = false;   break;}
                case 1: {bv = true;    break;}
                }

                tile->addComponent<CBoundingBox>(tile->getComponent<CAnimation>().animation.getSize(), bm, bv);
                

                if (dmg > 0) { tile->addComponent<CDamage>(dmg); }
                if (speed > 0)
                {
                    std::vector<Vec2> positions;
                    float coordX, coordY;
                    int numPoints;
                    fin >> numPoints;
                    for (int i = 0; i < numPoints; i++)
                    {
                        fin >> coordX >> coordY;
                        positions.push_back(gridToMidPixel(coordX, coordY, tile));
                    }
                    tile->addComponent<CPatrol>(positions, speed);
                }
            }
        }
        else if (temp == "Background")
        {
            int scroll;
            float scaleX, scaleY, x, y, scrollFactor;
            std::string texture;
            fin >> texture >> scroll >> scaleX >> scaleY >> x >> y;
            if (scroll == 1)
            {
                fin >> scrollFactor;
                auto backgroundScroll = m_entityManager.addEntity("scrollbackground");
                backgroundScroll->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
                backgroundScroll->addComponent<CTransform>(Vec2(x, y), Vec2(scaleX, scaleY), scrollFactor);

                auto backgroundScroll2 = m_entityManager.addEntity("scrollbackground");
                backgroundScroll2->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
                scaleX = -scaleX;
                backgroundScroll2->addComponent<CTransform>(Vec2(x + m_game->window().getSize().x, y), Vec2(scaleX, scaleY), scrollFactor);

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
        }
        else if (temp == "Lighting")
        {
            std::string time;
            fin >> time;
            if (time == "Night")
            {
                m_renderTexture.create(m_game->window().getSize().x, m_game->window().getSize().y);
                m_lightTexture = m_game->assets().getTexture("TexLight");
                m_night = true;
            }
            else
            {
                m_night = false;
            }
        }
        else if (temp == "Npc")
        {
            int scaleX, health, dmg;
            float x, y, jump, gravity, speed;
            std::string aiType;
            fin >> texture >> x >> y >> scaleX >> health >> dmg >> jump >> gravity >> speed >> aiType;
            auto npc = m_entityManager.addEntity("npc");
            npc->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
            npc->addComponent<CTransform>(gridToMidPixel(x, y, npc));
            npc->addComponent<CDamage>(dmg);
            npc->addComponent<CGravity>(gravity);
            npc->addComponent<CHealth>(health, health);
            npc->addComponent<CBoundingBox>(Vec2(64, 64));
            npc->addComponent<CState>(texture);
            if (aiType == "Patrol")
            {
                std::vector<Vec2> positions;
                float coordX, coordY;
                int numPoints;
                fin >> numPoints;
                for (int i = 0; i < numPoints; i++)
                {
                    fin >> coordX >> coordY;
                    positions.push_back(gridToMidPixel(coordX, coordY, npc));
                }
                npc->addComponent<CPatrol>(positions, speed);
                positions.clear();
            }
            else { npc->addComponent<CFollowPlayer>(gridToMidPixel(x, y, npc), speed); }
        }
        else if (temp == "Item")
        {
            std::string tileName;
            float x, y;
            fin >> tileName >> x >> y;

            auto item = m_entityManager.addEntity("item");

            item->addComponent<CAnimation>(m_game->assets().getAnimation(tileName), true);
            item->addComponent<CTransform>(gridToMidPixel(x, y, item));
            item->addComponent<CBoundingBox>(m_game->assets().getAnimation(tileName).getSize());
            item->addComponent<CInventory>(i++);
            item->addComponent<CClickable>();
            std::cout << item->getComponent<CInventory>().index << "\n";
            m_inventoryEntity->getComponent<CInventory>().inventoryItems.push_back(tileName);
        }
        else
        {
            PlayerConfig& pc = m_playerConfig;
            fin >> pc.X >> pc.Y >> pc.CX >> pc.CY >> pc.SPEED
                >> pc.JUMP >> pc.MAXSPEED >> pc.GRAVITY;
        }
    }


    // Load in the weapons
    // Hardcoded because weapons will be the same for each level
    // Third weapon will be the launcher on the player sprite's arm that will fire a missile
    auto raygun = m_entityManager.addEntity("weapon");
    raygun->addComponent<CAnimation>(m_game->assets().getAnimation("Raygun"), true);

    auto bomb = m_entityManager.addEntity("weapon");
    bomb->addComponent<CAnimation>(m_game->assets().getAnimation("Bomb"), true);

    spawnPlayer();

    m_game->assets().getMusic(m_levelMusic).setLoop(true);
    m_game->playMusic(m_levelMusic);

    // Load shaders
    if (!electric_shader.loadFromFile("images/new/electric_shader.frag", sf::Shader::Fragment))
    {
        std::cerr << "Error while loading electric shader" << std::endl;
        return;
    }
    if (!bright_shader.loadFromFile("images/new/bright_shader.frag", sf::Shader::Fragment))
    {
        std::cerr << "Error while loading bright shader" << std::endl;
        return;
    }
    if (!speed_shader.loadFromFile("images/new/speed_shader.frag", sf::Shader::Fragment))
    {
        std::cerr << "Error while loading speed shader" << std::endl;
        return;
    }
    if (!rainbow_shader.loadFromFile("images/new/rainbow_shader.frag", sf::Shader::Fragment))
    {
        std::cerr << "Error while loading rainbow shader" << std::endl;
        return;
    }
    if (!red_shader.loadFromFile("images/new/shader_red.frag", sf::Shader::Fragment))
    {
        std::cerr << "Error while loading red shader" << std::endl;
        return;
    }
}

void Scene_Play::loadBoss()
{
    m_game->assets().getMusic("Play").stop();
    loadLevel("BossFight.txt");

    auto boss = m_entityManager.addEntity("boss");
    boss->addComponent<CAnimation>(m_game->assets().getAnimation("BossIdle"), true);

    boss->addComponent<CTransform>(gridToMidPixel(13, 5, boss));
    boss->addComponent<CBoundingBox>(Vec2(75, 150));
    boss->addComponent<CHealth>(25, 25);
    boss->addComponent<CDamage>(2);
    boss->addComponent<CState>("idle");
    boss->addComponent<CBulletTimer>(300, m_currentFrame);
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
    m_player->addComponent<CHealth>(10, 10);
    // m_player->addComponent<CDamage>(2);
    m_player->addComponent<CState>("air");
    m_player->addComponent<CJump>();

    // raygun is default weapon
    m_player->addComponent<CWeapon>("Raygun");
}

/*
* Setup the bullet with the appropriate components
*/
std::shared_ptr<Entity> Scene_Play::setupBullet(Vec2 size, Vec2 pos, int lifetime, int dmg, Vec2 vel, std::string name)
{
    auto bullet = m_entityManager.addEntity("bullet");
    bullet->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
    bullet->addComponent<CTransform>(pos);
    bullet->getComponent<CTransform>().velocity = vel;
    if (name == "Laser") { bullet->getComponent<CTransform>().scale.x = m_player->getComponent<CTransform>().scale.x; }
    if (!(name == "Missile")) { bullet->addComponent<CBoundingBox>(size); };
    bullet->addComponent<CLifeSpan>(lifetime, m_currentFrame);
    bullet->addComponent<CDamage>(dmg);

    return bullet;
}

/*
* Spawn the correct bullet at location relative to given entity
*/
void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
    if (entity->tag() == "npc")
    {
        auto& weap = entity->getComponent<CWeapon>();
        if (entity->getComponent<CState>().state.find("Worm") != std::string::npos )
        {
            if (weap.lastFiredNpc == 0 || m_currentFrame - weap.lastFiredNpc >= 60)
            {
                weap.lastFiredNpc = m_currentFrame;
                
                auto bullet = m_entityManager.addEntity("EnemyBullet");
                bullet->addComponent<CAnimation>(m_game->assets().getAnimation("WormFire"), true);
                bullet->addComponent<CTransform>(Vec2(entity->getComponent<CTransform>().pos.x,
                entity->getComponent<CTransform>().pos.y));
                bullet->addComponent<CBoundingBox>(m_game->assets().getAnimation("WormFire").getSize());
                bullet->addComponent<CLifeSpan>(80, m_currentFrame);
                bullet->addComponent<CDamage>(entity->getComponent<CDamage>().damage);
                entity->getComponent<CState>().state = "Worm Attack";

                auto& scale = bullet->getComponent<CTransform>().scale.x;
                if (m_player->getComponent<CTransform>().pos.x - entity->getComponent<CTransform>().pos.x < 0) { scale = -1.0; }
                else if (m_player->getComponent<CTransform>().pos.x - entity->getComponent<CTransform>().pos.x > 0) { scale = 1.0; }
                bullet->getComponent<CTransform>().velocity = Vec2(m_playerConfig.SPEED, 0) * scale;
            }
        }

        if (entity->getComponent<CState>().state.find("Demon") != std::string::npos)
        {
            if (weap.frameCreated == 0 || m_currentFrame - weap.frameCreated >= 10)
            {
                weap.lastFiredNpc = m_currentFrame;
                entity->getComponent<CState>().state = "Demon Attack";
            }
        }
    }

    if (entity->tag() == "player")
    {
        auto& weap = m_player->getComponent<CWeapon>();
        // This means the player is standing idle and shooting so we set their attack state to shoot idle to play the correct animation
        // have to keep track of the frame created so we can choose how long we play the animation
        if (m_player->getComponent<CAnimation>().animation.getName() == "Stand" || m_player->getComponent<CAnimation>().animation.getName() == "Shoot")
        {
            weap.attackState = "ShootIdle";
            weap.frameCreated = m_currentFrame;
        }
        PlayerConfig& pc = m_playerConfig;
        CTransform& entityT = entity->getComponent<CTransform>();

        // Following code sets up the bullets based on the current weapon
        if (weap.currentWeapon == "Launcher")
        {
            // If enough time has passed since last firing bullet, it can be fired again
            if (weap.lastFiredLauncher == 0 || m_currentFrame - weap.lastFiredLauncher >= 240)
            {
                weap.lastFiredLauncher = m_currentFrame;
                Vec2 BULLET_SIZE = Vec2(67, 19);
                int BULLET_LIFETIME = 240;
                // int DMG = 4;
                int DMG = std::ceil(bulletScaler * 4);
                Vec2 pos = Vec2(entityT.pos.x + 26 * entityT.scale.x, entityT.pos.y);
                Vec2 velocity = Vec2(pc.SPEED * 1.25f * entityT.scale.x, 0.0f);
                auto bullet = setupBullet(BULLET_SIZE, pos, BULLET_LIFETIME, DMG, velocity, "Missile");
                m_game->playSound("se_bullet_missile");
            }
        }
        else if (weap.currentWeapon == "Bomb")
        {
            if (weap.lastFiredBomb == 0 || m_currentFrame - weap.lastFiredBomb >= 90)
            {
                auto& pScale = m_player->getComponent<CTransform>().scale;
                weap.lastFiredBomb = m_currentFrame;
                Vec2 BULLET_SIZE = Vec2(24, 24);
                int BULLET_LIFETIME = 180;
                // int DMG = 2;
                int DMG = std::ceil(bulletScaler * 2);
                Vec2 pos = Vec2(entityT.pos.x + 26 * entityT.scale.x, entityT.pos.y);
                Vec2 velocity = Vec2(pc.SPEED * entityT.scale.x * 2.0f, -15.0f * pScale.y);

                auto bullet = setupBullet(BULLET_SIZE, pos, BULLET_LIFETIME, DMG, velocity, "Bomb");
                bullet->addComponent<CGravity>(0.75);
                if (pScale.y == -1)
                {
                    bullet->getComponent<CGravity>().flipped = true;
                }
                else
                {
                    bullet->getComponent<CGravity>().flipped = false;
                }
                m_game->playSound("throw");
            }
        }
        else if (weap.currentWeapon == "Raygun")
        {
            if (weap.lastFiredRaygun == 0 || m_currentFrame - weap.lastFiredRaygun >= 15)
            {
                weap.lastFiredRaygun = m_currentFrame;
                Vec2 BULLET_SIZE = Vec2(30, 17);
                int BULLET_LIFETIME = 60;


                // DELETE 1
                // int damage = e->getComponent<CDamage>().damage;

                int DMG = std::ceil(bulletScaler*1);

                // std::cout << "Damage: " << bulletScaler << std::endl;

                Vec2 pos = Vec2(entityT.pos.x + 34 * entityT.scale.x, entityT.pos.y);
                Vec2 velocity = Vec2(pc.SPEED * entityT.scale.x * 2.5f, 0.0f);

                auto bullet = setupBullet(BULLET_SIZE, pos, BULLET_LIFETIME, DMG, velocity, "Laser");
                m_game->playSound("se_bullet_leaser");
            }
        }
    }
}

void Scene_Play::update()
{
    m_entityManager.update();

    if (!m_paused)
    {
        sMovement();
        sCollision();
        sLifespan();
        sAnimation();
        sAI();
    }
    sCamera();
    
    m_currentFrame++;
}

void Scene_Play::sMovement()
{
    PlayerConfig& pc = m_playerConfig;

    CTransform& transform = m_player->getComponent<CTransform>();
    CInput& input = m_player->getComponent<CInput>();
    CState& state = m_player->getComponent<CState>();
    CGravity& gravity = m_player->getComponent<CGravity>();
    CWeapon& weapon = m_player->getComponent<CWeapon>();

    // player movement

    // horizontal movement
    int dir = (input.right - input.left);

    if (state.state != "air") transform.velocity.x = dir * pc.SPEED;
    else
    {
        // only accelerate in x if player has jumped off a wall within last 30 frames
        if (m_currentFrame - m_player->getComponent<CJump>().lastJumped < 30)
        {
            if ((abs(transform.velocity.x) <= pc.SPEED) ||
                ((transform.velocity.x < 0 && dir > 0) || (transform.velocity.x > 0 && dir < 0))) transform.velocity.x += dir;
        }
        else { transform.velocity.x = dir * pc.SPEED; }
    }

    transform.prevScale.x = transform.scale.x;
    if (dir != 0) transform.scale.x = dir;

    // vertical movement
    if (state.state == "air")
    {
        if (input.sliding && 
           ((transform.velocity.y > 0 && gravity.gravity > 0) || (transform.velocity.y < 0 && gravity.gravity < 0)))
        {
            if ((input.right && transform.scale.x == 1) || (input.left && transform.scale.x == -1))
            {
                if (transform.scale.x == transform.prevScale.x) { transform.velocity.y = 2.5 * transform.scale.y; }
            }
            else 
            { 
                transform.velocity.y += gravity.gravity;
                if (input.up && input.canJump)
                {
                    input.canJump = false;
                    transform.velocity.y = gravity.gravity >= 0 ? pc.JUMP : -pc.JUMP;
                    transform.velocity.x += (pc.JUMP/4) * transform.scale.x;
                    transform.scale.x = -transform.scale.x;
                    m_player->getComponent<CJump>().lastJumped = m_currentFrame;
                }
            }
        }
        else 
        {   
            transform.velocity.y += gravity.gravity; 
        }

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

    if (m_touchedPlatform != NULL) transform.pos += m_touchedPlatform->getComponent<CTransform>().velocity;

    // weapon movement
    for (auto weapon : m_entityManager.getEntities("weapon"))
    {
        if (weapon->getComponent<CAnimation>().animation.getName() == m_player->getComponent<CWeapon>().currentWeapon)
        {
            auto& weapT = weapon->getComponent<CTransform>();
            weapT.scale = transform.scale;
            if (m_player->getComponent<CState>().state == "ground" && m_player->getComponent<CAnimation>().animation.getName() == "Stand")
            {
                if (weapT.scale.y == 1)
                { 
                    weapT.angle = 90.0 * weapT.scale.x;
                    weapT.pos = { m_player->getComponent<CTransform>().pos.x + 10 * weapT.scale.x, m_player->getComponent<CTransform>().pos.y + 32 };
                }
                else if (weapT.scale.y == -1)
                {
                    weapT.angle = 90.0 * weapT.scale.x * -1;
                    weapT.pos = { m_player->getComponent<CTransform>().pos.x + 10 * weapT.scale.x, m_player->getComponent<CTransform>().pos.y - 32 };
                }
            }
            else
            {
                weapT.angle = 0.0;
                weapT.pos = { m_player->getComponent<CTransform>().pos.x + 30 * weapT.scale.x, m_player->getComponent<CTransform>().pos.y + 4 * weapT.scale.y };
            }
        }
    }

    // player shooting
    if (input.shoot && input.canShoot)
    {
        input.canShoot = false;
        spawnBullet(m_player);
    }

    // bullet movement
    for (auto& b : m_entityManager.getEntities("bullet"))
    {
        CTransform& bt = b->getComponent<CTransform>();
        auto& weap = m_player->getComponent<CWeapon>();
        // Right now steering is not implemented as scale is set to 1, missile will just follow mouse currently
        // TODO: figure out best way to do steering for missile
        if (b->getComponent<CAnimation>().animation.getName() == "Missile")
        {
            float maxSpeed = m_playerConfig.SPEED * 1.25f;
            //Vec2 distVec = weap.target - Vec2(bt.pos.x, bt.pos.y);
            Vec2 distVec = windowToWorld(m_mPos) - Vec2(bt.pos.x, bt.pos.y);
            float dist = sqrtf(distVec.x * distVec.x + distVec.y * distVec.y);         
            Vec2 normalizeVec = distVec / dist;
            // scale to keep constant speed
            normalizeVec *= m_playerConfig.SPEED * 1.25f;

            // if within 20 pixels of target blow up
            if (abs(dist) <= 20)
            {
                b->removeComponent<CBoundingBox>();
                b->removeComponent<CLifeSpan>();
                b->getComponent<CTransform>().velocity = { 0.0, 0.0 };
                b->getComponent<CState>().state = "explosion";
                continue;
            }

            float scale = 0.03;
            Vec2 steering = normalizeVec - Vec2(bt.velocity.x, bt.velocity.y);
            steering *= scale;
            Vec2 actual = Vec2(bt.velocity.x, bt.velocity.y) + steering;

            Vec2 SIZE = Vec2(67, 19);
            bt.prevAngle = bt.angle;
            bt.angle = (atan2(actual.y, actual.x) * 180 / 3.14159265);
            // change bounding box of missile based on rotation
            if (abs(bt.angle) >= 60 && abs(bt.angle) <= 120) { b->addComponent<CBoundingBox>(Vec2(SIZE.y, SIZE.x)); }
            else { b->addComponent<CBoundingBox>(SIZE); }
            bt.velocity = { actual.x, actual.y};
        }
        // The bomb will have gravity
        if (b->hasComponent<CGravity>())
        {
            if (abs(b->getComponent<CTransform>().velocity.y) > pc.MAXSPEED)
            {
                if (!b->getComponent<CGravity>().flipped)
                {
                    b->getComponent<CTransform>().velocity.y = pc.MAXSPEED;
                }
                else
                {
                    b->getComponent<CTransform>().velocity.y = -pc.MAXSPEED;
                }
            }
            else
            {
                // if player is right side up
                if (!b->getComponent<CGravity>().flipped)
                {
                    b->getComponent<CTransform>().velocity.y += b->getComponent<CGravity>().gravity;
                }
                // if play is upside down
                else
                {
                    b->getComponent<CTransform>().velocity.y -= b->getComponent<CGravity>().gravity;
                }
            }
        }
        bt.pos += bt.velocity;
    }

    for (auto e : m_entityManager.getEntities("EnemyBullet"))
    {
        e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
        e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
    }

    for (auto bullet : m_entityManager.getEntities("bossbullet"))
    {
        bullet->getComponent<CTransform>().prevPos = bullet->getComponent<CTransform>().pos;
        bullet->getComponent<CTransform>().pos += bullet->getComponent<CTransform>().velocity;
    }

    // moving tiles
    for (auto tile : m_entityManager.getEntities("tile"))
    {
        if (tile->hasComponent<CPatrol>())
        {
            auto& tVel = tile->getComponent<CTransform>().velocity;
            auto& tSpeed = tile->getComponent<CPatrol>().speed;
            auto& tPos = tile->getComponent<CTransform>().pos;
            auto& positions = tile->getComponent<CPatrol>().positions;
            auto& index = tile->getComponent<CPatrol>().currentPosition;
            Vec2 currentPosition = positions[index];
            if (abs(currentPosition.x - tPos.x) <= 5 && abs(currentPosition.y - tPos.y) <= 5)
            {
                index = (index + 1) % positions.size();
            }
            else
            {
                Vec2 distVec = Vec2(currentPosition.x, currentPosition.y) - Vec2(tPos.x, tPos.y);
                float dist = sqrtf(distVec.x * distVec.x + distVec.y * distVec.y);
                Vec2 normalizeVec = distVec / dist;
                // make sure if the tile is within 5 pixels in x direction that they do not move
                // in the x direction
                if (abs(currentPosition.x - tPos.x) <= 5)
                {
                    normalizeVec.x = 0;
                }
                // do the same thing for y direction
                else if (abs(currentPosition.y - tPos.y) <= 5)
                {
                    normalizeVec.y = 0;
                }
                tVel = { normalizeVec.x * tSpeed, normalizeVec.y * tSpeed };
            }
            tile->getComponent<CTransform>().prevPos = tPos;
            tPos += tVel;
        }
    }
}

void Scene_Play::sAI()
{
    for (auto boss : m_entityManager.getEntities("boss"))
    {
        auto& prevPos = boss->getComponent<CTransform>().prevPos;
        auto& pos = boss->getComponent<CTransform>().pos;
        auto& velocity = boss->getComponent<CTransform>().velocity;
        auto& scale = boss->getComponent<CTransform>().scale;
        auto& state = boss->getComponent<CState>().state;

        if (boss->hasComponent<CBulletTimer>())
        {
            if (m_currentFrame - boss->getComponent<CBulletTimer>().frameCreated > boss->getComponent<CBulletTimer>().interval)
            {
                boss->getComponent<CBulletTimer>().frameCreated = m_currentFrame;

                for (int i = 0; i < 4; i++)
                {
                    auto bossBullet = m_entityManager.addEntity("bossbullet");
                    bossBullet->addComponent<CAnimation>(m_game->assets().getAnimation("BossBullet"), true);
                    bossBullet->addComponent<CBoundingBox>(Vec2(45, 45));
                    bossBullet->addComponent<CTransform>(pos);
                    bossBullet->addComponent<CLifeSpan>(300, m_currentFrame);
                    bossBullet->addComponent<CDamage>(1);
                    Vec2 bulletVelocity;
                    if (i == 0) { bulletVelocity = { 5, 0 }; }
                    else if (i == 1) { bulletVelocity = { -5, 0 }; }
                    else if (i == 2) { bulletVelocity = { 0, -5 }; }
                    else if (i == 3) { bulletVelocity = { 0, 5 }; }
                    bossBullet->getComponent<CTransform>().velocity = bulletVelocity;
                }
            }
        }

        if (boss->getComponent<CHealth>().current > 0)
        {
            Vec2 target = m_player->getComponent<CTransform>().pos;

            Vec2 distVec = target - pos;
            float dist = sqrtf(distVec.x * distVec.x + distVec.y * distVec.y);
            Vec2 normalizeVec = distVec / dist;

            velocity = { normalizeVec.x * 3, normalizeVec.y * 3 };

            scale.x = velocity.x > 0 ? 1.0 : -1.0;
            if (pos.dist(target) <= 75) { state = "attack"; }
            else { state = "idle"; }

            if (pos.dist(target) <= 50)
            {
                velocity = { 0.0, 0.0 };
            }
            prevPos = pos;
            pos += velocity;
        }
        else
        {
            if (!(state == "death"))
            {
                m_game->assets().getMusic("BossFight").stop();
                m_game->playSound("bossDeath");
                m_game->playSound("winSound");
                m_gameOver = true;
            }
            state = "death";
        }
    }
    for (auto e : m_entityManager.getEntities("npc"))
    {
        Vec2 aiVelocity(0, e->getComponent<CTransform>().velocity.y);
        // Implementing Patrol AI behaviour
        if (e->hasComponent<CPatrol>())
        {
            auto& pos = e->getComponent<CTransform>().pos;
            auto& vec = e->getComponent<CPatrol>().positions;
            auto& i = e->getComponent<CPatrol>().currentPosition;
            auto& speed = e->getComponent<CPatrol>().speed;
            auto& state = e->getComponent<CState>().state;

            if (pos.dist(vec[i]) <= 5)                              // if at next patrol position, change index to next patrol position
            {
                i = (i + 1) % vec.size();
            }

            else if ((vec[0].x <= m_player->getComponent<CTransform>().pos.x && m_player->getComponent<CTransform>().pos.x <= vec[1].x)
                || (vec[1].x <= m_player->getComponent<CTransform>().pos.x && m_player->getComponent<CTransform>().pos.x <= vec[0].x))
            {
                auto a = m_player->getComponent<CTransform>().pos;              // player position
                auto b = e->getComponent<CTransform>().pos;                     // npc position
                auto target = a + m_player->getComponent<CTransform>().velocity;

                auto dist = target.dist(b);
                auto x = speed * (target.x - b.x) / dist;
                //auto y = speed * (target.y - b.y) / dist;
                aiVelocity.x = x;
                spawnBullet(e);
            }

            else if (pos.dist(vec[i]) > 5)                                                 // if not at next patrol position, calc vector and move towards pos
            {
                auto dist = vec[i].dist(pos);
                auto x = speed * (vec[i].x - pos.x) / dist;
                //auto y = speed * (vec[i].y - pos.y) / dist;
                aiVelocity.x = x;
                state = state.substr(0, state.find(" ")) + " Walk";
            }
        }

        // Implementing Follow AI behaviour
        if (e->hasComponent<CFollowPlayer>())
        {
            auto a = m_player->getComponent<CTransform>().pos;              // player position
            auto b = e->getComponent<CTransform>().pos;                     // npc position
            auto home = e->getComponent<CFollowPlayer>().home;              // npc home position
            auto speed = e->getComponent<CFollowPlayer>().speed;            // npc follow speed
            auto& state = e->getComponent<CState>().state;
            bool intersect = false;                                          // bool to check if there is any intersection between ab

            for (auto p : m_entityManager.getEntities())
            {
                if (p->getComponent<CBoundingBox>().blockVision)
                {
                    if (Physics::EntityIntersect(a, b, p) || b.dist(a) > 500)
                    {
                        intersect = true;
                    }
                }
            }

            if (!intersect)                                     // if no intersection, move towards player
            {
                float maxDistance;
                if (state.find("Worm") != std::string::npos) { maxDistance = 500; }
                else { maxDistance = 50; }
                if (b.dist(a) > maxDistance)                              // if not at player position, calc vector and move towards player
                {
                    auto a = m_player->getComponent<CTransform>().pos;              // player position
                    auto b = e->getComponent<CTransform>().pos;                     // npc position
                    auto target = a + m_player->getComponent<CTransform>().velocity;

                    auto dist = target.dist(b);
                    auto x = speed * (target.x - b.x) / dist;
                    //auto y = speed * (target.y - b.y) / dist;
                    aiVelocity.x = x;
                    state = state.substr(0, state.find(" ")) + " Walk";
                }
                else                                                                 // if at desired posiiton stop moving
                {
                    aiVelocity = Vec2(0, 0);
                    spawnBullet(e);
                }
            }

            else                                                // if there is intersection, move towards home
            {
                if (b.dist(home) > 5)                           // if not at player position, calc vector and move towards home
                {
                    auto home = e->getComponent<CFollowPlayer>().home;               // npc home position
                    auto& b = e->getComponent<CTransform>().pos;                     // npc position
                    auto hdist = home.dist(b);
                    auto x = speed * (home.x - b.x) / hdist;
                    //auto y = speed * (home.y - b.y) / hdist;
                    aiVelocity.x = x;
                    state = state.substr(0, state.find(" ")) + " Walk";
                }
                else                                                                 // if at desired posiiton stop moving
                {
                    aiVelocity = Vec2(0, 0);
                    state = state.substr(0, state.find(" ")) + " Idle";
                }
            }
        }

        auto state = e->getComponent<CState>().state;
        if (aiVelocity.x < 0) { e->getComponent<CTransform>().scale.x = state.find("Demon") != std::string::npos ? 1.0 : -1.0; }
        else if (aiVelocity.x > 0) { e->getComponent<CTransform>().scale.x = state.find("Demon") != std::string::npos ? -1.0 : 1.0; }
        
        e->getComponent<CTransform>().velocity = aiVelocity;
        // gravity calculations
        if (e->hasComponent<CGravity>())
        {
            e->getComponent<CTransform>().scale.y = (e->getComponent<CGravity>().gravity < 0 ? -1 : 1);
            if (abs(e->getComponent<CTransform>().velocity.y) > m_playerConfig.MAXSPEED)
            {
                e->getComponent<CTransform>().velocity.y = m_playerConfig.MAXSPEED;
            }
            else
            {
                e->getComponent<CTransform>().velocity.y += e->getComponent<CGravity>().gravity;
            }
        }
        // Update positions based on movement
        e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
        e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
    }
}

void Scene_Play::sLifespan()
{
    // implemented lifespan
    for (auto& e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CLifeSpan>())
        {
            CLifeSpan& ls = e->getComponent<CLifeSpan>();
            if (m_currentFrame - ls.frameCreated > ls.lifespan)
            {
                if (e->getComponent<CAnimation>().animation.getName() == "Missile")
                {
                    e->getComponent<CState>().state = "explosion";
                }
                else { e->destroy(); }
            }
        }
    }

    for (auto e : m_entityManager.getEntities())
    {
        // implemented invincibility
        if (e->hasComponent<CInvincibility>())
        {
            auto& invincible = e->getComponent<CInvincibility>();
            invincible.iframes -= 1;
            if (invincible.iframes <= 0)
            {
                e->removeComponent<CInvincibility>();
            }
        }
    }

    // once duration has been exceeded change attack state so animation will be stopped playing
    // in sAnimation()
    if (m_player->getComponent<CWeapon>().attackState == "ShootIdle")
    {
        auto& weap = m_player->getComponent<CWeapon>();
        if (m_currentFrame - weap.frameCreated > weap.duration)
        {
            m_player->getComponent<CWeapon>().attackState = "Idle";
        }
    }
}

void Scene_Play::sInventory(std::string action, std::string name, int index)
{
    if (action == "add")
    {
        m_inventoryEntity->getComponent<CInventory>().in_Inventory[index] = true;
        m_inventoryEntity->getComponent<CInventory>().inventoryItems[index] = name;

        for (int i = 0; i < 5; i++)
        {
            if (m_inventoryEntity->getComponent<CInventory>().in_Inventory[i])
            {
                float x = m_game->window().getView().getCenter().x - width() / 2;
                float y = m_game->window().getView().getCenter().y - height() / 2 + 70;
                auto w = windowToWorld(Vec2(30, 90));
                //auto inv_t = Vec2(30 + x, 20 + y);
                auto inv_t = Vec2(0, 0);
                auto item = m_entityManager.addEntity("items");
                item->addComponent<CInventory>(i);
                item->addComponent<CTransform>(Vec2(inv_t.x + (73 * i), inv_t.y));
                auto s = m_inventoryEntity->getComponent<CInventory>().inventoryItems[i];
                std::cout << item->getComponent<CTransform>().pos.x << ", " << item->getComponent<CTransform>().pos.y << "\n";
                item->addComponent<CAnimation>(m_game->assets().getAnimation(s), true);
            }
        }
    }

    if (action == "use")
    {
        m_inventoryEntity->getComponent<CInventory>().in_Inventory[index] = false;
        for (auto& e : m_entityManager.getEntities("items"))
        {
            auto i = e->getComponent<CInventory>().index;
            if (!m_inventoryEntity->getComponent<CInventory>().in_Inventory[i])
            {
                // use item (); 
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

    m_touchedPlatform = NULL;
    m_player->getComponent<CState>().state = "air";
    m_player->getComponent<CInput>().sliding = false;
    for (auto& tile : m_entityManager.getEntities("tile"))
    {
        for (auto& e : m_entityManager.getEntities())
        {
            if (e->tag() == "player" || e->tag() == "bullet")
            {
                Vec2 overlap = Physics::GetOverlap(e, tile);
                if (overlap.x >= 0 && overlap.y >= 0)
                {
                    if (e->tag() == "player")
                    {
                        if (tile->getComponent<CAnimation>().animation.getName() == "Spaceship")
                        {
                            goal = true;
                            continue;
                        }
                        if (tile->hasComponent<CDamage>() && !e->hasComponent<CInvincibility>())
                        {
                            e->addComponent<CInvincibility>(30);
                            e->getComponent<CHealth>().current -= tile->getComponent<CDamage>().damage;


                            //DELETE
                            std::cout << "Damage to player: " << tile->getComponent<CDamage>().damage << std::endl;
                            std::cout << "Player health: " << e->getComponent<CHealth>().current << std::endl;
                            std::cout << typeid(tile->getComponent<CDamage>().damage).name() << '\n';
                            std::cout << typeid(e->getComponent<CHealth>().current).name() << '\n';




                            m_game->playSound("ow");
                            if (e->getComponent<CHealth>().current <= 0)
                            {
                                m_game->playSound("death");
                                e->destroy();
                                spawnPlayer();
                            }
                        }
                        
                        if (tile->hasComponent<CBoundingBox>() && !tile->getComponent<CBoundingBox>().blockMove) continue;

                        CTransform& et = e->getComponent<CTransform>();
                        CTransform& tileT = tile->getComponent<CTransform>();
                        Vec2 delta = et.pos - tileT.pos;
                        Vec2 prev = Physics::GetPreviousOverlap(e, tile);

                        // collison correction for player
                        if (prev.y > 0)
                        {
                            e->getComponent<CInput>().sliding = true;
                            et.pos.x += delta.x > 0 ? overlap.x : -overlap.x;
                            if ((delta.x >= 0 && et.velocity.x < 0) || (delta.x < 0 && et.velocity.x > 0)) et.velocity.x = 0;

                            for (auto weapon : m_entityManager.getEntities("weapon"))
                            {
                                if (weapon->getComponent<CAnimation>().animation.getName() == m_player->getComponent<CWeapon>().currentWeapon)
                                {
                                    weapon->getComponent<CTransform>().pos.x += delta.x > 0 ? overlap.x : -overlap.x;
                                }
                            }
                        }

                        else if (prev.x > 0)
                        {
                            if (delta.y > 0)
                            {
                                if (m_player->getComponent<CGravity>().gravity >= 0)
                                {
                                    if (et.velocity.y < 0) et.velocity.y = 0;
                                }
                                else
                                {
                                    m_player->getComponent<CState>().state = "ground";
                                    if (tile->hasComponent<CPatrol>() && m_touchedPlatform == NULL) m_touchedPlatform = tile;
                                }
                                et.pos.y += overlap.y;

                            }
                            else
                            {
                                if (m_player->getComponent<CGravity>().gravity >= 0)
                                {
                                    m_player->getComponent<CState>().state = "ground";
                                    if (tile->hasComponent<CPatrol>() && m_touchedPlatform == NULL) m_touchedPlatform = tile;
                                }
                                else
                                {
                                    if (et.velocity.y > 0) et.velocity.y = 0;
                                }
                                et.pos.y -= overlap.y;
                            }
                        }
                    }
                    else if (overlap.x != 0 && overlap.y != 0)
                    {
                        // if bullet collides with a tile set its state to impact to play the proper animation
                        if (e->tag() == "bullet")
                        {
                            e->getComponent<CState>().state = "explosion";
                        }
                        else
                        {
                            e->destroy();
                        }
                    }
                }
            }
        }
    }

    // boundary collison
    CTransform& transform = m_player->getComponent<CTransform>();
    CBoundingBox& box = m_player->getComponent<CBoundingBox>();

    // when the player goes to far left
    if (transform.pos.x < box.halfSize.x) transform.pos.x = box.halfSize.x;

    // when the player falls into a pit or falls into space
    float upperBarrier = ((30 - ceil(height() / m_gridSize.y)) * -m_gridSize.y);

    if (transform.pos.y > height() || transform.pos.y < upperBarrier)
    {
        m_game->playSound("death");
        m_player->destroy();
        spawnPlayer();
    }

    for (auto e : m_entityManager.getEntities("npc"))
    {
        if (e->getComponent<CState>().state.find("Demon") != std::string::npos)      // check if the entity is a demon npc entity
        {
            auto& tt = e->getComponent<CTransform>();               // npc entity transfom
            auto& bt = e->getComponent<CBoundingBox>();             // npc entity bounding box

            auto overlap = Physics::GetOverlap(m_player, e);        // get the overlap between player and npc entity

            if (overlap.x > 0 && overlap.y > 0)
            {
                if (m_player->hasComponent<CInvincibility>()) { break; }
                m_player->getComponent<CHealth>().current -= e->getComponent<CDamage>().damage;

                // DELETE
                std::cout << "Damage to player: " << e->getComponent<CDamage>().damage << std::endl;
                std::cout << "Player health: " << m_player->getComponent<CHealth>().current << std::endl;

                m_player->addComponent<CInvincibility>(60);
                m_game->playSound("ow");

                // destroy dead player and spawn new player
                if (m_player->getComponent<CHealth>().current <= 0)
                {
                    m_game->playSound("death");
                    m_player->destroy();
                    spawnPlayer();
                }
            }
        }
    }

    // Boss collisions
    for (auto boss : m_entityManager.getEntities("boss"))
    {
        if (Physics::GetOverlap(boss, m_player).x > 0 && Physics::GetOverlap(boss, m_player).y > 0)
        {
            if (!m_player->hasComponent<CInvincibility>())
            {
                m_player->getComponent<CHealth>().current -= boss->getComponent<CDamage>().damage;
                m_player->addComponent<CInvincibility>(30);
                m_game->playSound("ow");
                if (m_player->getComponent<CHealth>().current <= 0)
                {
                    m_player->getComponent<CHealth>().current = 0;
                    m_game->playSound("death");
                    loadBoss();
                }
            }
        }
        for (auto bullet : m_entityManager.getEntities("bullet"))
        {
            if (Physics::GetOverlap(boss, bullet).x > 0 && Physics::GetOverlap(boss, bullet).y > 0)
            {
                // don't care about boss taking damage when invincible, just wanna apply red shader each time they are hit
                boss->addComponent<CInvincibility>(10);
                m_game->playSound("bossHit");
                bullet->getComponent<CState>().state = "explosion";
                boss->getComponent<CHealth>().current -= bullet->getComponent<CDamage>().damage;
            }
            // the boss is dead
            if (boss->getComponent<CHealth>().current <= 0)
            {
                boss->getComponent<CHealth>().current = 0;               
                boss->removeComponent<CBoundingBox>();
                boss->removeComponent<CDamage>();
                boss->removeComponent<CBulletTimer>();
            }
        }
    }

    // Boss bullet collisions
    for (auto bBullet : m_entityManager.getEntities("bossbullet"))
    {
        if (Physics::GetOverlap(bBullet, m_player).x > 0 && Physics::GetOverlap(bBullet, m_player).y > 0)
        {
            if (!m_player->hasComponent<CInvincibility>())
            {
                m_player->getComponent<CHealth>().current -= bBullet->getComponent<CDamage>().damage;
                m_player->addComponent<CInvincibility>(30);
                m_game->playSound("ow");
                if (m_player->getComponent<CHealth>().current <= 0)
                {
                    m_player->getComponent<CHealth>().current = 0;
                    m_player->destroy();
                    spawnPlayer();
                }
            }
        }
    }

    // handling bullet and npc collisiobs
    for (auto& npc : m_entityManager.getEntities("npc"))
    {
        for (auto& bullet : m_entityManager.getEntities("bullet"))
        {
            // if there is a collision adjust npc health and set bullet state
            if (Physics::GetOverlap(npc, bullet).x > 0 && Physics::GetOverlap(npc, bullet).y > 0)
            {
                bullet->getComponent<CState>().state = "explosion";
                npc->getComponent<CHealth>().current -= bullet->getComponent<CDamage>().damage;
                auto& state = npc->getComponent<CState>().state;
                state = state.substr(0, state.find(" ")) + " Hit";

                //DELETE
                std::cout << "bullet hit npc damge:   " << bullet->getComponent<CDamage>().damage << std::endl;
                std::cout << "npc health: " << npc->getComponent<CHealth>().current << std::endl;
            }

            // the npc is dead
            if (npc->getComponent<CHealth>().current <= 0)
            {
                npc->getComponent<CHealth>().current = 0;
                if (npc->hasComponent<CPatrol>()) { npc->removeComponent<CPatrol>(); }
                else if (npc->hasComponent<CFollowPlayer>()) { npc->removeComponent<CFollowPlayer>(); }

                auto& animation = npc->getComponent<CAnimation>().animation;
                auto& state = npc->getComponent<CState>().state;

                npc->removeComponent<CBoundingBox>();
                npc->removeComponent<CDamage>();
                npc->removeComponent<CGravity>();
                npc->getComponent<CTransform>().velocity = { 0.0, 0.0 };
            }
        }
    }

    // if the bullet has impacted something need to remove and adjust components
    for (auto& bullet : m_entityManager.getEntities("bullet"))
    {
        if (bullet->getComponent<CState>().state == "explosion")
        {
            bullet->removeComponent<CBoundingBox>();
            bullet->removeComponent<CLifeSpan>();
            if (bullet->hasComponent<CGravity>()) { bullet->removeComponent<CGravity>(); }
            bullet->removeComponent<CDamage>();
            bullet->getComponent<CTransform>().velocity = { 0.0, 0.0 };
        }
    }

    for (auto& e : m_entityManager.getEntities("npc"))
    {
        auto& nt = e->getComponent<CTransform>();                       // npc transform
        auto& nb = e->getComponent<CBoundingBox>();                     // npc bounding box

        for (auto& t : m_entityManager.getEntities("tile"))
        {
            auto& tt = t->getComponent<CTransform>();               // tile transfom
            auto overlap = Physics::GetOverlap(e, t);               // get the overlap between npc and tile

            if (t->getComponent<CBoundingBox>().blockMove && overlap.x >= 0 && overlap.y >= 0)
            {

                auto prev = Physics::GetPreviousOverlap(e, t);
                if (prev.y > 0)                                     // horizontal collision
                {
                    if (nt.pos.x < tt.pos.x)                        // collision from right side of tile
                    {
                        nt.velocity.x = 0;
                        nt.pos.x -= overlap.x;
                    }
                    if (nt.pos.x > tt.pos.x)                        // collision from left side of tile
                    {
                        nt.velocity.x = 0;
                        nt.pos.x += overlap.x;
                    }
                }

                if (prev.x > 0)                                     // vertical collision 
                {
                    if (nt.pos.y < tt.pos.y)                        // collision from top of tile
                    {
                        nt.velocity.y = 0;
                        nt.pos.y -= overlap.y;
                    }
                    if (nt.pos.y > tt.pos.y)                        // collision from bottom of tile
                    {
                        nt.velocity.y = 0;
                        nt.pos.y += overlap.y;
                    }
                }
            }
        }
    }

    for (auto& f : m_entityManager.getEntities("EnemyBullet"))
    {
        auto& tt = f->getComponent<CTransform>();               // fireball entity transfom
        auto& bt = f->getComponent<CBoundingBox>();             // fireball entity bounding box

        for (auto& e : m_entityManager.getEntities())
        {
            if (e->tag() == "player" || e->tag() == "tile")
            {
                auto overlap = Physics::GetOverlap(e, f);        // get the overlap between player and fireball

                if(Physics::IsInside(tt.pos, e))
                {
                    f->addComponent<CAnimation>(m_game->assets().getAnimation("WormExplode"), false);
                    if (e->tag() == "player")
                    {
                        m_player->addComponent<CInvincibility>(10);
                        m_player->getComponent<CHealth>().current -= f->getComponent<CDamage>().damage;

                        //DELETE
                        std::cout << "Damage to player: " << f->getComponent<CDamage>().damage << std::endl;
                        std::cout << "Player health: " << m_player->getComponent<CHealth>().current << std::endl;
                        m_game->playSound("ow");
                    }
                    f->destroy();

                    // destroy dead player and spawn new player
                    if (m_player->getComponent<CHealth>().current <= 0)
                    {
                        m_game->playSound("death");
                        m_player->destroy();
                        spawnPlayer();
                    }
                }
            }
        }
    }

    // item collisions
    for (auto& e : m_entityManager.getEntities("item"))
    {
        auto overlap = Physics::GetOverlap(m_player, e);        // get the overlap between player and item
        if (overlap.x > 0 && overlap.y > 0)
        {
            if (e->hasComponent<CAnimation>() && e->getComponent<CAnimation>().animation.getName() == "PillRed")
            {
                e->destroy();
                m_player->getComponent<CHealth>().current = m_player->getComponent<CHealth>().max;
                continue;
            }

            auto index = e->getComponent<CInventory>().index;
            /*m_inventoryEntity->getComponent<CInventory>().in_Inventory[index] = true;
            m_inventoryEntity->getComponent<CInventory>().inventoryItems[index] = e->getComponent<CAnimation>().animation.getName();*/
            e->destroy();
            sInventory("add", e->getComponent<CAnimation>().animation.getName(), index);
        }
    }

    // if player has reached end of the level
    if (goal)
    {
        m_game->assets().getMusic(m_levelMusic).stop();
        if (m_game->progress == 3 && m_levelPath == "level3.txt")
        {
            if (m_transition == 0) { m_game->playSound("winSound"); }
            m_transition++;
            if (m_transition == 275) { loadBoss(); }
        }
        else
        {
            if (m_countdown == 150) { m_game->playSound("winSound"); }
            m_countdown--;
            // Change scene after m_countdown number of frames has passed
            m_action = 1;
            if (m_countdown <= 0) onEnd();
        }
    }
}

void Scene_Play::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
        else if (action.name() == "INVENTORY") { m_inventory = !m_inventory; setPaused(!m_paused); }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "TOGGLE_OPTION_MENU")
        {
            setPaused(true);
            setOptionMenu(true);
        }
        else if (action.name() == "PAUSE") { setPaused(!m_paused); }
        else if (action.name() == "QUIT") { onEnd(); }
        else if (action.name() == "BOSS") { loadBoss(); }

        else if (action.name() == "UP") 
        {
            m_player->getComponent<CInput>().up = true;
            if (m_inventory)
            {
                sInventory("use", "item", m_invSelect);
            }
        }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = true; }
        else if (action.name() == "LEFT") 
        {
            m_player->getComponent<CInput>().left = true; 
            if (m_inventory)
            {
                m_invSelect--;
                if (m_invSelect < 0) m_invSelect = 4;
            }
        }
        else if (action.name() == "RIGHT") 
        {
            m_player->getComponent<CInput>().right = true; 
            if (m_inventory) m_invSelect = (m_invSelect + 1) % 5;
        }

        else if (action.name() == "SHOOT") 
        { 
            m_player->getComponent<CInput>().shoot = true; 
        }
        else if (action.name() == "MOUSE_MOVE") { m_mPos = action.pos(); }

        else if (action.name() == "RAYGUN")   { m_player->getComponent<CWeapon>().currentWeapon = "Raygun";   }
        else if (action.name() == "BOMB")     { m_player->getComponent<CWeapon>().currentWeapon = "Bomb";     }
        else if (action.name() == "LAUNCHER") { m_player->getComponent<CWeapon>().currentWeapon = "Launcher"; }

        else if (action.name() == "LEFT_CLICK")
        {
            Vec2 worldPos = windowToWorld(m_mPos);
            std::cout << "Mouse Clickled: " << worldPos.x << ", " << worldPos.y << "\n";
            for (auto e : m_entityManager.getEntities())
            {
                if (e->hasComponent<CClickable>() && Physics::IsInside(worldPos, e))
                {
                    e->getComponent<CClickable>().clicking = !e->getComponent<CClickable>().clicking;
                }
            }
        }
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
        if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = false; }
        if (action.name() == "SHOOT")
        {
            m_player->getComponent<CInput>().shoot = false;
            m_player->getComponent<CInput>().canShoot = true;
        }
    }
}

void Scene_Play::sAnimation()
{
    // player animations
    CAnimation& pAni = m_player->getComponent<CAnimation>();

    // bullet animations
    for (auto& bullet : m_entityManager.getEntities("bullet"))
    {
        // play explosion animation
        if (bullet->getComponent<CState>().state == "explosion")
        {
            if (bullet->getComponent<CAnimation>().animation.getName() != "Explosion")
            {
                bullet->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
                m_game->playSound("explosion");
            }
        }
    }

    // boss animations
    for (auto boss : m_entityManager.getEntities("boss"))
    {
        if (boss->getComponent<CState>().state == "death" && !(boss->getComponent<CAnimation>().animation.getName() == "BossDeath"))
        {
            boss->addComponent<CAnimation>(m_game->assets().getAnimation("BossDeath"), false);
        }
        else if (boss->getComponent<CState>().state == "idle" && !(boss->getComponent<CAnimation>().animation.getName() == "BossIdle"))
        {
            boss->addComponent<CAnimation>(m_game->assets().getAnimation("BossIdle"), true);
        }
        else if (boss->getComponent<CState>().state == "attack" && !(boss->getComponent<CAnimation>().animation.getName() == "BossAttack"))
        {
            boss->addComponent<CAnimation>(m_game->assets().getAnimation("BossAttack"), true);
        }
    }

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
            // when they are standing and not attacking
            if (pAni.animation.getName() != "Stand" && !(m_player->getComponent<CWeapon>().attackState == "ShootIdle"))
            {
                m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
            }
            // standing and attacking
            else if (pAni.animation.getName() != "Shoot" && m_player->getComponent<CWeapon>().attackState == "ShootIdle")
            {
                m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Shoot"), true);
            }
        }
    }
    // animation when player is in the air
    else
    {
        if (pAni.animation.getName() != "Air" && !m_player->getComponent<CInput>().sliding)
        {
            m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Air"), true);
        }
        else if (pAni.animation.getName() != "Slide" && m_player->getComponent<CInput>().sliding)
        {
            m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Slide"), true);
        }
    }

    for (auto& e : m_entityManager.getEntities("npc"))
    {
        auto& state = e->getComponent<CState>().state;
        auto& animation = e->getComponent<CAnimation>().animation.getName();
        if (e->getComponent<CHealth>().current <= 0)
        {
            if (animation.find("Worm") != std::string::npos)
            {
                if (!(animation == "WormDeath"))
                {
                    e->addComponent<CAnimation>(m_game->assets().getAnimation("WormDeath"), false);
                }
            }
            else
            {
                if (!(animation == "DemonDeath"))
                {
                    e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonDeath"), false);
                }
            }
            break;
        }
        if (state.find("Worm") != std::string::npos)
        {
            if (state == "Worm Idle" && animation != "WormIdle")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("WormIdle"), true);
            }
            else if (state == "Worm Walk" && animation != "WormWalk")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("WormWalk"), true);
            }
            else if (state == "Worm Attack" && animation != "WormAttack")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("WormAttack"), true);
            }
            else if (state == "Worm Hit" && animation != "WormHit")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("WormHit"), true);
            }
        }

        else if (state.find("Demon") != std::string::npos)
        {
            if (state == "Demon Idle" && animation != "DemonIdle")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonIdle"), true);
            }
            else if (state == "Demon Walk" && animation != "DemonWalk")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonWalk"), true);
            }
            else if (state == "Demon Attack" && animation != "DemonAttack")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonAttack"), true);
            }
            else if (state == "Demon Hit" && animation != "DemonHit")
            {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonHit"), true);
            }
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

void Scene_Play::sClick()
{
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CClickable>() && e->getComponent<CClickable>().clicking)
        {
            //Vec2 worldPos = windowToWorld(m_mPos);
            //auto index = e->getComponent<CInventory>().index;
            //inventoryItems[index] = false;
            //m_inventoryEntity->getComponent<CInventory>().in_Inventory[index] = false;
        }
    }
}

Vec2 Scene_Play::windowToWorld(const Vec2& window) const
{
    auto view = m_game->window().getView();

    float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
    float wy = view.getCenter().y - (m_game->window().getSize().y / 2);

    return Vec2(window.x + wx, window.y + wy);
}

void Scene_Play::onEnd()
{
    m_hasEnded = true;
    m_game->assets().getMusic(m_levelMusic).stop();
    m_game->assets().getMusic("OverWorld").setLoop(true);
    m_game->playMusic("OverWorld");

    if (m_action == 0) m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game));
    else
    {
        if (m_levelPath == "level1.txt" && m_game->progress == 1 || m_levelPath == "level2.txt" && m_game->progress == 2)
        {
            m_game->progress++;
            if (m_game->progress > 3) m_game->progress = 3;
        }

        m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game));
    }
}

/*
* Set the positions of each set of layers constantly for parallaxing
*/
void Scene_Play::updateBackgrounds()
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
            if (abs(playerTransform.pos.x - backgroundOnePos.x) <= abs(playerTransform.velocity.x - playerTransform.velocity.x * eScrollFactor) + 5)
            {
                if (playerTransform.pos.x > playerTransform.prevPos.x)
                {
                    backgroundTwoPos.x = backgroundOnePos.x + m_game->window().getSize().x - eVelocity.x;
                }
                else
                {
                    backgroundTwoPos.x = backgroundOnePos.x - m_game->window().getSize().x - eVelocity.x;
                }
            }
            if (abs(playerTransform.pos.x - backgroundTwoPos.x) <= abs(playerTransform.velocity.x - playerTransform.velocity.x * eScrollFactor) + 5)
            {
                if (playerTransform.pos.x > playerTransform.prevPos.x)
                {
                    backgroundOnePos.x = backgroundTwoPos.x + m_game->window().getSize().x - eVelocity.x;
                }
                else
                {
                    backgroundOnePos.x = backgroundTwoPos.x - m_game->window().getSize().x - eVelocity.x;
                }
            }
        }
        if (playerTransform.pos.x <= m_game->window().getSize().x / 2.0f)
        {
            ePos.x = eOriginalPos.x;
        }
        if (playerTransform.pos.y >= m_game->window().getSize().y / 2.0f)
        {
            ePos.y = eOriginalPos.y;
        }
    }
}

void Scene_Play::sCamera()
{
    auto& pPos = m_player->getComponent<CTransform>().pos;

    sf::View view = m_game->window().getView();
    float viewCenterX = m_game->window().getView().getCenter().x;
    float viewCenterY = m_game->window().getView().getCenter().y;
    float windowX = m_game->window().getSize().x;
    float windowY = m_game->window().getSize().y;

    // needed to determine speed for parallax scrolling
    m_prevCameraPos = Vec2(viewCenterX - windowX / 2.0f, viewCenterY - windowY / 2.0f);
    sf::View gameView(sf::FloatRect(0, 0, windowX, windowY));
    float windowCenterX = std::max(windowX / 2.0f, pPos.x);
    float windowCenterY = std::min(windowY / 2.0f, pPos.y);
    // set the viewport of the window to be centered on the player if it's far enough right and far enough up
    gameView.setCenter(windowCenterX, windowCenterY);
    gameView.setViewport(sf::FloatRect(0, 0, 1, 1));
    
    if (goal && m_transition < 275) // if player at the end of level, zoom in on player
    {
        Vec2 before(m_player->getComponent<CTransform>().pos.x - view.getCenter().x - (m_game->window().getSize().x / 2),
            m_player->getComponent<CTransform>().pos.y - view.getCenter().y - (m_game->window().getSize().y / 2));
        
        gameView.zoom(0.6f);
        m_game->window().setView(gameView);

        Vec2 after(m_player->getComponent<CTransform>().pos.x - view.getCenter().x - (m_game->window().getSize().x / 2),
            m_player->getComponent<CTransform>().pos.y - view.getCenter().y - (m_game->window().getSize().y / 2));
        
        Vec2 offset = before - after;
        
        gameView.move(sf::Vector2f(offset.x, offset.y + 32));
    }
    m_game->window().setView(gameView);
    
    if (!(m_prevCameraPos == Vec2(0, 0)))
    {
        // update scroll background movement based on camera movement
        for (auto e : m_entityManager.getEntities("scrollbackground"))
        {
            auto& eTran = e->getComponent<CTransform>();
            eTran.velocity.x = ((gameView.getCenter().x - m_game->window().getSize().x / 2.0f) - m_prevCameraPos.x) * eTran.scrollFactor;
            eTran.velocity.y = ((gameView.getCenter().y - m_game->window().getSize().y / 2.0f) - m_prevCameraPos.y) * eTran.scrollFactor;
            eTran.pos += eTran.velocity;
        }
    }
    
    // always keep no scroll background on screen
    for (auto e : m_entityManager.getEntities("noscrollbackground"))
    {
        auto& ePos = e->getComponent<CTransform>().pos;
        ePos.x = m_game->window().getView().getCenter().x;
        ePos.y = m_game->window().getView().getCenter().y;
    }

    if (!(m_prevCameraPos == Vec2(0, 0))) { updateBackgrounds(); }
}

/*
* Get the entire night time sprite to be drawn on the window
* 
* Passing in sprite for gauge so it is drawn over the dark texture
*/
sf::Sprite Scene_Play::getLightingSprite(sf::Sprite gaugeSprite)
{
    // this is for lighting
    sf::BlendMode blendMode(
        sf::BlendMode::Factor::Zero,
        sf::BlendMode::Factor::DstColor,
        sf::BlendMode::Equation::Add,
        sf::BlendMode::Factor::Zero,
        sf::BlendMode::Factor::OneMinusSrcAlpha,
        sf::BlendMode::Equation::Add);

    // keep the sprite at it's original position
    gaugeSprite.setPosition(735, 10);

    // create the lighting sprite and set its position
    sf::Sprite light(m_lightTexture);
    light.setOrigin(light.getTexture()->getSize().x / 2.0f, light.getTexture()->getSize().y / 2.0f);
    auto& pPos = m_player->getComponent<CTransform>().pos;
    float x_pos = std::min(pPos.x, m_game->window().getSize().x / 2.0f);
    float y_pos = std::max(pPos.y, m_game->window().getSize().y / 2.0f);
    light.setPosition(x_pos, y_pos);
    // draw the lighting sprite and a rect over the healthbar to keep it the same colour
    m_renderTexture.clear();
    m_renderTexture.draw(gaugeSprite, blendMode);
    m_renderTexture.draw(light, blendMode);
    m_renderTexture.display();
    // get the night time level sprite based off the render texture
    sf::Sprite night(m_renderTexture.getTexture());
    night.setOrigin(night.getTexture()->getSize().x / 2.0f, night.getTexture()->getSize().y / 2.0f);
    float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, m_player->getComponent<CTransform>().pos.x);
    float windowCenterY = std::min(m_game->window().getSize().y / 2.0f, m_player->getComponent<CTransform>().pos.y);
    night.setPosition(windowCenterX, windowCenterY);
    float luminosity = 240.0f;
    night.setColor(sf::Color(10.0f, 10.0f, 10.0f, luminosity));

    return night;
}

/*
* Display a weapon box sprite at given position
*/
sf::Sprite Scene_Play::displayBox(float x, float y)
{
    sf::Sprite box(m_game->assets().getTexture("TexWeaponBox"));

    box.setOrigin(sf::Vector2f(m_game->assets().getTexture("TexWeaponBox").getSize().x / 2, m_game->assets().getTexture("TexWeaponBox").getSize().y / 2));
    box.setPosition(x, y);

    return box;
}

/*
* Display given text at given location
*/
sf::Text Scene_Play::displayText(std::string text, float x, float y)
{
    m_weaponUIText.setString(text);
    m_weaponUIText.setPosition(x, y);

    return m_weaponUIText;
}

/*
* Draw the weapon display which will be 3 boxes that appear in the bottom middle of the screen
* with the weapon sprites
*/
void Scene_Play::drawWeaponDisplay()
{
    float viewCenterX = m_game->window().getView().getCenter().x;
    float viewCenterY = m_game->window().getView().getCenter().y;
    float windowSizeYHalf = m_game->window().getSize().y / 2.0f;
    const int size = 48;

    m_game->window().draw(displayBox(viewCenterX - size - 555, viewCenterY + windowSizeYHalf - size / 2 - 12));
    m_game->window().draw(displayBox(viewCenterX - 535, viewCenterY + windowSizeYHalf - size / 2 - 12));
    m_game->window().draw(displayBox(viewCenterX + size - 515, viewCenterY + windowSizeYHalf - size / 2 - 12));

    m_game->window().draw(displayText("1", viewCenterX - size - 575, viewCenterY + windowSizeYHalf - size / 2 - 36));
    m_game->window().draw(displayText("2", viewCenterX - 555, viewCenterY + windowSizeYHalf - size / 2 - 36));
    m_game->window().draw(displayText("3", viewCenterX + size - 535, viewCenterY + windowSizeYHalf - size / 2 - 36));

    sf::Sprite raygunDisplay(m_game->assets().getTexture("TexRaygun"));
    raygunDisplay.setOrigin(sf::Vector2f(size / 2, size / 2));
    raygunDisplay.setPosition(viewCenterX - size - 555, viewCenterY + windowSizeYHalf - size / 2 - 6);
    m_game->window().draw(raygunDisplay);

    sf::Sprite bombDisplay(m_game->assets().getTexture("TexBomb"));
    bombDisplay.setOrigin(sf::Vector2f(size / 2, size / 2));
    bombDisplay.setPosition(viewCenterX - 535, viewCenterY + windowSizeYHalf - size / 2 - 8);
    m_game->window().draw(bombDisplay);

    sf::Sprite launcherDisplay(m_game->assets().getTexture("TexLauncher"));
    launcherDisplay.setOrigin(sf::Vector2f(size / 2, size / 2));
    launcherDisplay.setPosition(viewCenterX + size - 515, viewCenterY + windowSizeYHalf - size / 2 - 6);
    m_game->window().draw(launcherDisplay);
}

/*
* Draw the current weapon in the correct location relative to player
*/
void Scene_Play::drawWeapon()
{
    for (auto weapon : m_entityManager.getEntities("weapon"))
    {
        auto& animation = weapon->getComponent<CAnimation>().animation;
        if (animation.getName() == m_player->getComponent<CWeapon>().currentWeapon)
        {
            auto& transform = weapon->getComponent<CTransform>();
            animation.getSprite().setRotation(transform.angle);
            animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
            animation.getSprite().setScale(transform.scale.x, transform.scale.y);
            m_game->window().draw(animation.getSprite());
        }
    }
}

sf::Text Scene_Play::getText(std::string str)
{
    sf::Text text;
    text.setFont(m_game->assets().getFont("Roboto"));
    text.setCharacterSize(60);
    text.setFillColor(sf::Color::White);
    text.setString(str);
    text.setOrigin(text.getLocalBounds().width / 2 + text.getLocalBounds().left,
        text.getLocalBounds().height / 2 + text.getLocalBounds().top);

    return text;
}

void Scene_Play::sRender()
{
    // color the background darker so you know that the game is paused
    //if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
    //else { m_game->window().clear(sf::Color(50, 50, 150)); }
    m_game->window().clear();

    sf::RectangleShape tick({ 1.0f, 6.0f });
    tick.setFillColor(sf::Color::Black);

    // need to set parameters for electric shader
    float ran = (float)rand() / (RAND_MAX);
    electric_shader.setUniform("rnd", ran);
    electric_shader.setUniform("intensity", 0.99f);
    // need to set parameters for rainbow shader and red shader
    rainbow_shader.setUniform("time", time.getElapsedTime().asSeconds());

    // draw all Entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities())
        {
            auto& transform = e->getComponent<CTransform>();
            sf::Color c = sf::Color::White;

            if (e->hasComponent<CAnimation>() && e->tag() != "inventory" && e->tag() != "items" && e->tag() != "weapon")
            {
                auto& animation = e->getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(transform.angle);
                animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
                animation.getSprite().setScale(transform.scale.x, transform.scale.y);

                if (e->hasComponent<CInvincibility>() && !(e->tag() == "boss"))
                {
                    c = sf::Color(255, 255, 255, 128);
                    animation.getSprite().setColor(c);
                }
                else
                {
                    c = sf::Color(255, 255, 255, 255);
                    animation.getSprite().setColor(c);
                }

                if (e->tag() == "player" && m_night && !(e->hasComponent<CInvincibility>())) { m_game->window().draw(animation.getSprite(), &bright_shader); }
                //if (e->tag() == "player") { m_game->window().draw(animation.getSprite(), &speed_shader); }
                else if (e->tag() == "boss" && e->hasComponent<CInvincibility>()) { m_game->window().draw(animation.getSprite(), &red_shader); }
                else { m_game->window().draw(animation.getSprite()); }
            }
        }

        for (auto e : m_entityManager.getEntities())
        {
            auto& transform = e->getComponent<CTransform>();
            auto& animation = e->getComponent<CAnimation>().animation;

            // the health bar for the npc
            if (e->tag() == "npc" || e->tag() == "boss" && e->hasComponent<CHealth>())
            {
                auto& h = e->getComponent<CHealth>();
                Vec2 size;
                sf::RectangleShape rect;
                if (e->tag() == "npc")
                {
                    size = Vec2(64, 6);
                    rect.setSize({size.x, size.y});
                    rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
                }
                else
                {
                    size = Vec2(128, 6);
                    rect.setSize({ size.x, size.y });
                    rect.setPosition(transform.pos.x - 70, transform.pos.y - 74);
                }
                rect.setFillColor(sf::Color(96, 96, 96));
                rect.setOutlineColor(sf::Color::Black);
                rect.setOutlineThickness(2);
                m_game->window().draw(rect);

                float ratio = (float)(h.current) / h.max;
                size.x *= ratio;
                rect.setSize({ size.x, size.y });
                rect.setFillColor(sf::Color(255, 0, 0));
                rect.setOutlineThickness(0);
                m_game->window().draw(rect);

                if (e->tag() == "npc")
                {
                    for (int i = 0; i < h.max; i++)
                    {
                        tick.setPosition(rect.getPosition() + sf::Vector2f(i * 64 * 1 / h.max, 0));
                        m_game->window().draw(tick);
                    }
                }
            }
        }
    }
    // draw the current weapon
    drawWeapon();

    /*
    * Setup the minimap view: Load mini map and mini player textures, set scale and create sprites
    */

    sf::Texture gaugeTexture, miniPlayerTexture;
    gaugeTexture.loadFromFile("images/new/Gauge.png");
    gaugeTexture.setSmooth(true);

    miniPlayerTexture.loadFromFile("images/new/miniPlayer.png");
    miniPlayerTexture.setSmooth(true);

    sf::Sprite gauge, miniPlayer;

    gauge.setScale(2, 2);
    gauge.setTexture(gaugeTexture);

    miniPlayer.setTexture(miniPlayerTexture);

    // set position of mini map at top left of window
    gauge.setPosition(m_game->window().getView().getCenter().x - width() / 2 + 735, m_game->window().getView().getCenter().y - height() / 2 + 10);
    m_game->window().draw(gauge); 

    // set posiiton and drae mini map player at top left of the window relative to the player's position in game
    for (auto e : m_entityManager.getEntities())
    {
        auto transform = e->getComponent<CTransform>();
        if (e->hasComponent<CAnimation>() && e->tag() == "player")
        {
            auto x = m_game->window().getView().getCenter().x - width() / 2 + 700 + (transform.pos.x * gauge.getGlobalBounds().width / m_levelEnd);
            miniPlayer.setPosition(x, m_game->window().getView().getCenter().y - height() / 2 + 10);
            m_game->window().draw(miniPlayer);
        }
    }

    // draw all Entity collision bounding boxes with a rectangleshape
    if (m_drawCollision)
    {
        sf::CircleShape dot(4);
        dot.setFillColor(sf::Color::Black);
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

                if (box.blockMove && box.blockVision) { rect.setOutlineColor(sf::Color::Black); }
                if (box.blockMove && !box.blockVision) { rect.setOutlineColor(sf::Color::Blue); }
                if (!box.blockMove && box.blockVision) { rect.setOutlineColor(sf::Color::Red); }
                if (!box.blockMove && !box.blockVision) { rect.setOutlineColor(sf::Color::White); }

                rect.setOutlineThickness(1);
                m_game->window().draw(rect);
            }

            if (e->hasComponent<CPatrol>())
            {
                auto& patrol = e->getComponent<CPatrol>().positions;
                for (size_t p = 0; p < patrol.size(); p++)
                {
                    dot.setPosition(patrol[p].x, patrol[p].y);
                    m_game->window().draw(dot);
                }
            }

            if (e->hasComponent<CFollowPlayer>())
            {
                sf::VertexArray lines(sf::LinesStrip, 2);
                lines[0].position.x = e->getComponent<CTransform>().pos.x;
                lines[0].position.y = e->getComponent<CTransform>().pos.y;
                lines[0].color = sf::Color::Black;
                lines[1].position.x = m_player->getComponent<CTransform>().pos.x;
                lines[1].position.y = m_player->getComponent<CTransform>().pos.y;
                lines[1].color = sf::Color::Black;
                m_game->window().draw(lines);
                dot.setPosition(e->getComponent<CFollowPlayer>().home.x, e->getComponent<CFollowPlayer>().home.y);
                m_game->window().draw(dot);
            }
        }
    }

    // only do lighting for night time levels
    if (m_night)
    {
        sf::Sprite sprite = getLightingSprite(gauge);
        m_game->window().draw(sprite);
    }

    // draw player health
    auto& health = m_player->getComponent<CHealth>();

    Vec2 upperLeftCorner = Vec2((m_game->window().getView().getCenter().x - width() / 2),
        (m_game->window().getView().getCenter().y - height() / 2));

    float w = m_game->assets().getTexture("TexHeart").getSize().x;
    float h = m_game->assets().getTexture("TexHeart").getSize().y;
    bool even = true;
    for (int i = 0; i < health.current; i++)
    {
        if (even)
        {
            sf::Texture ht(m_game->assets().getTexture(i + 1 != health.current ? "TexHeart" : "TexBrokenHeart"));

            sf::Sprite border(ht);
            border.setColor(sf::Color(0, 0, 0, 200));
            border.setPosition(upperLeftCorner.x + (w * i / 2) + (w * 0.1), upperLeftCorner.y + h * 0.1);
            border.setScale(1.1, 1.1);
            m_game->window().draw(border);

            sf::Sprite heart(ht);
            heart.setPosition(upperLeftCorner.x + (w * i / 2), upperLeftCorner.y);
            m_game->window().draw(heart);
        }
        even = !even;
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

    if (m_optionMenuOpen)
    {   
        m_game->changeScene("OPTIONMENU", std::make_shared<Scene_OptionMenu>(m_game), false);
    }

    if (m_inventory)
    {
        for (auto e : m_entityManager.getEntities("inventory"))
        {
            auto& transform = e->getComponent<CTransform>();
            transform.pos = Vec2(m_game->window().getView().getCenter().x - width() / 2, m_game->window().getView().getCenter().y - height() / 2 + 70);
            auto& animation = e->getComponent<CAnimation>().animation;

            sf::RectangleShape rect;
            rect.setSize(sf::Vector2f(animation.getSize().y, animation.getSize().y));
            //rect.setOrigin(0,0);
            float x = m_game->window().getView().getCenter().x - width() / 2;
            float y = m_game->window().getView().getCenter().y - height() / 2 + 70;
            rect.setPosition(transform.pos.x + ((m_gridSize.x + 8) * m_invSelect) + 8.5, transform.pos.y);
            rect.setFillColor(sf::Color(0, 0, 0, 0));
            rect.setOutlineColor(sf::Color(100, 200, 100));
            rect.setOutlineThickness(6);

            m_game->window().draw(rect);

            animation.getSprite().setRotation(transform.angle);
            animation.getSprite().setOrigin(0, 0);
            animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
            animation.getSprite().setScale(transform.scale.x, transform.scale.y);
            m_game->window().draw(animation.getSprite());
        }

        for (auto e : m_entityManager.getEntities("items"))
        {
            auto transform = e->getComponent<CTransform>();
            float x = m_game->window().getView().getCenter().x - width() / 2;
            float y = m_game->window().getView().getCenter().y - height() / 2 + 70;
            auto inv_t = Vec2(20 + x, 15 + y);
            transform.pos += Vec2(inv_t.x, inv_t.y);
            //std::cout << "Render stuff: " << e->getComponent<CInventory>().index  << " ";
            //std::cout << transform.pos.x << ", " << transform.pos.y << "\n";
            auto& animation = e->getComponent<CAnimation>().animation;
            animation.getSprite().setRotation(transform.angle);
            animation.getSprite().setOrigin(0, 0);
            animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
            animation.getSprite().setScale(transform.scale.x, transform.scale.y);
            m_game->window().draw(animation.getSprite());
        }
    }
    // Draw the weapon display UI
    drawWeaponDisplay();

    if (goal && m_transition < 275) // if level complete, display level complete text just above player's head
    {
        sf::Text gameOverText;
        gameOverText.setString("Congratulations! Level Completed");
        gameOverText.setFont(m_game->assets().getFont("Roboto"));
        gameOverText.setCharacterSize(20);
        gameOverText.setFillColor(sf::Color::White);
        gameOverText.setOrigin(gameOverText.getLocalBounds().width / 2 + gameOverText.getLocalBounds().left,
            gameOverText.getLocalBounds().height / 2 + gameOverText.getLocalBounds().top);
        gameOverText.setPosition(sf::Vector2f(m_player->getComponent<CTransform>().pos.x, m_player->getComponent<CTransform>().pos.y - 80));
        m_game->window().draw(gameOverText);
    }

    if (m_game->progress == 3 && m_transition < 275)
    {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));
        rect.setOrigin(m_game->window().getSize().x / 2.0f, m_game->window().getSize().y / 2.0f);
        rect.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y);
        int transparency = m_transition;
        if (m_transition > 255) { transparency = 255; }
        rect.setFillColor(sf::Color(0, 0, 0, transparency));
        m_game->window().draw(rect);
    }

    if (m_gameOver)
    {
        m_creditCountdown++;
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));
        rect.setOrigin(m_game->window().getSize().x / 2.0f, m_game->window().getSize().y / 2.0f);
        rect.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y);
        if (m_creditCountdown < 255)
        {
            int transparency = m_creditCountdown;
            rect.setFillColor(sf::Color(0, 0, 0, transparency));

            sf::Text winText;
            winText.setString("Congratulations You Win!");
            winText.setFont(m_game->assets().getFont("Roboto"));
            winText.setCharacterSize(40);
            winText.setFillColor(sf::Color::White);
            winText.setOrigin(winText.getLocalBounds().width / 2 + winText.getLocalBounds().left,
                winText.getLocalBounds().height / 2 + winText.getLocalBounds().top);
            winText.setPosition(sf::Vector2f(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y));
            m_game->window().draw(rect);
            m_game->window().draw(winText);
        }
        if (m_creditCountdown >= 255)
        {
            rect.setFillColor(sf::Color(0, 0, 0, 255));
            setPaused(true);
            m_game->window().draw(rect);

            if (m_creditCountdown == 255)
            {
                sf::Text text;
                for (int i = 0; i < 6; i++)
                {
                    if (i == 0)
                    {
                        text = getText("Game Credits");
                        text.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y);
                        m_credits.push_back(text);
                    }
                    else if (i == 1)
                    {
                        text = getText("Developed By");
                        text.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y + 200);
                        m_credits.push_back(text);
                    }
                    else if (i == 2)
                    {
                        text = getText("Brendon Thorne");
                        text.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y + 400);
                        m_credits.push_back(text);
                    }
                    else if (i == 3)
                    {
                        text = getText("Ryland (lastname)");
                        text.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y + 525);
                        m_credits.push_back(text);
                    }
                    else if (i == 4)
                    {
                        text = getText("Jeff Anga");
                        text.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y + 650);
                        m_credits.push_back(text);
                    }
                    else if (i == 5)
                    {
                        text = getText("Ze Liu");
                        text.setPosition(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y + 775);
                        m_credits.push_back(text);
                    }
                }
            }
            else
            {
                for (int i = 0; i < m_credits.size(); i++)
                {
                    m_credits[i].setPosition(m_credits[i].getPosition().x, m_credits[i].getPosition().y - 1.5);
                    m_game->window().draw(m_credits[i]);
                }

                if (m_creditCountdown == 1150)
                {
                    m_gameOver = false;
                    m_game->progress = 3;
                    m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game));
                }
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
