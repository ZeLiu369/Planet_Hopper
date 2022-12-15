///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
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

#include "Scene_Menu.h"
#include "Scene_OptionMenu.h"
#include "Scene_Play.h"
#include "Scene_Editor.h"
#include "Scene_Overworld.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

Scene_Menu::Scene_Menu(GameEngine * gameEngine)
    : Scene(gameEngine)
{
    init();
}

void Scene_Menu::init()
{
    registerAction(sf::Keyboard::W,     "UP");
    registerAction(sf::Keyboard::S,     "DOWN");
    registerAction(sf::Keyboard::D,     "PLAY");
    registerAction(sf::Keyboard::Escape, "QUIT");

    m_title = "PLANET HOPPER";
    m_menuStrings.push_back("Continue");
    m_menuStrings.push_back("New Game");
    m_menuStrings.push_back("Options");
    m_menuStrings.push_back("Level Editor");

    m_levelPaths.push_back("level1.txt");
    m_levelPaths.push_back("level2.txt");
    m_levelPaths.push_back("level3.txt");

    m_menuText.setFont(m_game->assets().getFont("Roboto"));

    m_game->assets().getMusic("MusicTitle").setLoop(true);
    m_game->assets().getMusic("MusicTitle").setVolume(50.f);
    m_game->playMusic("MusicTitle");
}

void Scene_Menu::update()
{
    m_entityManager.update();

}
                                                     
void Scene_Menu::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "UP")
        {
            if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
            else { m_selectedMenuIndex = m_menuStrings.size() - 1; }
        }
        else if (action.name() == "DOWN")
        {
            m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
        }
        else if (action.name() == "PLAY")
        {
            if (m_selectedMenuIndex == 0)
            {
                m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game));
                m_game->assets().getMusic("MusicTitle").stop();
            }
            if (m_selectedMenuIndex == 1)
            {
                m_game->progress = 1;
                m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game));
                m_game->assets().getMusic("MusicTitle").stop();
            }
            if (m_selectedMenuIndex == 2)
            {
                m_game->changeScene("OPTIONMENU", std::make_shared<Scene_OptionMenu>(m_game));
            }
            if (m_selectedMenuIndex == 3)
            {
                m_game->changeScene("EDIT", std::make_shared<Scene_Editor>(m_game));
            }
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}
                                                     
void Scene_Menu::sRender()
{
    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear(sf::Color(150, 200, 255));

    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("images/new/background3.png");
    backgroundTexture.setSmooth(true);
    backgroundTexture.setRepeated(true);

    Vec2 TextureSize(backgroundTexture.getSize().x, backgroundTexture.getSize().y);  //Added to store texture size.
    Vec2 WindowSize(m_game->window().getSize().x, m_game->window().getSize().y);   //Added to store window size.

    float ScaleX = (float)WindowSize.x / TextureSize.x;
    float ScaleY = (float)WindowSize.y / TextureSize.y;     //Calculate scale.

    sf::Sprite background;
    background.setTexture(backgroundTexture);
    background.setScale(ScaleX, ScaleY);      //Set scale. 

    m_game->window().draw(background);
                                                     
    // draw the game title in the top-left of the screen
    m_menuText.setCharacterSize(70);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color::Cyan);
    m_menuText.setOrigin(m_menuText.getLocalBounds().width / 2 + m_menuText.getLocalBounds().left,
        m_menuText.getLocalBounds().height / 2 + m_menuText.getLocalBounds().top);
    m_menuText.setPosition(sf::Vector2f(m_game->window().getSize().x / 2.0f, 60));
    m_game->window().draw(m_menuText);

    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setOrigin(m_menuText.getLocalBounds().width / 2 + m_menuText.getLocalBounds().left,
            m_menuText.getLocalBounds().height / 2 + m_menuText.getLocalBounds().top);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::White : sf::Color(0, 0, 0));
        m_menuText.setPosition(sf::Vector2f(m_game->window().getSize().x / 2.0f, 200 + i * 110));
        m_game->window().draw(m_menuText);
    }

    m_menuText.setOrigin(0, 0);
    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(40);
    m_menuText.setFillColor(sf::Color::White);
    m_menuText.setString("up: w     down: s    play: d    back: esc");
    m_menuText.setPosition(sf::Vector2f(10, 690));
    m_game->window().draw(m_menuText);
}

void Scene_Menu::onEnd()
{
    m_hasEnded = true;
    m_game->quit();
}

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
