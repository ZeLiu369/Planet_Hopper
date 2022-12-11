///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Assets.h
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
#include "Animation.h"
                                                     
class Assets
{
    std::map<std::string, sf::Texture>      m_textureMap;
    std::map<std::string, Animation>        m_animationMap;
    std::map<std::string, sf::Font>         m_fontMap;
    std::map<std::string, sf::SoundBuffer>  m_soundBufferMap;
    std::map<std::string, sf::Sound>        m_soundMap;

    void addTexture(const std::string & textureName, const std::string & path, bool smooth = true);
    void addAnimation(const std::string & animationName, const std::string & textureName, size_t frameCount, size_t speed);
    void addFont(const std::string & fontName, const std::string & path);
    void addSound(const std::string &fontName, const std::string &path);

public:

    Assets();
                                                     
    void loadFromFile(const std::string & path);

    const sf::Texture & getTexture(const std::string & textureName) const;
    const Animation &   getAnimation(const std::string & animationName) const;
    const sf::Font &    getFont(const std::string & fontName) const;
    sf::Sound&          getSound(const std::string& soundName);
    void                changeVolume(const float& vol);
};

// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
