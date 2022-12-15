///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       GameEngine.h
// 
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): [enter student name(s)]
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
                                                     
#pragma once

#include "Common.h"
#include "Scene.h"
#include "Assets.h"
#include "Scene_OptionMenu.h"

#include <memory>

typedef std::map<std::string, std::shared_ptr<Scene>> SceneMap;
                                                     
class GameEngine
{

    struct controls
    {
        sf::Keyboard::Key right = sf::Keyboard::D, left = sf::Keyboard::L, up = sf::Keyboard::W, gravity = sf::Keyboard::S,
                          shoot = sf::Keyboard::Space, inventory = sf::Keyboard::I;
    };
                                                     
protected:

    sf::RenderWindow    m_window;
    Assets              m_assets;
    size_t              m_simulationSpeed = 1;
    bool                m_running = true;

    void init(const std::string & path);
    void update();
                                                     
    void sUserInput();
    std::shared_ptr<Scene> currentScene();

public:

    int   progress = 1;

    float musicVol = 20.0f;
    float soundVol = 80.0f;

    std::string diff;
    float bulletScaler = 1.0f;
    float takenScaler = 1.0f;

    std::string shootKey = "SPACE";

    sf::Event event;

    std::string m_currentScene;
    SceneMap m_sceneMap;

    std::string     optionMenu_return_scene = "MENU";

    controls gameControls;
    
    GameEngine(const std::string & path);

    void changeScene(const std::string & sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene = false);
    std::shared_ptr<Scene> getScene(const std::string &sceneName);
    bool hasScene(const std::string sceneName);

    void quit();
    void run();

    void playSound(const std::string &soundName);
    void playMusic(const std::string &musicName);

    std::string getDiff();
    void setDiff(std::string d);

    void setShootKey(std::string s);
    std::string getShootKey();

    sf::RenderWindow &window();
    Assets& assets();
    bool isRunning();
};

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
