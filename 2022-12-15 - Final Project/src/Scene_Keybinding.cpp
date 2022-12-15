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
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Right, "INCREASE10");
    registerAction(sf::Keyboard::Left, "DECREASE10");
    registerAction(sf::Keyboard::Up, "INCREASE");
    registerAction(sf::Keyboard::Down, "DECREASE");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::Enter, "Enter");






    m_title = "Key Binding";
    m_menuStrings.push_back("THIS IS WORKING ");
    m_menuText.setFont(m_game->assets().getFont("ChunkFive"));
    m_menuText.setCharacterSize(64);

    int num = 1;
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
              
            }

            if (m_selectedMenuIndex == 1)
            {
                
            }

            if (m_selectedMenuIndex == 2)
            {
                if (diff1 == "EASY")
                {
                    diff1 = "NORMAL";
                }
                else if (diff1 == "NORMAL")
                {
                    diff1 = "HARD";
                }
                else if (diff1 == "HARD")
                {
                    diff1 = "EASY";
                }
            }
        }
        else if (action.name() == "DECREASE")
        {

            if (m_selectedMenuIndex == 0)
            {
            
            }

            if (m_selectedMenuIndex == 1)
            {
                
            }
            // decrease the difficulty
            if (m_selectedMenuIndex == 2)
            {
                if (diff1 == "EASY")
                {
                    diff1 = "HARD";
                }
                else if (diff1 == "NORMAL")
                {
                    diff1 = "EASY";
                }
                else if (diff1 == "HARD")
                {
                    diff1 = "NORMAL";
                }
            }
        }
        else if (action.name() == "INCREASE10")
        {

            if (m_selectedMenuIndex == 0)
            {
               
            }
          
            if (m_selectedMenuIndex == 1)
            {
               
            }
        }
        else if (action.name() == "DECREASE10")
        {

            if (m_selectedMenuIndex == 0)
            {
            
            }

            if (m_selectedMenuIndex == 1)
            {
                
            }
        }
        else if (action.name() == "Enter")
        {   
            // confirm the difficulty
            if (m_selectedMenuIndex == 2)
            {
                // m_game->setDiff(diff1);

                // display the confirmation text (prompt)
                confirmText.setString("Difficulty set to " + diff1 + "!");
                confirmText.setCharacterSize(20);
                confirmText.setFillColor(sf::Color::Red);
                confirmText.setFont(m_game->assets().getFont("ChunkFive"));
                confirmText.setPosition(sf::Vector2f(500, 10));
            }
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

    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuStrings[0] = "Music Volume: " + std::to_string(int(music_volume)) + "/100";
        m_menuStrings[1] = "Sounds Effect Volume: " + std::to_string(int(sounds_volume)) + "/100";
        m_menuStrings[2] = "Difficulty: " + diff1;
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::White : sf::Color(0, 0, 0));
        m_menuText.setPosition(sf::Vector2f(10, 110 + i * 72));
        m_game->window().draw(m_menuText);
    }

    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(20);
    m_menuText.setFillColor(sf::Color::Black);
    m_menuText.setString("up: w  down: s adjust: arrows back: esc  enter: confirm");
    m_menuText.setPosition(sf::Vector2f(10, 690));

    // disapper the confirmation text after 2 seconds
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

    // change the scene back to the sceneMenu
    // m_game->changeScene(prev_scene, nullptr, true);
    // m_game->getScene(m_game->m_currentScene)->setOptionMenu(false);
    // m_game->getScene(m_game->m_currentScene)->setPaused(false);
    m_game->changeScene("OPTIONMENU", std::make_shared<Scene_OptionMenu>(m_game));
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
