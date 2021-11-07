//  board.h
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#ifndef board_h
#define board_h

#include "cell.h"

// Function prototypes
std::ostream& operator<<(std::ostream& out, const std::set<Coordinate> setToPrint);
std::ostream& operator<<(std::ostream& out, const Coordinate& c);

class Board {
    private:
        // Phenotype
        Cell** grid;
        int columns;
        int rows;

        // Genotype
        Coordinate pacLocation;
        MoveMap ghostLocations;
        Coordinate fruitLocation;
        std::set<Coordinate> emptySet;
        std::set<Coordinate> wallSet;
        std::set<Coordinate> pillSet;

        // Global world values
        int score;
        int timeRemaining, totalTime;
        int pillsConsumed, totalPills;
        int fruitConsumed, fruitValue;

    public:
        Board();
        Board(const Board& b);
        Board(const Board& b, std::set<Coordinate> childBulbSet);
        ~Board();

        // Initializer
        Board(int columns, int rows, float pillDensity, float wallDensity, int timeMultiplier, int fScore);

        // Setters
        void setDimensions(int x, int y);
        void setWall(Coordinate c);
        void adjustScore();
        void decreaseTime();
        void spawnFruit();
        void updateMsPacLocation(const Coordinate& newLoc);
        void updateMsPacLocation(const MoveMap& newLoc);
        void updateGhostLocations(const MoveMap& newLocs);

        // Boolean functions
        bool noFruit() const;
        bool ghostGameOver(const Coordinate& oldMsPac, const MoveMap& oldGhosts) const;
        bool otherGameOver();
        
        // Getters
        int getColumns() const { return columns; }
        int getRows() const { return rows; }
        int getfruitValue() const { return fruitValue; }
        int getFruitConsumed() const { return fruitConsumed; }
        int getTimeRemaining() const { return timeRemaining; }
        int getTotalTime() const { return totalTime; }
        int getScore() const { return score; }
        int getPillsConsumed() const { return pillsConsumed; }
        int getTotalPills() const { return totalPills; }
        Coordinate getPacLocation() const { return pacLocation; }
        MoveMap getGhostLocations() const { return ghostLocations; }
        std::set<Coordinate> getWallSet() const { return wallSet; }
        std::set<Coordinate> getPillSet() const { return pillSet; }
        Coordinate getFruitLocation() const { return fruitLocation; }
        std::set<Coordinate> getEmptySet() const { return emptySet; }

        // Sensory functions
        PacSensors getPacSensors() const;
        GhostSensors getGhostSensors() const;

        // Adjacent cell calculations
        std::stack<Coordinate> getNeighbors(const Coordinate& c) const;
        std::queue<Coordinate> senseNeighbors(const Coordinate& c) const;
        int senseWalls(const Coordinate& c) const;
        std::set<Coordinate> getMsPacMoves() const;
        std::map<int, std::set<Coordinate>> getGhostMoves() const; 

        // Overloaded operators
        const Board& operator=(const Board& rhs);
        friend std::ostream& operator<<(std::ostream& out, const Board& b);
};

#endif