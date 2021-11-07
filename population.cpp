//  population.cpp
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#include "population.h"

// Default constructor
Population::Population() {
    cumulativeFitness = 0;
    avgFitness = 0;
    bestFitness = 0;
    gCumulativeFitness = 0;
    gAvgFitness = 0;
    gBestFitness = 0;
}

// Ramped half-and-half constructor
Population::Population(const int& pacSize, const int& ghostSize, const int& treeDepth) {
    for (int i = 0; i < pacSize; i++) {
        pacPopulation.insert(MsPacController(treeDepth, 0));
    }
    for (int i = 0; i < ghostSize; i++) {
        ghostPopulation.insert(GhostController(treeDepth, 0));
    }

    cumulativeFitness = 0;
    avgFitness = 0;
    bestFitness = 0;
    gCumulativeFitness = 0;
    gAvgFitness = 0;
    gBestFitness = 0;
}

// Insert Ms. Pac-Man controller into the population
void Population::insert(const MsPacController& controller) {
    pacPopulation.insert(controller);

    return;
}

// Insert ghost controller into the population
void Population::insert(const GhostController& controller) {
    ghostPopulation.insert(controller);

    return;
}

// Runs games for all members of the population
bool Population::runGames(int& evalCount, int& highestScore, int& highestGhostScore, const double& pacP, 
                          const double& ghostP, std::ostream& logOut, std::vector<std::string>& bestGameArchive, 
                          Tree& bestTree, Tree& bestGhostTree, const std::vector<std::string>& parseVector) {
    // Establish world variables
    int columns, rows, fruitValue, timeMultiplier;
    float pillDensity, wallDensity, fruitSpawn;
    columns = std::stoi(parseVector.at(COLUMNS_INDEX));
    rows = std::stoi(parseVector.at(ROWS_INDEX));
    pillDensity = std::stod(parseVector.at(PILL_DENSITY_INDEX));
    wallDensity = std::stod(parseVector.at(WALL_DENSITY_INDEX));
    fruitSpawn = std::stod(parseVector.at(FRUIT_SPAWN_INDEX));
    fruitValue = std::stoi(parseVector.at(FRUIT_SCORE_INDEX));
    timeMultiplier = std::stoi(parseVector.at(TIME_MULT_INDEX));

    // New temporary container for controllers with updated fitness
    std::multiset<MsPacController, ControllerCompare> pacTemp;
    std::multiset<GhostController, ControllerCompare> ghostTemp; 

    // Iterate through all members of the population, run a game with each controller
    while (!pacPopulation.empty() || !ghostPopulation.empty()) {
        evalCount++;

        // If the fitness evaluation limit has been reached, stop
        if (evalCount > std::stoi(parseVector.at(EVALS_INDEX))) { return false; }
        
        // Output progress to standard output
        if (evalCount % PROGRESS_VAR == 0) { std::cout << "Running Game " << evalCount << std::endl; }

        // Initialize the world to play on
        bool gameOver = false;
        bool fruitSpawned = false;
        Board world(columns, rows, pillDensity, wallDensity, timeMultiplier, fruitValue);
        if (world.noFruit() && rand() % 100 < (fruitSpawn * 100)) { 
            world.spawnFruit(); 
            fruitSpawned = true;
        }
        else { fruitSpawned = false; }

        // Get the controllers to compete
        MsPacController msPacMan;
        if (!pacPopulation.empty()) {
            auto it2 = pacPopulation.begin();
            std::advance(it2, rand() % pacPopulation.size());
            msPacMan = *it2;
            msPacMan.setGamesPlayed(0);
            pacPopulation.erase(it2);
        }
        else {
            auto it2 = pacTemp.begin();
            std::advance(it2, rand() % pacTemp.size());
            msPacMan = *it2;
            pacTemp.erase(it2);
        }
 
        GhostController ghosts;
        if (!ghostPopulation.empty()) {
            auto it2 = ghostPopulation.begin();
            std::advance(it2, rand() % ghostPopulation.size());
            ghosts = *it2;
            ghosts.setGamesPlayed(0);
            ghostPopulation.erase(it2);
        }
        else {
            auto it2 = ghostTemp.begin();
            std::advance(it2, rand() % ghostTemp.size());
            ghosts = *it2;
            ghostTemp.erase(it2);
        }

        // Archiving for local run
        std::vector<std::string> localGameArchive;
        localGameArchive.push_back(genString(world, fruitSpawned));

        // Repeatedly make moves until game is over
        while (!gameOver) {
            // Record old locations
            Coordinate oldMsPac = world.getPacLocation();
            MoveMap oldGhosts = world.getGhostLocations();

            // Get ghost to make a move based on their controller
            world.updateGhostLocations(ghosts.makeMove(world));

            // Get Ms. Pac-Man to make a move (see makeMove() function for details)
            world.updateMsPacLocation(msPacMan.makeMove(world));

            // Decrease time
            world.decreaseTime();

            // Check if new state causes game over due to the ghosts
            gameOver = world.ghostGameOver(oldMsPac, oldGhosts);
            if (!gameOver) {
                // Increment the score if game's not over
                world.adjustScore();

                // Check if the game is over due to pills or time limit
                gameOver = world.otherGameOver();
            }

            // Chance for fruit to spawn
            if (world.noFruit() && rand() % 100 < (fruitSpawn * 100)) { 
                world.spawnFruit(); 
                fruitSpawned = true;
            }
            else { fruitSpawned = false; }

            localGameArchive.push_back(genString(world, fruitSpawned));
        }

        // Store the Ms. Pac-Man controller with its new fitness value
        MsPacController tempPacController(msPacMan);
        tempPacController.setGamesPlayed(msPacMan.getGamesPlayed() + 1);
        if (tempPacController.getGamesPlayed() > 1) { decrementPacFitness(msPacMan.getFinalGameScore()); }
        tempPacController.setFinalGameScore(pacP, msPacMan, world.getScore());
        pacTemp.insert(tempPacController);
        incrementPacFitness(tempPacController.getFinalGameScore());

        // If the game had the highest score over all runs, grab the game archive info
        if (tempPacController.getFinalGameScore() >= highestScore) {
            bestGameArchive = localGameArchive;
            highestScore = tempPacController.getFinalGameScore();
            bestTree = msPacMan.getTree();
        }
        
        // Store the ghost controller with its new fitness value
        GhostController tempGhostController(ghosts);
        tempGhostController.setGamesPlayed(ghosts.getGamesPlayed() + 1);
        if (tempGhostController.getGamesPlayed() > 1) { decrementGhostFitness(ghosts.getFinalGameScore()); } 
        tempGhostController.setFinalGameScore(ghostP, ghosts, world.getScore());
        ghostTemp.insert(tempGhostController);
        incrementGhostFitness(tempGhostController.getFinalGameScore());

        if (tempGhostController.getFinalGameScore() >= highestGhostScore) {
            highestGhostScore = tempGhostController.getFinalGameScore();
            bestGhostTree = ghosts.getTree();
        }
    }

    // Update the population 
    pacPopulation = pacTemp;
    ghostPopulation = ghostTemp;

    avgFitness = static_cast<float>(cumulativeFitness) / pacPopulation.size();
    bestFitness = pacPopulation.rbegin()->getFinalGameScore();

    gAvgFitness = static_cast<float>(gCumulativeFitness) / ghostPopulation.size();
    gBestFitness = ghostPopulation.rbegin()->getFinalGameScore();

    return true;
}

// GP-specific function to select parents in the population based on pacStrategy and ghostStrategy
void Population::selectParents(const std::string& pacStrategy, const std::string& ghostStrategy) {
    const int NUM_MATING = 5;

    // Fitness proportional parent selection for Ms. Pac-Man population
    if (pacStrategy == "fitness") {
        // Edge case
        int temp;
        if (cumulativeFitness == 0) { temp = 1; }
        else { temp = cumulativeFitness; }

        // Select the Ms. Pac-Man controller parents
        for (int i = 0; i < NUM_MATING; i++) {
            int random_int = rand() % temp;
            for (auto it = pacPopulation.begin(); it != pacPopulation.end(); it++) {
                random_int -= it->getFinalGameScore();
                if (random_int < 0) { pacMatingPool.insert(*it); break; }
            }
        }
    }
    // Over-selection for Ms. Pac-Man population
    else {
        /*  Split the population into the top 20% and the bottom 80%
            Draw from top 20% 80% of the time, fitness proportionately
            Draw from bottom 80% 20% of the time, fitness proportionately */
        const int dividingIndex = pacPopulation.size() * 0.8;

        // Separate population
        std::multiset<MsPacController, ControllerCompare> topPop;
        int cumulativeTopFitness = 0;
        std::multiset<MsPacController, ControllerCompare> bottomPop;
        int cumulativeBottomFitness = 0;
        int i = 1;
        for (auto it = pacPopulation.begin(); it != pacPopulation.end(); it++) {
            if (i <= dividingIndex) { bottomPop.insert(*it); cumulativeBottomFitness += it->getFinalGameScore(); }
            else { topPop.insert(*it); cumulativeTopFitness += it->getFinalGameScore(); }
            i++;
        }

        // Edge cases
        if (cumulativeBottomFitness == 0) { cumulativeBottomFitness = 1; }
        if (cumulativeTopFitness == 0) { cumulativeTopFitness = 1; }

        // Select the parents fitness proportionately from sub-populations
        for (int i = 0; i < NUM_MATING; i++) {
            if (rand() % 100 > 20) {
                int random_int = rand() % cumulativeTopFitness;
                for (auto it = topPop.begin(); it != topPop.end(); it++) {
                    random_int -= it->getFinalGameScore();
                    if (random_int <= 0) { pacMatingPool.insert(*it); break; }
                }
            }
            else {
                int random_int = rand() % cumulativeBottomFitness;
                for (auto it = bottomPop.begin(); it != bottomPop.end(); it++) {
                    random_int -= it->getFinalGameScore();
                    if (random_int <= 0) { pacMatingPool.insert(*it); break; }
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    // Fitness proportionate selection for ghost population
    if (ghostStrategy == "fitness") {
        // "Normalize" the negative fitness values
        std::vector<int> normalizedFitness;
        int lowestValue = ghostPopulation.begin()->getFinalGameScore() * -1;
        for (auto it = ghostPopulation.begin(); it != ghostPopulation.end(); it++) {
            normalizedFitness.push_back(it->getFinalGameScore() + lowestValue);
        }

        // Select the ghost controller parents
        for (int i = 0; i < NUM_MATING; i++) {
            // Edge case
            int temp;
            if (gCumulativeFitness == 0) { temp = 1; }
            else { temp = gCumulativeFitness; }

            int random_int = rand() % (temp * -1);
            for (int i = 0; i < normalizedFitness.size(); i++) {
                random_int -= normalizedFitness.at(i);
                if (random_int < 0) { 
                    auto it = ghostPopulation.begin();
                    std::advance(it, i);
                    ghostMatingPool.insert(*it); 
                    break; 
                }
            }
        }
    }
    // Overselection for ghost population
    else {
        /*  Split the population into the top 20% and the bottom 80%
            Draw from top 20% 80% of the time, fitness proportionately
            Draw from bottom 80% 20% of the time, fitness proportionately */
        const int dividingIndex = ghostPopulation.size() * 0.8;

        // Separate population
        std::multiset<GhostController, ControllerCompare> topPop;
        int cumulativeTopFitness = 0;
        std::multiset<GhostController, ControllerCompare> bottomPop;
        int cumulativeBottomFitness = 0;
        int i = 1;
        for (auto it = ghostPopulation.begin(); it != ghostPopulation.end(); it++) {
            if (i <= dividingIndex) { bottomPop.insert(*it); cumulativeBottomFitness += it->getFinalGameScore(); }
            else { topPop.insert(*it); cumulativeTopFitness += it->getFinalGameScore(); }
            i++;
        }

        // Select the parents fitness proportionately from sub-populations
        for (int i = 0; i < NUM_MATING; i++) {
            if (rand() % 100 > 20) {
                // Edge case
                if (cumulativeTopFitness == 0) {
                    auto it = topPop.begin();
                    std::advance(it, rand() % topPop.size());
                    ghostMatingPool.insert(*it);
                }
                else {
                    // "Normalize" the negative fitness values
                    std::vector<int> normalizedFitness;
                    int lowestValue = topPop.begin()->getFinalGameScore() * -1;
                    for (auto it = topPop.begin(); it != topPop.end(); it++) {
                        normalizedFitness.push_back(it->getFinalGameScore() + lowestValue);
                    }

                    int random_int = rand() % (cumulativeTopFitness * -1);
                    for (int i = 0; i < normalizedFitness.size(); i++) {
                        random_int -= normalizedFitness.at(i);
                        if (random_int <= 0) { 
                            auto it = topPop.begin();
                            std::advance(it, i);
                            ghostMatingPool.insert(*it); 
                            break; 
                        }
                    }
                }
            }
            else {
                // Edge case
                if (cumulativeBottomFitness == 0) {
                    auto it = bottomPop.begin();
                    std::advance(it, rand() % bottomPop.size());
                    ghostMatingPool.insert(*it);
                    break;
                }
                else {
                    // "Normalize" the negative fitness values
                    std::vector<int> normalizedFitness;
                    int lowestValue = bottomPop.begin()->getFinalGameScore() * -1;
                    for (auto it = bottomPop.begin(); it != bottomPop.end(); it++) {
                        normalizedFitness.push_back(it->getFinalGameScore() + lowestValue);
                    }

                    int random_int = rand() % (cumulativeBottomFitness * -1);
                    for (int i = 0; i < normalizedFitness.size(); i++) {
                        random_int -= normalizedFitness.at(i);
                        if (random_int <= 0) { 
                            auto it = bottomPop.begin();
                            std::advance(it, i);
                            ghostMatingPool.insert(*it); 
                            break; 
                        }
                    }
                }
            }
        }
    }
    
    return;
}

// Perform sub-tree crossover
Population Population::mate(const int& offspringSize, const double& mutateRate, const double& ghostMutateRate, const int& treeDepthInit) {
    Population offspringPop;

    // Generate offspringSize number of offspring
    for (int i = 0; i < offspringSize; i++) {

        // Get random members of the mating pools
        auto it = pacMatingPool.begin();
        auto it2 = pacMatingPool.begin();
        std::advance(it, rand() % pacMatingPool.size());
        std::advance(it2, rand() % pacMatingPool.size());

        auto it3 = ghostMatingPool.begin();
        auto it4 = ghostMatingPool.begin();
        std::advance(it3, rand() % ghostMatingPool.size());
        std::advance(it4, rand() % ghostMatingPool.size());

        MsPacController parentOne = *it;
        MsPacController parentTwo = *it2;
        GhostController parentThree = *it3;
        GhostController parentFour = *it4;

        //boost::write_graphviz(std::cout, parentThree.getTree(), boost::make_label_writer(boost::get(&Node::debugText, parentThree.getTree())));
        //boost::write_graphviz(std::cout, parentFour.getTree(), boost::make_label_writer(boost::get(&Node::debugText, parentFour.getTree())));

        // Either mutate one Ms. Pac-Man or mate Ms. Pac-Man
        if (rand() % 100 < mutateRate * 100) {
            parentOne.mutate(treeDepthInit);
        }
        else {
            // Get a random vertex from both parents (crossover points)
            Vertex randomVertexOne;
            if (parentTwo.getTreeSize() > 1) { randomVertexOne = parentOne.getRandomVertex(); }
            else { randomVertexOne = parentOne.getRandomTermVertex(); }
            Vertex randomVertexTwo;
            if (parentOne.isFunction(randomVertexOne)) { randomVertexTwo = parentTwo.getRandomFuncVertex(); }
            else { randomVertexTwo = parentTwo.getRandomTermVertex(); }

            // Put crossover point and all points following from parent two to parent one
            std::queue<Vertex> verticesToAdd;
            std::queue<Vertex> verticesAdded;
            Vertex subTreeRoot = parentOne.addVertex(parentTwo.getNode(randomVertexTwo));
            verticesAdded.push(subTreeRoot);
            parentTwo.getNeighbors(randomVertexTwo, verticesToAdd);

            while (!verticesToAdd.empty()) {
                Vertex front = verticesToAdd.front();
                parentTwo.getNeighbors(front, verticesToAdd);

                // Add vertex
                Vertex newVertex = parentOne.addVertex(parentTwo.getNode(front));

                // Add an edge 
                if (!verticesAdded.empty()) { 
                    parentOne.addEdge(newVertex, verticesAdded.front()); 
                }

                // Update states
                if (parentOne.isFunction(newVertex)) { verticesAdded.push(newVertex); }
                verticesToAdd.pop();

                // Once the new vertex has reached proper arity, move to next one
                if (parentOne.inDegree(verticesAdded.front()) == FUNCTION_ARITY) { verticesAdded.pop(); }
            }

            // Add an edge to link the new sub-tree
            parentOne.setNewSubTree(subTreeRoot, randomVertexOne);

            // Prune the tree
            parentOne.prune(randomVertexOne);

            // Update states
            parentOne.updateState();
        }

        // Either mutate a ghost or mate two ghosts
        if (rand() % 100 < ghostMutateRate * 100) {
            //std::cout << "Mutation selected!" << std::endl;
            parentThree.mutate(treeDepthInit);
        }
        else {
            //std::cout << "Mating selected!" << std::endl;
            // Get a random vertex from both parents (crossover points)
            Vertex randomVertexThree;
            if (parentFour.getTreeSize() > 1) { randomVertexThree = parentThree.getRandomVertex(); }
            else { randomVertexThree = parentThree.getRandomTermVertex(); }
            //std::cout << parentThree.getNode(randomVertexThree).debugText << " " << parentThree.getNode(randomVertexThree).id << std::endl;
            Vertex randomVertexFour;
            if (parentThree.isFunction(randomVertexThree)) { randomVertexFour = parentFour.getRandomFuncVertex(); }
            else { randomVertexFour = parentFour.getRandomTermVertex(); }
            //std::cout << parentFour.getNode(randomVertexFour).debugText << " " << parentFour.getNode(randomVertexFour).id << std::endl;

            // Put crossover point and all points following from parent two to parent one
            std::queue<Vertex> verticesToAdd;
            std::queue<Vertex> verticesAdded;
            Vertex subTreeRoot = parentThree.addVertex(parentFour.getNode(randomVertexFour));
            verticesAdded.push(subTreeRoot);
            parentFour.getNeighbors(randomVertexFour, verticesToAdd);

            while (!verticesToAdd.empty()) {
                Vertex front = verticesToAdd.front();
                parentFour.getNeighbors(front, verticesToAdd);

                // Add vertex
                Vertex newVertex = parentThree.addVertex(parentFour.getNode(front));

                // Add an edge 
                if (!verticesAdded.empty()) { 
                    parentThree.addEdge(newVertex, verticesAdded.front()); 
                }

                // Update states
                if (parentThree.isFunction(newVertex)) { verticesAdded.push(newVertex); }
                verticesToAdd.pop();

                // Once the new vertex has reached proper arity, move to next one
                if (parentThree.inDegree(verticesAdded.front()) == FUNCTION_ARITY) { verticesAdded.pop(); }
            }

            // Add an edge to link the new sub-tree
            parentThree.setNewSubTree(subTreeRoot, randomVertexThree);

            // Prune the tree
            parentThree.prune(randomVertexThree);

            // Update states
            parentThree.updateState();
        }

        // Add the offspring to the offspring population
        offspringPop.insert(parentOne);
        offspringPop.insert(parentThree);

        /*boost::write_graphviz(std::cout, parentThree.getTree(), boost::make_label_writer(boost::get(&Node::debugText, parentThree.getTree())));
        exit(1);*/
    }

    // Clear the mating pools
    pacMatingPool.clear();
    ghostMatingPool.clear();

    return offspringPop;
}

void Population::selectSurvivors(const Population& offspringPop, const int& pacK, const int& pacPopSize, const std::string& pacSurvivalStrat,
                    const int& ghostK, const int& ghostPopSize, const std::string& ghostSurvivalStrat) {
    // Add all members of the offspring population to the total population
    std::multiset<MsPacController, ControllerCompare> temp = offspringPop.getPacPopulation();
    for (auto it = temp.begin(); it != temp.end(); it++) {
        cumulativeFitness += it->getFinalGameScore();
        pacPopulation.insert(*it);
    }
    std::multiset<GhostController, ControllerCompare> temp2 = offspringPop.getGhostPopulation();
    for (auto it = temp2.begin(); it != temp2.end(); it++) {
        cumulativeFitness += it->getFinalGameScore();
        ghostPopulation.insert(*it);
    }

    //////////////////////////////// SURVIVAL TECHNIQUES ///////////////////////////////
    // Survival selection for Ms. Pac-Man
    // Truncation
    if (pacSurvivalStrat == "truncation") {
        while (pacPopulation.size() > pacPopSize) {
            auto it = pacPopulation.begin();
            cumulativeFitness -= it->getFinalGameScore();
            pacPopulation.erase(it);
        }
    }
    // k-tournament without replacement
    else {
        std::multiset<MsPacController, ControllerCompare> survivalSet;
        int newCumulativeFitness = 0;

        // Build up survival set by conducting tournaments
        while (survivalSet.size() != pacPopSize) {
            std::multiset<MsPacController, ControllerCompare> tournamentSet;

            for (int i = 0; i < pacK; i++) {
                auto it = pacPopulation.begin();
                std::advance(it, rand() % pacPopulation.size());
                tournamentSet.insert(*it);

                // To prevent one individual from being selected multiple times
                pacPopulation.erase(it);
            }

            auto it2 = tournamentSet.rbegin();
            survivalSet.insert(*it2);
            newCumulativeFitness += it2->getFinalGameScore();
            tournamentSet.erase(std::prev(tournamentSet.end()));

            // Restore the individuals that did not win the tournament back into original population
            for (auto it = tournamentSet.begin(); it != tournamentSet.end(); it++) {
                pacPopulation.insert(*it);
            }
        }

        pacPopulation = survivalSet;
        cumulativeFitness = newCumulativeFitness;
    }

    // Survival selection for ghosts
    // Truncation
    if (ghostSurvivalStrat == "truncation") {
        while (ghostPopulation.size() > ghostPopSize) {
            auto it = ghostPopulation.begin();
            cumulativeFitness -= it->getFinalGameScore();
            ghostPopulation.erase(it);
        }
    }
    // k-tournament without replacement
    else {
        std::multiset<GhostController, ControllerCompare> survivalSet;
        int newCumulativeFitness = 0;

        // Build up survival set by conducting tournaments
        while (survivalSet.size() != ghostPopSize) {
            std::multiset<GhostController, ControllerCompare> tournamentSet;

            for (int i = 0; i < ghostK; i++) {
                auto it = ghostPopulation.begin();
                std::advance(it, rand() % ghostPopulation.size());
                tournamentSet.insert(*it);

                // To prevent one individual from being selected multiple times
                ghostPopulation.erase(it);
            }

            auto it2 = tournamentSet.rbegin();
            survivalSet.insert(*it2);
            newCumulativeFitness += it2->getFinalGameScore();
            tournamentSet.erase(std::prev(tournamentSet.end()));

            // Restore the individuals that did not win the tournament back into original population
            for (auto it = tournamentSet.begin(); it != tournamentSet.end(); it++) {
                ghostPopulation.insert(*it);
            }
        }

        ghostPopulation = survivalSet;
        gCumulativeFitness = newCumulativeFitness;
    }

    //////////////////////////////////////////////////////////////////////////////

    // Update average fitness of the new population
    avgFitness = static_cast<float>(cumulativeFitness) / pacPopSize;
    bestFitness = pacPopulation.rbegin()->getFinalGameScore();

    gAvgFitness = static_cast<float>(gCumulativeFitness) / ghostPopSize;
    gBestFitness = ghostPopulation.rbegin()->getFinalGameScore();

    return;
}

void Population::print() const {
    for (auto it = pacPopulation.begin(); it != pacPopulation.end(); it++) {
        std::cout << it->getFinalGameScore() << std::endl;
    }

    return;
}

void Population::printGhostTree() const {
    GhostController temp = *ghostPopulation.begin();
    //boost::write_graphviz(std::cout, temp.getTree(), boost::make_label_writer(boost::get(&Node::debugText, temp.getTree())));

    return;
}