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
#include <math.h>

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

    registerAction(sf::Keyboard::Num1, "RAYGUN");
    registerAction(sf::Keyboard::Num2, "BOMB");
    registerAction(sf::Keyboard::Num3, "LAUNCHER");

    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Space, "SHOOT");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Tech"));

    m_weaponUIText.setCharacterSize(14);
    m_weaponUIText.setFont(m_game->assets().getFont("Tech"));

    inventoryItems = { false, false, false, false, false };
    loadLevel(levelPath);
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

    m_game->assets().getSound("OverWorld").stop();
    //m_game->playSound("Play");

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
        if (temp == "Dec" || temp == "Tile")
        {
            std::string type = temp;
            std::string texture;
            float x, y;
            int bm, bv;
            fin >> texture >> x >> y;
            auto tile = m_entityManager.addEntity(type == "Dec" ? "dec" : "tile");
            tile->addComponent<CAnimation>(m_game->assets().getAnimation(texture), true);
            tile->addComponent<CTransform>(gridToMidPixel(x, y, tile));

            if (type == "Tile") 
            {
                fin >> bm >> bv;
                tile->addComponent<CBoundingBox>(tile->getComponent<CAnimation>().animation.getSize());
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
        }
        else if (temp == "NPC")
        {
            std::string tileName, AI;
            int x, y;
            fin >> tileName >> x >> y >> AI;

            auto npc = m_entityManager.addEntity("npc");
            Vec2 worldPos = gridToMidPixel(x, y, npc);

            npc->addComponent<CAnimation>(m_game->assets().getAnimation(tileName), true);
            npc->addComponent<CTransform>(gridToMidPixel(x, y, npc));
            if (tileName == "DemonAttack") { npc->addComponent<CBoundingBox>(m_game->assets().getAnimation(tileName).getSize()); }
            else                                         { npc->addComponent<CBoundingBox>(m_gridSize); }
            npc->addComponent<CState>(tileName);
            npc->addComponent<CHealth>(5, 5);
            npc->addComponent<CDamage>(2);
            if (AI == "Patrol")
            {
                for (int i = 0; i < 2; i++)
                {
                    fin >> x1 >> y1;
                    pos.push_back(Vec2(gridToMidPixel(x1, y1, npc)));
                }
                npc->addComponent<CPatrol>(pos, 2);
                pos.clear();
            }
            else if (AI == "Follow")
            {
                npc->addComponent<CFollowPlayer>(gridToMidPixel(x, y, npc), 2);
            }

        }
        else if (temp == "Item")
        {
            std::string tileName;
            float x, y;
            fin >> tileName >> x >> y;
            //Vec2 tSize = m_game->assets().getAnimation(tileName).getSize();

            auto item = m_entityManager.addEntity("item");

            item->addComponent<CAnimation>(m_game->assets().getAnimation(tileName), true);
            item->addComponent<CTransform>(gridToMidPixel(x, y, item));
            item->addComponent<CBoundingBox>(m_game->assets().getAnimation(tileName).getSize());
            item->addComponent<CInventory>(i++);
            item->addComponent<CClickable>();
            //std::cout << item->getComponent<CInventory>().index << "\n";
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
    
    if (filename == "level1.txt") { m_level = 1;  }
    if (filename == "level2.txt") { m_level = 2; }
    if (filename == "level3.txt") { m_level = 3; }

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
    m_player->addComponent<CHealth>(10, 10);
    m_player->addComponent<CDamage>(2);
    m_player->addComponent<CState>("air");

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
    bullet->addComponent<CBoundingBox>(size);
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
        if (entity->getComponent<CState>().state == "WormIdle")
        {
            if (weap.lastFiredRaygun == 0 || m_currentFrame - weap.lastFiredRaygun >= 70)
            {
                weap.lastFiredRaygun = m_currentFrame;
                auto bullet = m_entityManager.addEntity("EnemyBullet");
                bullet->addComponent<CAnimation>(m_game->assets().getAnimation("WormFire"), true);
                bullet->addComponent<CTransform>(Vec2(entity->getComponent<CTransform>().pos.x,
                    entity->getComponent<CTransform>().pos.y));
                bullet->addComponent<CBoundingBox>(m_game->assets().getAnimation("WormFire").getSize());
                bullet->addComponent<CLifeSpan>(50, m_currentFrame);
                bullet->addComponent<CDamage>(entity->getComponent<CDamage>().damage);

                auto& scale = bullet->getComponent<CTransform>().scale.x;
                if (m_player->getComponent<CTransform>().pos.x - entity->getComponent<CTransform>().pos.x < 0) { scale = -1.0; }
                else if (m_player->getComponent<CTransform>().pos.x - entity->getComponent<CTransform>().pos.x > 0) { scale = 1.0; }
                bullet->getComponent<CTransform>().velocity = Vec2(m_playerConfig.SPEED, 0) * scale;
            }
        }

        if (entity->getComponent<CState>().state == "DemonWalk")
        {
            if (weap.frameCreated == 0 || m_currentFrame - weap.frameCreated >= 4)
            {
                weap.lastFiredRaygun = m_currentFrame;
                entity->addComponent<CAnimation>(m_game->assets().getAnimation("DemonAttack"), true);
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
        auto bullet = m_entityManager.addEntity("bullet");
        // Following code sets up the bullets based on the current weapon
        if (weap.currentWeapon == "Launcher")
        {
            // If enough time has passed since last firing bullet, it can be fired again
            if (weap.lastFiredLauncher == 0 || m_currentFrame - weap.lastFiredLauncher >= 240)
            {
                weap.lastFiredLauncher = m_currentFrame;
                Vec2 BULLET_SIZE = Vec2(67, 19);
                int BULLET_LIFETIME = 240;
                int DMG = 1;
                Vec2 pos = Vec2(entityT.pos.x + 26 * entityT.scale.x, entityT.pos.y);
                Vec2 velocity = Vec2(pc.SPEED * entityT.scale.x * 1.25f, 0.0f);

            auto bullet = setupBullet(BULLET_SIZE, pos, BULLET_LIFETIME, DMG, velocity, "Missile");
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
            int DMG = 1;
            Vec2 pos = Vec2(entityT.pos.x + 26 * entityT.scale.x, entityT.pos.y);
            Vec2 velocity = Vec2(pc.SPEED * entityT.scale.x * 2.0f, -15.0f * pScale.y);

            auto bullet = setupBullet(BULLET_SIZE, pos, BULLET_LIFETIME, DMG, velocity, "Bomb");
            bullet->addComponent<CGravity>(pc.GRAVITY);
            if (pScale.y == -1)
            {
                bullet->getComponent<CGravity>().flipped = true;
            }
            else
            {
                bullet->getComponent<CGravity>().flipped = false;
            }
        }
    }
    else if (weap.currentWeapon == "Raygun")
    {
        if (weap.lastFiredRaygun == 0 || m_currentFrame - weap.lastFiredRaygun >= 15)
        {
            weap.lastFiredRaygun = m_currentFrame;
            Vec2 BULLET_SIZE = Vec2(30, 17);
            int BULLET_LIFETIME = 60;
            int DMG = 1;
            Vec2 pos = Vec2(entityT.pos.x + 34 * entityT.scale.x, entityT.pos.y);
            Vec2 velocity = Vec2(pc.SPEED * entityT.scale.x * 2.5f, 0.0f);

            auto bullet = setupBullet(BULLET_SIZE, pos, BULLET_LIFETIME, DMG, velocity, "Laser");
            m_game->playSound("se_bullet_leaser");
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
    if (input.shoot)
    {
        spawnBullet(m_player);
    }

    // bullet movement
    for (auto& b : m_entityManager.getEntities("bullet"))
    {
        CTransform& bt = b->getComponent<CTransform>();
        // Right now steering is not implemented as scale is set to 1, missile will just follow mouse currently
        // TODO: figure out best way to do steering for missile
        if (b->getComponent<CAnimation>().animation.getName() == "Missile")
        {
            Vec2 worldPos = windowToWorld(m_mPos);
            Vec2 distVec = worldPos - Vec2(bt.pos.x, bt.pos.y);
            float dist = sqrtf(distVec.x * distVec.x + distVec.y * distVec.y);         
            Vec2 normalizeVec = distVec / dist;
            float dist2 = sqrtf(bt.velocity.x * bt.velocity.x + bt.velocity.y * bt.velocity.y);
            normalizeVec *= dist2;

            if (abs(dist) <= 5)
            {
                b->removeComponent<CBoundingBox>();
                b->removeComponent<CLifeSpan>();
                b->getComponent<CTransform>().velocity = { 0.0, 0.0 };
                b->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
                continue;
            }

            float scale = 1;
            Vec2 steering = normalizeVec - Vec2(bt.velocity.x, bt.velocity.y);
            steering *= scale;
            Vec2 actual = Vec2(bt.velocity.x, bt.velocity.y) + steering;

            bt.angle = (atan2(actual.y, actual.x) * 180 / 3.14159265);
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

    for (auto& c : m_entityManager.getEntities("coinbullet"))
    {
        if (c->hasComponent<CBoundingBox>())
        {
            CTransform& ct = c->getComponent<CTransform>();
            ct.pos.x += ct.velocity.x;
        }
    }

    for (auto e : m_entityManager.getEntities("npc"))
    {
        // Implementing Patrol AI behaviour
        if (e->tag() == "npc" && e->hasComponent<CPatrol>())
        {
            auto& pos = e->getComponent<CTransform>().pos;
            auto& vec = e->getComponent<CPatrol>().positions;
            auto& i = e->getComponent<CPatrol>().currentPosition;
            auto& speed = e->getComponent<CPatrol>().speed;
            //e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonWalk"), true);

            if (pos.dist(vec[i]) <= 5)                              // if at next patrol position, change index to next patrol position
            {
                i = (i + 1) % vec.size();
            }

            else if (vec[0].x <= m_player->getComponent<CTransform>().pos.x && m_player->getComponent<CTransform>().pos.x <= vec[1].x)
            {
                auto a = m_player->getComponent<CTransform>().pos;              // player position
                auto b = e->getComponent<CTransform>().pos;                     // npc position
                auto target = a + m_player->getComponent<CTransform>().velocity;

                auto dist = target.dist(b);
                auto x = speed * (target.x - b.x) / dist;
                auto y = speed * (target.y - b.y) / dist;
                e->getComponent<CTransform>().velocity = Vec2(x, 0);
                if (e->getComponent<CAnimation>().animation.getName() != "DemonAttack")
                {
                    e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonAttack"), true);
                }
                
            }

            else if (pos.dist(vec[i]) > 5)                                                 // if not at next patrol position, calc vector and move towards pos
            {
                auto dist = vec[i].dist(pos);
                auto x = speed * (vec[i].x - pos.x) / dist;
                auto y = speed * (vec[i].y - pos.y) / dist;
                e->getComponent<CTransform>().velocity = Vec2(x, 0);
                e->addComponent<CAnimation>(m_game->assets().getAnimation("DemonWalk"), true);
            }

            else                                                                 // if at desired posiiton stop moving
            {
                e->getComponent<CTransform>().velocity = Vec2(0, 0);
                //spawnBullet(e);
            }

            if (e->getComponent<CTransform>().velocity.x < 0) { e->getComponent<CTransform>().scale.x = 1.0; }
            else { e->getComponent<CTransform>().scale.x = -1.0; }
        }

        // Implementing Follow AI behaviour
        if (e->tag() == "npc" && e->hasComponent<CFollowPlayer>())
        {
            auto a = m_player->getComponent<CTransform>().pos;              // player position
            auto b = e->getComponent<CTransform>().pos;                     // npc position
            auto home = e->getComponent<CFollowPlayer>().home;              // npc home position
            auto speed = e->getComponent<CFollowPlayer>().speed;            // npc follow speed
            bool intersect = false;                                          // bool to check if there is any intersection between ab


            if (b.dist(a) > 500)
            {
                intersect = true;
            }

            if (!intersect)                                     // if no intersection, move towards player
            {
                if (b.dist(a) > 300)                              // if not at player position, calc vector and move towards player
                {
                    auto a = m_player->getComponent<CTransform>().pos;              // player position
                    auto b = e->getComponent<CTransform>().pos;                     // npc position
                    auto target = a + m_player->getComponent<CTransform>().velocity;

                    auto dist = target.dist(b);
                    auto x = speed * (target.x - b.x) / dist;
                    auto y = speed * (target.y - b.y) / dist;
                    e->getComponent<CTransform>().velocity = Vec2(x, 0);
                }
                else                                                                 // if at desired posiiton stop moving
                {
                    e->getComponent<CTransform>().velocity = Vec2(0, 0);
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
                    auto y = speed * (home.y - b.y) / hdist;
                    e->getComponent<CTransform>().velocity = Vec2(x, 0);
                }
                else                                                                 // if at desired posiiton stop moving
                {
                    e->getComponent<CTransform>().velocity = Vec2(0, 0);
                }
            }
            if      (m_player->getComponent<CTransform>().pos.x - e->getComponent<CTransform>().pos.x < 0) { e->getComponent<CTransform>().scale.x = -1.0; }
            else if (m_player->getComponent<CTransform>().pos.x - e->getComponent<CTransform>().pos.x > 0) { e->getComponent<CTransform>().scale.x = 1.0; }
        }


        // add velocity for movement
        e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
        e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
    }
    for (auto e : m_entityManager.getEntities("EnemyBullet"))
    {
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
                e->destroy();
            }
        }
    }

    // implemented invincibility
    if (m_player->hasComponent<CInvincibility>())
    {
        auto& invincible = m_player->getComponent<CInvincibility>();
        invincible.iframes -= 1;

        if (invincible.iframes > 0)
        {
            m_player->getComponent<CAnimation>().animation.getSprite().setColor(sf::Color::Transparent);
        }
        if (invincible.iframes <= 0)
        {
            m_player->removeComponent<CInvincibility>();
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

    //for (auto& i : m_entityManager.getEntities("item"))
    //{
    //    if (inventoryItems[i->getComponent<CInventory>().index])
    //    {
    //        float x = m_game->window().getView().getCenter().x - width() / 2;
    //        auto inv_t = Vec2(7 + x, 3);
    //        auto& item = m_entityManager.addEntity("items");
    //        item->addComponent<CTransform>(Vec2(inv_t.x + (73 * i->getComponent<CInventory>().index), inv_t.y));
    //        auto s = i->getComponent<CAnimation>().animation.getName();
    //        std::cout << s << "; " << i->getComponent<CInventory>().index << ";  ";
    //        std::cout << item->getComponent<CTransform>().pos.x << ", " << item->getComponent<CTransform>().pos.y << "\n";
    //        item->addComponent<CAnimation>(m_game->assets().getAnimation(s), true);
    //    }
    //}

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
                        if (e->tag() == "bullet")
                        {
                            e->removeComponent<CBoundingBox>();
                            e->removeComponent<CLifeSpan>();
                            e->removeComponent<CGravity>();
                            e->getComponent<CTransform>().velocity = { 0.0, 0.0 };
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
                    //m_player->getComponent<CCoinCounter>().coins++;
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
    }

    // item collisions
    for (auto& e : m_entityManager.getEntities("item"))
    {
        auto overlap = Physics::GetOverlap(m_player, e);        // get the overlap between player and coin
        if (overlap.x >= 0 && overlap.y >= 0)
        {
            auto index = e->getComponent<CInventory>().index;
            inventoryItems[index] = true;
            m_inventoryEntity->getComponent<CInventory>().in_Inventory[index] = true;
            e->destroy();
        }
    }

    auto& tp = m_player->getComponent<CTransform>();    // player transform
    auto& bp = m_player->getComponent<CBoundingBox>();  // player bounding box
    for (auto& e : m_entityManager.getEntities("npc"))
    {
        if (e->getComponent<CState>().state == "DemonWalk")      // check if the entity is a demon npc entity
        {
            auto& tt = e->getComponent<CTransform>();               // npc entity transfom
            auto& bt = e->getComponent<CBoundingBox>();             // npc entity bounding box

            auto overlap = Physics::GetOverlap(m_player, e);        // get the overlap between player and npc entity

            if (overlap.x >= 0 && overlap.y >= 0)
            {
                if (m_player->hasComponent<CInvincibility>()) { break; }
                m_player->getComponent<CHealth>().current -= e->getComponent<CDamage>().damage;
                m_player->addComponent<CInvincibility>(60);
                //m_game->playSound("LinkHurt");

                // destroy dead player and spawn new player
                if (m_player->getComponent<CHealth>().current <= 0)
                {
                    //m_game->playSound("LinkDie");
                    m_player->destroy();
                    spawnPlayer();
                }
            }
        }
    }

    for (auto& npc : m_entityManager.getEntities("npc"))
    {
        for (auto& bullet : m_entityManager.getEntities("bullet"))
        {
            if (Physics::GetOverlap(bullet, npc).x > 0 && Physics::GetOverlap(bullet, npc).y > 0)
            {
                npc->getComponent<CHealth>().current -= bullet->getComponent<CDamage>().damage;
                bullet->removeComponent<CBoundingBox>();
                bullet->removeComponent<CLifeSpan>();
                if (bullet->hasComponent<CGravity>()) bullet->removeComponent<CGravity>();
                bullet->removeComponent<CDamage>();
                bullet->getComponent<CTransform>().velocity = { 0.0, 0.0 };
                bullet->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
            }
            if (npc->getComponent<CHealth>().current <= 0)
            {
                // playing death animation doesn't work
                auto& animation = npc->getComponent<CAnimation>().animation;
                if (animation.getName().find("Worm") != std::string::npos)
                {
                    if (!(animation.getName() == "WormDeath")) { npc->addComponent<CAnimation>(m_game->assets().getAnimation("WormDeath"), false); }
                }
                else
                {
                    if (!(animation.getName() == "DemonDeath")) { npc->addComponent<CAnimation>(m_game->assets().getAnimation("DemonDeath"), false); }
                }
                if (npc->hasComponent<CPatrol>())
                {
                    npc->removeComponent<CPatrol>();
                }
                else if (npc->hasComponent<CFollowPlayer>())
                {
                    npc->removeComponent<CFollowPlayer>();
                }
                npc->removeComponent<CBoundingBox>();
                npc->removeComponent<CDamage>();
                if (npc->hasComponent<CGravity>()) { npc->removeComponent<CGravity>(); }
                npc->getComponent<CTransform>().velocity = { 0.0, 0.0 };
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
                        m_player->getComponent<CHealth>().current -= f->getComponent<CDamage>().damage;
                    }
                    f->destroy();

                    // destroy dead player and spawn new player
                    if (m_player->getComponent<CHealth>().current <= 0)
                    {
                        //m_game->playSound("LinkDie");
                        m_player->destroy();
                        spawnPlayer();
                    }
                }
            }
        }
        
    }

    if (goal)
    {
        m_level++;
        m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game, m_level));
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
        else if (action.name() == "PAUSE") { setPaused(!m_paused); }
        else if (action.name() == "QUIT") { onEnd(); }

        else if (action.name() == "UP") { m_player->getComponent<CInput>().up = true; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = true; }
        else if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = true; }

        else if (action.name() == "SHOOT") { m_player->getComponent<CInput>().shoot = true; }
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
    m_game->assets().getSound("Play").stop();
    m_game->playSound("OverWorld");

    sf::View gameView(sf::FloatRect(0, 0, 1280, 768));
    gameView.setViewport(sf::FloatRect(0, 0, 1, 1));
    m_game->window().setView(gameView);
    m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game, m_level));
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
            if (abs(playerTransform.pos.x - backgroundOnePos.x) <= abs(playerTransform.velocity.x - playerTransform.velocity.x * eScrollFactor))
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
            if (abs(playerTransform.pos.x - backgroundTwoPos.x) <= abs(playerTransform.velocity.x - playerTransform.velocity.x * eScrollFactor))
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
    // set the viewport of the window to be centered on the player if it's far enough right
    auto& pPos = m_player->getComponent<CTransform>().pos;
    // needed to determine speed for parallax scrolling
    float viewCenterX = m_game->window().getView().getCenter().x;
    float viewCenterY = m_game->window().getView().getCenter().y;
    float windowX = m_game->window().getSize().x;
    float windowY = m_game->window().getSize().y;
    m_prevCameraPos = Vec2(viewCenterX - windowX / 2.0f, viewCenterY - windowY / 2.0f);
    sf::View gameView(sf::FloatRect(0, 0, windowX, windowY));
    float windowCenterX = std::max(windowX / 2.0f, pPos.x);
    float windowCenterY = std::min(windowY / 2.0f, pPos.y);
    gameView.setCenter(windowCenterX, windowCenterY);
    gameView.setViewport(sf::FloatRect(0, 0, 1, 1));
    m_game->window().setView(gameView);

    // update scroll background movement based on camera movement
    for (auto e : m_entityManager.getEntities("scrollbackground"))
    {
        auto& eTran = e->getComponent<CTransform>();
        eTran.velocity.x = ((gameView.getCenter().x - m_game->window().getSize().x / 2.0f) - m_prevCameraPos.x) * eTran.scrollFactor;
        eTran.velocity.y = ((gameView.getCenter().y - m_game->window().getSize().y / 2.0f) - m_prevCameraPos.y) * eTran.scrollFactor;
        eTran.pos += eTran.velocity;
    }

    // always keep no scroll background on screen
    for (auto e : m_entityManager.getEntities("noscrollbackground"))
    {
        auto& ePos = e->getComponent<CTransform>().pos;
        ePos.x = m_game->window().getView().getCenter().x;
        ePos.y = m_game->window().getView().getCenter().y;
    }

    updateBackgrounds();
}

/*
* Get the lighting sprite to be drawn on the screen around player for illumination
*/
sf::Sprite Scene_Play::getLightingSprite()
{
    // this is for lighting
    sf::BlendMode blendMode(
        sf::BlendMode::Factor::Zero,
        sf::BlendMode::Factor::DstColor,
        sf::BlendMode::Equation::Add,
        sf::BlendMode::Factor::Zero,
        sf::BlendMode::Factor::OneMinusSrcAlpha,
        sf::BlendMode::Equation::Add);

    sf::Sprite light(m_lightTexture);
    light.setOrigin(light.getTexture()->getSize().x / 2.0f, light.getTexture()->getSize().y / 2.0f);
    auto& pPos = m_player->getComponent<CTransform>().pos;
    float x_pos = std::min(pPos.x, m_game->window().getSize().x / 2.0f);
    float y_pos = std::max(pPos.y, m_game->window().getSize().y / 2.0f);
    light.setPosition(x_pos, y_pos);
    m_renderTexture.clear();
    m_renderTexture.draw(light, blendMode);
    m_renderTexture.display();
    sf::Sprite night(m_renderTexture.getTexture());
    night.setOrigin(night.getTexture()->getSize().x / 2, night.getTexture()->getSize().y / 2);
    float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, m_player->getComponent<CTransform>().pos.x);
    float windowCenterY = std::min(m_game->window().getSize().y / 2.0f, m_player->getComponent<CTransform>().pos.y);
    night.setPosition(windowCenterX, windowCenterY);
    float luminosity = 240.0f;
    night.setColor(sf::Color(10.0f, 10.0f, 10.0f, luminosity));

    return night;
}

/*
* Display a rectangle with white outline at given position with given size
*/
sf::RectangleShape Scene_Play::displayRect(float x, float y, const int size)
{
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(size, size));
    rect.setOrigin(sf::Vector2f(size / 2, size / 2));
    rect.setPosition(x, y);
    rect.setFillColor(sf::Color(0, 0, 0, 0));
    rect.setOutlineColor(sf::Color(255, 255, 255, 255));
    rect.setOutlineThickness(1);
   
    return rect;
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
    m_game->window().draw(displayText("1", viewCenterX - size - 20, viewCenterY + windowSizeYHalf - size / 2 - 30));
    m_game->window().draw(displayText("2", viewCenterX - 20, viewCenterY + windowSizeYHalf - size / 2 - 30));
    m_game->window().draw(displayText("3", viewCenterX + size - 20, viewCenterY + windowSizeYHalf - size / 2 - 30));

    sf::Sprite raygunDisplay(m_game->assets().getTexture("TexRaygun"));
    raygunDisplay.setOrigin(sf::Vector2f(size / 2, size / 2));
    raygunDisplay.setPosition(viewCenterX - size, viewCenterY + windowSizeYHalf - size / 2 - 4);
    m_game->window().draw(raygunDisplay);

    sf::Sprite bombDisplay(m_game->assets().getTexture("TexBomb"));
    bombDisplay.setOrigin(sf::Vector2f(size / 2, size / 2));
    bombDisplay.setPosition(viewCenterX, viewCenterY + windowSizeYHalf - size / 2 - 4);
    m_game->window().draw(bombDisplay);

    sf::Sprite launcherDisplay(m_game->assets().getTexture("TexLauncher"));
    launcherDisplay.setOrigin(sf::Vector2f(size / 2, size / 2));
    launcherDisplay.setPosition(viewCenterX + size, viewCenterY + windowSizeYHalf - size / 2 - 4);
    m_game->window().draw(launcherDisplay);

    m_game->window().draw(displayRect(viewCenterX - size, viewCenterY + windowSizeYHalf - size / 2 - 4, size));
    m_game->window().draw(displayRect(viewCenterX, viewCenterY + windowSizeYHalf - size / 2 - 4, size));
    m_game->window().draw(displayRect(viewCenterX + size, viewCenterY + windowSizeYHalf - size / 2 - 4, size));
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

void Scene_Play::sRender()
{
    // color the background darker so you know that the game is paused
    //if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
    //else { m_game->window().clear(sf::Color(50, 50, 150)); }
    m_game->window().clear();

    sf::RectangleShape tick({ 1.0f, 6.0f });
    tick.setFillColor(sf::Color::Black);

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

                if (e->hasComponent<CInvincibility>())
                {
                    c = sf::Color(255, 255, 255, 128);
                    animation.getSprite().setColor(c);
                }
                m_game->window().draw(animation.getSprite());
            }
        }

        for (auto e : m_entityManager.getEntities())
        {
            // the health bar for the player
            auto& transform = e->getComponent<CTransform>();
            if (e->tag() == "player" && e->hasComponent<CHealth>())
            {
                auto &h = e->getComponent<CHealth>();
                Vec2 size(320, 50);
                sf::RectangleShape rect({size.x, size.y});
                // on the top of head: 
                // rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
                rect.setPosition(m_game->window().getView().getCenter().x - width() / 2 + 10, m_game->window().getView().getCenter().y - height() / 2 + 10);
                rect.setFillColor(sf::Color(96, 96, 96));
                rect.setOutlineColor(sf::Color::Black);
                rect.setOutlineThickness(2);
                m_game->window().draw(rect);

                float ratio = (float)(h.current) / h.max;
                size.x *= ratio;
                rect.setSize({size.x, size.y});
                rect.setFillColor(sf::Color(255, 0, 0));
                rect.setOutlineThickness(0);
                m_game->window().draw(rect);

                for (int i = 0; i < h.max; i++)
                {
                    tick.setPosition(rect.getPosition() + sf::Vector2f(i * 64 * 1 / h.max, 0));
                    m_game->window().draw(tick);
                }
            }
            // the health bar for the npc
            if (e->tag()=="npc" && e->hasComponent<CHealth>())
            {
                auto& h = e->getComponent<CHealth>();
                Vec2 size(64, 6);
                sf::RectangleShape rect({ size.x, size.y });
                rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
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

                for (int i = 0; i < h.max; i++)
                {
                    tick.setPosition(rect.getPosition() + sf::Vector2f(i * 64 * 1 / h.max, 0));
                    m_game->window().draw(tick);
                }
            }
        }
    }
    // draw the current weapon
    drawWeapon();

    sf::View miniView(sf::FloatRect(0, 0, 1280, 768));
    miniView.setViewport(sf::FloatRect(0.8, 0, 0.2, 0.2));
    m_game->window().setView(miniView);

    sf::RectangleShape line(sf::Vector2f(1280, 5));
    line.setPosition(0, 300);
    m_game->window().draw(line);

    //gridX* m_gridSize.x) + size.x / 2,
    for (auto e : m_entityManager.getEntities())
    {
        auto transform = e->getComponent<CTransform>();

        if (e->hasComponent<CAnimation>() && e->tag() == "player")
        {
            transform.pos.y = 680;
            Animation animation = m_game->assets().getAnimation("Stand");
            animation.getSprite().setRotation(transform.angle);
            auto x = (animation.getSize().x / 2) + (15 * (transform.pos.x - (animation.getSize().x / 2)) / 64);
            animation.getSprite().setPosition(x, (transform.pos.y / 2) - 70);
            animation.getSprite().setScale(transform.scale.x, transform.scale.y);
            m_game->window().draw(animation.getSprite());
        }
        if (e->hasComponent<CAnimation>() && (e->getComponent<CAnimation>().animation.getName() == "Pole"
            || e->getComponent<CAnimation>().animation.getName() == "PoleTop"))
        {
            auto& animation = e->getComponent<CAnimation>().animation;
            animation.getSprite().setRotation(transform.angle);
            auto x = (animation.getSize().x / 2) + (15 * (transform.pos.x - (animation.getSize().x / 2)) / 64);
            animation.getSprite().setPosition(x - 20, (transform.pos.y / 2) - 34);
            animation.getSprite().setScale(transform.scale.x, transform.scale.y);
            m_game->window().draw(animation.getSprite());
        }
    }

    auto& pPos = m_player->getComponent<CTransform>().pos;
    float viewCenterX = m_game->window().getView().getCenter().x;
    float viewCenterY = m_game->window().getView().getCenter().y;
    float windowX = m_game->window().getSize().x;
    float windowY = m_game->window().getSize().y;
    m_prevCameraPos = Vec2(viewCenterX - windowX / 2.0f, viewCenterY - windowY / 2.0f);
    sf::View gameView(sf::FloatRect(0, 0, windowX, windowY));
    float windowCenterX = std::max(windowX / 2.0f, pPos.x);
    float dist = windowY - gameView.getCenter().y;
    float windowCenterY = pPos.y < dist ? pPos.y : dist;
    gameView.setCenter(windowCenterX, windowCenterY);
    gameView.setViewport(sf::FloatRect(0, 0, 1, 1));
    m_game->window().setView(gameView);

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
        sf::Sprite sprite = getLightingSprite();
        m_game->window().draw(sprite);
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

    if (m_inventory)
    {
        for (auto e : m_entityManager.getEntities("inventory"))
        {
            auto& transform = e->getComponent<CTransform>();
            transform.pos = Vec2(m_game->window().getView().getCenter().x - width() / 2, m_game->window().getView().getCenter().y - height() / 2 + 70);

            auto& animation = e->getComponent<CAnimation>().animation;
            animation.getSprite().setRotation(transform.angle);
            animation.getSprite().setOrigin(0, 0);
            animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
            animation.getSprite().setScale(transform.scale.x, transform.scale.y);
            m_game->window().draw(animation.getSprite());
        }

        for (auto e : m_entityManager.getEntities("items"))
        {
            auto& transform = e->getComponent<CTransform>();
            //float x = m_game->window().getView().getCenter().x - width() / 2;
            //auto inv_t = Vec2(7 + x, 3);
            //transform.pos = Vec2(inv_t.x + (73 * i), inv_t.y);
            //std::cout << "Render stuff: " << " ";
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
}


// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
