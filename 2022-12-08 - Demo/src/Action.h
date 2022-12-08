///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Action.h
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
#include "Entity.h"

class Action
{
    std::string m_name  = "NONE"; 
    std::string m_type  = "NONE"; 
    Vec2        m_pos = Vec2(0, 0);
    
public:

    Action();
    Action(const std::string & name, const std::string & type);
    Action(const std::string& name, const std::string& type, const Vec2& pos);

    const std::string & name() const;
    const std::string & type() const;
    const Vec2& pos() const;
    std::string toString() const;
};

                                                     
// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
