//  msPacController.h
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#ifndef msPacController_h
#define msPacController_h

#include "controller.h"

class MsPacController: public Controller {
    private:
        // Sensor input storage
        double s_PAC_GHOST;
        double s_PAC_PILL;
        double s_PAC_WALL;
        double s_PAC_FRUIT;

        // Record of how many games this controller has played
        int gamesPlayed;

    public:
        // Constructor and constructor-helper functions
        MsPacController();
        MsPacController(const MsPacController& toCopy);
        MsPacController(const int& d, const int& nodeId);
        void growTree(const int& m);
        void fillTree(const int& d);

        // Setters
        void mutate(const int& treeDepthInit);
        void setSensors(const Board& world);
        void setFinalGameScore(const double& parsimonyCoefficient, const MsPacController& oldPac, const int& score);
        void setGamesPlayed(const int& games) { gamesPlayed = games; return; }

        // Getters
        int getGamesPlayed() const { return gamesPlayed; }

        // State evaluator function
        void evaluateState();

        // Move decision driver function
        MoveMap makeMove(const Board& world) const;

        // Getters
        double getPacGhost() const { return s_PAC_GHOST; }
        double getPacPill() const { return s_PAC_PILL; } 
        double getPacWall() const { return s_PAC_WALL; }
        double getPacFruit() const { return s_PAC_FRUIT; }

        // Assignment 2a function
        Coordinate makeRandomMove(const Board& world);
};

#endif