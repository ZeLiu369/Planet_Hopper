///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene.cpp
// 
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
                                                     
#include "Scene.h"
#include "GameEngine.h"
                                                     
Scene::Scene()
{

}
                                                     
Scene::Scene(GameEngine * gameEngine)
    : m_game(gameEngine)
{ 
    
}

void Scene::setPaused(bool paused)
{
    m_paused = paused;
}

size_t Scene::width() const
{
    return m_game->window().getSize().x;
}
                                                     
size_t Scene::height() const
{
    return m_game->window().getSize().y;
}

size_t Scene::currentFrame() const
{
    return m_currentFrame;
}
                                                     
bool Scene::hasEnded() const
{
    return m_hasEnded;
}
                                                     
const ActionMap& Scene::getActionMap() const
{
    return m_actionMap;
}
                                                     
void Scene::registerAction(int inputKey, const std::string& actionName)
{
    m_actionMap[inputKey] = actionName;
}

void Scene::doAction(const Action& action)
{
    // ignore null actions
    if (action.name() == "NONE") { return; }

    sDoAction(action);
}

void Scene::simulate(const size_t frames)
{
    for (size_t i = 0; i < frames; i++)
    {
        update();
    }
}

void Scene::drawLine(const Vec2& p1, const Vec2& p2)
{
    sf::Vertex line[] = { sf::Vector2f(p1.x, p1.y), sf::Vector2f(p2.x, p2.y) };
    m_game->window().draw(line, 2, sf::Lines);
}

                                                     
// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
