///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Scene_Menu.cpp
//
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
#include "Scene_Keybinding.h"
#include "Scene_OptionMenu.h"
#include "Scene_Play.h"
#include "Scene_Editor.h"
#include "Scene_Overworld.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

Scene_Keybinding::Scene_Keybinding(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
}

void Scene_Keybinding::init()
{
    
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");

    registerAction(sf::Keyboard::D, "INCREASE");
    registerAction(sf::Keyboard::Right, "INCREASE");
    registerAction(sf::Keyboard::A, "DECREASE");
    registerAction(sf::Keyboard::Left, "DECREASE");

    registerAction(sf::Keyboard::Enter, "CONFIRM");
    registerAction(sf::Keyboard::Escape, "QUIT");

    int num = 1;
    shootKey1 = m_game->getShootKey();
    moveKey1 = m_game->getMoveKey();

    m_title = "Key Binding";
    m_menuText.setFont(m_game->assets().getFont("ChunkFive"));
    m_menuText.setCharacterSize(64);

    m_menuStrings.push_back("Movement: ");
    m_menuStrings.push_back("Shoot: ");

    clock.restart();
}

void Scene_Keybinding::update()
{
    m_entityManager.update();
}

void Scene_Keybinding::sDoAction(const Action &action)
{
    if (action.type() == "START")
    {
        if (action.name() == "UP")
        {
            if (m_selectedMenuIndex > 0)
            {
                m_selectedMenuIndex--;
            }
            else
            {
                m_selectedMenuIndex = m_menuStrings.size() - 1;
            }
        }
        else if (action.name() == "DOWN")
        {
            m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
        }
        else if (action.name() == "INCREASE")
        {
            if (m_selectedMenuIndex == 0)
            {
                if (moveKey1 == "WASD")
                {
                    moveKey1 = "ARROW";
                }
                else if (moveKey1 == "ARROW")
                {
                    moveKey1 = "WASD";
                }
            }

            if (m_selectedMenuIndex == 1)
            {
                if (shootKey1 == "J")
                {
                    shootKey1 = "K";
                }
                else if (shootKey1 == "SPACE")
                {
                    shootKey1 = "J";
                }
                else if (shootKey1 == "K")
                {
                    shootKey1 = "SPACE";
                }
            }
        }
        else if (action.name() == "DECREASE")
        {
            if (m_selectedMenuIndex == 0)
            {
                if (moveKey1 == "WASD")
                {
                    moveKey1 = "ARROW";
                }
                else if (moveKey1 == "ARROW")
                {
                    moveKey1 = "WASD";
                }
            }
            if (m_selectedMenuIndex == 1)
            {
                if (shootKey1 == "J")
                {
                    shootKey1 = "K";
                }
                else if (shootKey1 == "SPACE")
                {
                    shootKey1 = "J";
                }
                else if (shootKey1 == "K")
                {
                    shootKey1 = "SPACE";
                }
            }
        }
        else if (action.name() == "CONFIRM")
        {   
            m_game->setShootKey(shootKey1);
            m_game->setMoveKey(moveKey1);
            if (m_game -> hasScene("PLAY"))
            {   
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::J);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::K);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::Space);
                m_game->getScene("PLAY") -> registerAction(m_game->gameControls.shoot, "SHOOT");
            }

            if (m_game -> hasScene("PLAY") && moveKey1 == "ARROW")
            {
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::W);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::A);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::S);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::D);
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::Up, "UP");
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::Down, "DOWN");
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::Left, "LEFT");
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::Right, "RIGHT");
            }
            else if (m_game -> hasScene("PLAY") && moveKey1 == "WASD")
            {
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::Up);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::Left);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::Down);
                m_game->getScene("PLAY") -> unregisterAction(sf::Keyboard::Right);
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::W, "UP");
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::A, "LEFT");
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::S, "DOWN");
                m_game->getScene("PLAY") -> registerAction(sf::Keyboard::D, "RIGHT");
            }

             // display the confirmation text (prompt)
            confirmText.setString("Key binding saved!");
            confirmText.setCharacterSize(20);
            confirmText.setFillColor(sf::Color::Red);
            confirmText.setFont(m_game->assets().getFont("ChunkFive"));
            confirmText.setPosition(sf::Vector2f(500, 10));
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}

void Scene_Keybinding::sRender()
{
     sf::Time elapsed1 = clock.getElapsedTime();

    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear(sf::Color(150, 200, 255));

    // draw the game title in the top-left of the screen
    m_menuText.setCharacterSize(48);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color::Black);
    m_menuText.setPosition(sf::Vector2f(10, 10));
    m_game->window().draw(m_menuText);

    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuStrings[0] = "Movement: " + moveKey1;
        m_menuStrings[1] = "Shoot: " + shootKey1;
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::White : sf::Color(0, 0, 0));
        m_menuText.setPosition(sf::Vector2f(10, 110 + i * 72));
        m_game->window().draw(m_menuText);
    }


    m_game->window().draw(m_menuText);


    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(20);
    m_menuText.setFillColor(sf::Color::Black);
    m_menuText.setString("UP: w/up arrow  DOWN: s/down arrow  ADJUST: a/d/left arrow/right arrow,  QUIT: esc  CONFIRM: enter");
    m_menuText.setPosition(sf::Vector2f(10, 690));

    //disapper the confirmation text after 2 seconds
    if (elapsed1.asSeconds() > 2)
    {
        confirmText.setString("");
        clock.restart();
    }
    m_game->window().draw(confirmText);


    m_game->window().draw(m_menuText);
}

void Scene_Keybinding::onEnd()
{
    m_hasEnded = true;
    m_game->changeScene("OPTIONMENU", std::make_shared<Scene_OptionMenu>(m_game));
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
