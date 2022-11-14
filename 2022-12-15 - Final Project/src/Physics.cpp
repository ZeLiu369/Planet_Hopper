///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/
//
//  Assignment       COMP4300 - Assignment 3
//  Professor:       David Churchill
//  Year / Term:     2022-09
//  File Name:       Physics.cpp
// 
//  Student Name:    Nathan French
//  Student User:    ncfrench
//  Student Email:   ncfrench@mun.ca
//  Student ID:      201943859
//  Group Member(s): Nathan
//
///\/\/\\\/\\\\\////\///\\//\\/\//\/\/\\\/\\\\\////\///\\//\\/\/

#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    
    if (!a->hasComponent<CBoundingBox>() || !b->hasComponent<CBoundingBox>()) return Vec2(0, 0);

    CTransform& atrans = a->getComponent<CTransform>();
    CTransform& btrans = b->getComponent<CTransform>();
    CBoundingBox& abox = a->getComponent<CBoundingBox>();
    CBoundingBox& bbox = b->getComponent<CBoundingBox>();

    Vec2 delta = Vec2( abs(atrans.pos.x - btrans.pos.x), abs(atrans.pos.y - btrans.pos.y) );
    Vec2 overlap = abox.halfSize + bbox.halfSize - delta;

    return overlap;
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
             
    if (!a->hasComponent<CBoundingBox>() || !b->hasComponent<CBoundingBox>()) return Vec2(0, 0);

    CTransform& atrans = a->getComponent<CTransform>();
    CTransform& btrans = b->getComponent<CTransform>();
    CBoundingBox& abox = a->getComponent<CBoundingBox>();
    CBoundingBox& bbox = b->getComponent<CBoundingBox>();

    Vec2 delta = Vec2(abs(atrans.prevPos.x - btrans.prevPos.x), abs(atrans.prevPos.y - btrans.prevPos.y));
    Vec2 overlap = abox.halfSize + bbox.halfSize - delta;

    return overlap;
}


// Copyright (C) David Churchill - All Rights Reserved
// COMP4300 - 2022-09 - Assignment 3
// Written by David Churchill (dave.churchill@gmail.com)
// Unauthorized copying of these files are strictly prohibited
// Distributed only for course work at Memorial University
// If you see this file online please contact email above
