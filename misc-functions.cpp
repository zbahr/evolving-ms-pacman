//  misc-functions.cpp
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#include "board.h"

bool isValid(Board temp, Coordinate startLoc) {
    std::stack<Coordinate> neighbors;
    std::stack<Coordinate> newNeighbors;
    neighbors = temp.getNeighbors(startLoc);

    while (!neighbors.empty()) {
        temp.setWall(neighbors.top());
        newNeighbors = temp.getNeighbors(neighbors.top());
        neighbors.pop();

        while (!newNeighbors.empty()) {
            neighbors.push(newNeighbors.top());
            newNeighbors.pop();
        }
    }

    if (temp.getWallSet().size() == temp.getColumns() * temp.getRows() - 2) {
        return true;
    }

    return false;
}

std::string genString(const Board& world, const bool& fruitSpawned) {
    std::stringstream ss;

    // If this is the first state of the world, output columns, rows, pills, and walls, plus locations of pieces
    if (world.getTimeRemaining() == world.getTotalTime()) {
        ss << world.getColumns() << "\n" << world.getRows() << "\nm " << world.getPacLocation().first << " " << world.getPacLocation().second << "\n";
            for (int i = 1; i <= NUM_GHOSTS; i++) {
                ss << i << " " << world.getGhostLocations().at(i).first << " " << world.getGhostLocations().at(i).second << "\n";
            }
            std::set<Coordinate> walls = world.getWallSet();
            for (auto it = walls.begin(); it != walls.end(); it++) {
                ss << "w " << it->first << " " << it->second << "\n";
            }
            std::set<Coordinate> pills = world.getPillSet();
            for (auto it = pills.begin(); it != pills.end(); it++) {
                ss << "p " << it->first << " " << it->second << "\n";
            }
            ss << "t " << world.getTimeRemaining() << " " << world.getScore() << "\n";
    }
    else {
        ss << "m " << world.getPacLocation().first << " " << world.getPacLocation().second << "\n";
        for (int i = 1; i <= NUM_GHOSTS; i++) {
            ss << i << " " << world.getGhostLocations().at(i).first << " " << world.getGhostLocations().at(i).second << "\n";
        }
        if (fruitSpawned && world.getFruitLocation().first == -1 && world.getFruitLocation().second == -1) { 
            ss << "f " << world.getPacLocation().first << " " << world.getPacLocation().second << "\n"; 
        }
        else if (fruitSpawned) { 
            ss << "f " << world.getFruitLocation().first << " " << world.getFruitLocation().second << "\n";
        }
        ss << "t " << world.getTimeRemaining() << " " << world.getScore() << "\n";
    }


    return ss.str();
}


void outputParameters(std::ostream& out, std::vector<std::string> parseVector, time_t seedValue) {
    out << "*** Config Parameters ***" << std::endl;
    out << "Columns: " << parseVector.at(COLUMNS_INDEX) << std::endl;
    out << "Rows: " << parseVector.at(ROWS_INDEX) << std::endl;
    out << "Pill Density: " << parseVector.at(PILL_DENSITY_INDEX) << std::endl;
    out << "Wall Density: " << parseVector.at(WALL_DENSITY_INDEX) << std::endl;
    out << "Fruit Spawn Probability: " << parseVector.at(FRUIT_SPAWN_INDEX) << std::endl;
    out << "Fruit Score: " << parseVector.at(FRUIT_SCORE_INDEX) << std::endl;
    out << "Time Multiplier: " << parseVector.at(TIME_MULT_INDEX) << std::endl;
    out <<  "Seed: " << parseVector.at(SEED_INDEX) << std::endl;
    if (parseVector.at(SEED_INDEX) == "time") { out << "Seed Value: " << seedValue << std::endl; }
    out << "Runs: " << parseVector.at(RUNS_INDEX) << std::endl;
    out << "Fitness Evaluations: " << parseVector.at(EVALS_INDEX) << std::endl;
    out << "Ms. Pac-Man Population Size: " << parseVector.at(PAC_POP_INDEX) << std::endl;
    out << "Ms. Pac-Man Offspring Size: " << parseVector.at(PAC_OFFSPRING_INDEX) << std::endl;
    out << "Ms. Pac-Man Mutation Rate: " << parseVector.at(PAC_MUTATE_INDEX) << std::endl;
    out << "Ms. Pac-Man k (tournament selection): " << parseVector.at(PAC_K_INDEX) << std::endl;
    out << "Ms. Pac-Man p (parsimony coefficient): " << parseVector.at(PAC_P_INDEX) << std::endl;
    out << "Ms. Pac-Man Parent Selection Strategy: " << parseVector.at(PAC_PARENT_STRAT_INDEX) << std::endl;
    out << "Ms. Pac-Man Survival Selection Strategy: " << parseVector.at(PAC_SURVIVAL_STRAT_INDEX) << std::endl;
    out << "Ghosts Population Size: " << parseVector.at(GHOSTS_POP_INDEX) << std::endl;
    out << "Ghosts Offspring Size: " << parseVector.at(GHOSTS_OFFSPRING_INDEX) << std::endl;
    out << "Ghosts Mutation Rate: " << parseVector.at(GHOSTS_MUTATE_INDEX) << std::endl;
    out << "Ghosts k (tournament selection): " << parseVector.at(GHOSTS_K_INDEX) << std::endl;
    out << "Ghosts p (parsimony coefficient): " << parseVector.at(GHOSTS_P_INDEX) << std::endl;
    out << "Ghosts Parent Selection Strategy: " << parseVector.at(GHOSTS_PARENT_STRAT_INDEX) << std::endl;
    out << "Ghosts Survival Selection Strategy: " << parseVector.at(GHOSTS_SURVIVAL_STRAT_INDEX) << std::endl;
    out << "Max Tree Initialization Depth: " << parseVector.at(TREE_INIT_DEPTH_INDEX) << std::endl;
    out << "Termination Strategy: " << parseVector.at(TERMINATION_STRAT_INDEX) << std::endl;
    out << "Termination Criterion: " << parseVector.at(TERM_CRIT_INDEX) << std::endl;
    out << "Log File: " << parseVector.at(LOG_INDEX) << std::endl;
    out << "World File: " << parseVector.at(WORLD_INDEX) << std::endl;
    out << "Ms. Pac-Man Controller Tree File: " << parseVector.at(PAC_CONTROLLER_INDEX) << std::endl;
    out << "Ghosts Controller Tree File: " << parseVector.at(GHOSTS_CONTROLLER_INDEX) << std::endl;

    return;
}