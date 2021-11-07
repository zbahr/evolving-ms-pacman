//  cell.h
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#ifndef cell_h
#define cell_h

#include "header.h"

struct Cell {

    bool isPac;
    bool isGhost;
    bool isWall;
    bool isPill;
    bool isFruit;

    int ghostCount = 0;

    Cell() {
        isPac = isGhost = isWall = isPill = isFruit = false;
    }
};

#endif
