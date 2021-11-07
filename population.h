//  population.h
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#ifndef population_h
#define population_h

#include "msPacController.h"
#include "ghostController.h"

// Function prototypes
void outputParameters(std::ostream& out, std::vector<std::string> parseVector, time_t seedValue);
std::string genString(const Board& world, const bool& fruitSpawned);

// Defining the population of controllers
class Population {
    private:
        // General population of Ms. Pac-Man controllers
        std::multiset<MsPacController, ControllerCompare> pacPopulation;

        // Mating population of Ms. Pac-Man controllers
        std::multiset<MsPacController, ControllerCompare> pacMatingPool;

        // General population of Ghost controllers
        std::multiset<GhostController, ControllerCompare> ghostPopulation;

        // Mating population of ghost controllers
        std::multiset<GhostController, ControllerCompare> ghostMatingPool;

        // Ms. Pac-Man population fitness values
        int cumulativeFitness, bestFitness;
        float avgFitness;

        // Ghost population fitness values
        int gCumulativeFitness, gBestFitness;
        float gAvgFitness;

    public:
        // Default constructor
        Population();

        // Constructor uses the ramped half-and-half method for initialization
        Population(const int& pacSize, const int& ghostSize, const int& treeDepth);

        // Getters
        std::multiset<MsPacController, ControllerCompare> getPacPopulation() const { return pacPopulation; }
        std::multiset<GhostController, ControllerCompare> getGhostPopulation() const { return ghostPopulation; }
        int getPacPopSize() const { return pacPopulation.size(); }
        int getCumulativeFitness() const { return cumulativeFitness; }
        int getBestFitness() const { return bestFitness; }
        float getAvgFitness() const { return avgFitness; }

        // Setters
        void insert(const MsPacController& controller);
        void insert(const GhostController& controller);

        // Fitness evaluation
        bool runGames(int& evalCount, int& highestScore, int& highestGhostScore, const double& pacP, 
                      const double& ghostP, std::ostream& logOut, std::vector<std::string>& bestGameArchive, 
                      Tree& bestTree, Tree& bestGhostTree, const std::vector<std::string>& parseVector);

        // GP-specific functions
        void selectParents(const std::string& pacStrategy, const std::string& ghostStrategy);
        Population mate(const int& offspringSize, const double& mutateRate, const double& ghostMutateRate, const int& treeDepthInit);
        void selectSurvivors(const Population& offspringPop, const int& pacK, const int& pacPopSize, const std::string& pacSurvivalStrat,
                                const int& ghostK, const int& ghostPopSize, const std::string& ghostSurvivalStrat);
        void incrementPacFitness(const int& incrValue) { cumulativeFitness += incrValue; return; }
        void incrementGhostFitness(const int& incrValue) { gCumulativeFitness += incrValue; return; }
        void decrementPacFitness(const int& decrValue) { cumulativeFitness -= decrValue; return; }
        void decrementGhostFitness(const int& decrValue) { gCumulativeFitness -= decrValue; return; }
    

        // Debugging functions
        void print() const;
        void printGhostTree() const;
        
};

#endif