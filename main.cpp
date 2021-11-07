//  main.cpp
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#include "population.h"

int main(int argc, const char * argv[]) {
    std::ifstream fin;
    std::ofstream logOut;
    std::ofstream solutionOut;
    std::ofstream pacControllerOut;
    std::ofstream ghostControllerOut;
    std::stringstream ss;

    // Container for world states of game with highest score across all runs 
    Tree bestTree, bestGhostTree;
    std::vector<std::string> bestGameArchive;
    int highestScore = 0;
    int highestGhostScore = INT_MIN;

    // Get the config file
    if (argc > 1) {
        std::string filename = argv[1];
        fin.open(filename);
        if (!fin.is_open()) { std::cout << "Invalid config file." << std::endl; exit(1); }
    }
    else { fin.open("./default.cfg"); }

    ///////////////////////////////////////// CONFIG FILE PARSING ////////////////////////////////////////////
    std::vector<std::string> parseVector;
    std::string seed, pacParentStrat, pacSurvStrat, ghostParentStrat, ghostSurvStrat;
    std::string termStrategy, logFile, worldFile, pacControlFile, ghostControlFile;
    int runs, evals, pacPopSize, pacOffspringSize, ghostPopSize, ghostOffspringSize;
    int pacK, ghostK, treeDepthInit, termCriterion;
    double pacP, pacMutateRate, ghostP, ghostMutateRate;
    time_t seedValue;

    while (!fin.eof()) {
        std::string type;
        std::string value;
        
        std::getline(fin, type, ':');
        fin >> value;

        parseVector.push_back(value);
    }
    fin.close();

    seed = parseVector.at(SEED_INDEX);
    runs = std::stoi(parseVector.at(RUNS_INDEX));
    evals = std::stoi(parseVector.at(EVALS_INDEX));
    pacPopSize = std::stoi(parseVector.at(PAC_POP_INDEX));
    pacOffspringSize = std::stoi(parseVector.at(PAC_OFFSPRING_INDEX));
    pacMutateRate = std::stod(parseVector.at(PAC_MUTATE_INDEX));
    pacK = std::stoi(parseVector.at(PAC_K_INDEX));
    pacP = std::stod(parseVector.at(PAC_P_INDEX));
    pacParentStrat = parseVector.at(PAC_PARENT_STRAT_INDEX);
    pacSurvStrat = parseVector.at(PAC_SURVIVAL_STRAT_INDEX);
    ghostPopSize = std::stoi(parseVector.at(GHOSTS_POP_INDEX));
    ghostOffspringSize = std::stoi(parseVector.at(GHOSTS_OFFSPRING_INDEX));
    ghostMutateRate = std::stod(parseVector.at(GHOSTS_MUTATE_INDEX));
    ghostK = std::stoi(parseVector.at(GHOSTS_K_INDEX));
    ghostP = std::stod(parseVector.at(GHOSTS_P_INDEX));
    ghostParentStrat = parseVector.at(GHOSTS_PARENT_STRAT_INDEX);
    ghostSurvStrat = parseVector.at(GHOSTS_SURVIVAL_STRAT_INDEX);
    treeDepthInit = std::stoi(parseVector.at(TREE_INIT_DEPTH_INDEX));
    termStrategy = parseVector.at(TERMINATION_STRAT_INDEX);
    termCriterion = std::stoi(parseVector.at(TERM_CRIT_INDEX));
    logFile = parseVector.at(LOG_INDEX);
    worldFile = parseVector.at(WORLD_INDEX);
    pacControlFile = parseVector.at(PAC_CONTROLLER_INDEX);
    ghostControlFile = parseVector.at(GHOSTS_CONTROLLER_INDEX);

    // Establish seed for random number generation
    if (seed == "time") { seedValue = time(NULL); srand(seedValue); }
    else { srand(std::stoi(seed)); }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Create log file
    logOut.open(logFile);
    outputParameters(logOut, parseVector, seedValue);
    logOut << std::endl << "Result Log" << std::endl;

    // Run experiments
    for (int i = 1; i <= runs; i++) {
        // Output progress to standard output
        std::cout << "Running Experiment " << i << " with " << evals << " games!" << std::endl;

        // Log file details
        logOut << std::endl << "Run " << i << std::endl;
        int highestRunScore = 0;
        int evalCount = 0;

        // Instantiate population of Ms. Pac-Man controllers and population of Ghost controllers
        Population pop(pacPopSize, ghostPopSize, treeDepthInit);

        /* For all the controllers in the population, run a full game to get fitness
           and update population so that it is ordered by these fitness values */
        pop.runGames(evalCount, highestScore, highestGhostScore, pacP, ghostP, logOut, bestGameArchive, bestTree, bestGhostTree, parseVector);
        int sameFitnessCounter = 1;  
        int localBestFitness = pop.getBestFitness();

        // Log output for initial population
        logOut << evalCount << "\t" << pop.getAvgFitness() << "\t" << pop.getBestFitness() << std::endl;

        // Evolve the controllers over the course of evals number of evaluations
        while (evalCount <= evals) {
            Population offspringPop;

            // Select the parents to mate from the general population
            pop.selectParents(pacParentStrat, ghostParentStrat);

            // Mate the parents in the mating pool and produce offspringSize number of offspring
            offspringPop = pop.mate(pacOffspringSize, pacMutateRate, ghostMutateRate, treeDepthInit);

            // Run fitness evaluation on offspring (if condition returns false when eval count is exceeded)
            if (offspringPop.runGames(evalCount, highestScore, highestGhostScore, pacP, ghostP, logOut, bestGameArchive, bestTree, bestGhostTree, parseVector)) {
                // Select the survivors using the comma strategy
                pop.selectSurvivors(offspringPop, pacK, pacPopSize, pacSurvStrat, ghostK, ghostPopSize, ghostSurvStrat);
                logOut << evalCount << "\t" << pop.getAvgFitness() << "\t" << pop.getBestFitness() << std::endl;
            }

            // If termination criterion specified . . .
            if (termStrategy == "no-change") {
                if (pop.getBestFitness() == localBestFitness) { sameFitnessCounter++; }
                else { localBestFitness = pop.getBestFitness(); sameFitnessCounter = 1; }

                if (sameFitnessCounter == termCriterion) { 
                    std::cout << "No change in best fitness for " << termCriterion << " generations!" << std::endl;
                    break; 
                }
            }

            // Output the best game archive to file
            solutionOut.open(worldFile);
            for (auto it = bestGameArchive.begin(); it != bestGameArchive.end(); it++) {
                solutionOut << *it; 
            }
            solutionOut.close();

            // Output the best controller tree to file
            pacControllerOut.open(pacControlFile);
            ghostControllerOut.open(ghostControlFile);
            boost::write_graphviz(pacControllerOut, bestTree, boost::make_label_writer(boost::get(&Node::debugText, bestTree)));
            boost::write_graphviz(ghostControllerOut, bestGhostTree, boost::make_label_writer(boost::get(&Node::debugText, bestGhostTree)));
            pacControllerOut.close();
            ghostControllerOut.close();
        }
    }

    return 0;
}