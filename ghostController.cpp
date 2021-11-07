//  ghostController.cpp
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#include "ghostController.h"

// Default constructor
GhostController::GhostController() {
    currentNodeId = stateEval = finalGameScore = gamesPlayed = 0;
}

// Copy constructor from base class
GhostController::GhostController(const GhostController& toCopy) {
    stateEvalTree = toCopy.getTree();
    evalNodes = toCopy.getEvalNodes();
    termVertices = toCopy.getTermVertices();
    funcVertices = toCopy.getFuncVertices();
    vertices = toCopy.getVertices();
    currentNodeId = toCopy.getCurrentNodeId();
    stateEval = toCopy.getStateEval();
    finalGameScore = toCopy.getFinalGameScore();
}

/* Constructs the state eval tree for this controller. 
  Equal chances to "grow" or "fill" the tree for ramped half-and-half initialization */
GhostController::GhostController(const int& d, const int& nodeId) {
    currentNodeId = nodeId;
    if (rand() % 2 == 0) { growTree(d); }
    else { fillTree(d); }

    // Debug
    //boost::write_graphviz(std::cout, stateEvalTree, boost::make_label_writer(boost::get(&Node::debugText, stateEvalTree)));
    //std::cout << std::endl;
}

void GhostController::growTree(const int& m) {
    // Containers to record which nodes still need children added
    std::stack<Vertex> oldChildless;
    std::stack<Vertex> newChildless;
    for (int i = 0; i < m; i++) {
        bool onlyTerminals = false;
        if (i == m - 1) { onlyTerminals = true; }
        do {
            Node child;
            Vertex parent;
            if (!oldChildless.empty()) { parent = oldChildless.top(); }

            // Randomly decide whether the child is a function or terminal node
            if (!onlyTerminals && rand() % 2 == FUNCTION_NODE) {
                child.isFunction = true;
                child.nodeType = rand() % NUM_FUNCTIONS;
                child.nodeValue = UNDEFINED;
                child.id = currentNodeId;
                currentNodeId++;

                ////////////////// Debug ////////////////////
                switch(child.nodeType) {
                    case 0: child.debugText = "f_ADD"; break;
                    case 1: child.debugText = "f_SUB"; break;
                    case 2: child.debugText = "f_MULT"; break;
                    case 3: child.debugText = "f_DIV"; break;
                    case 4: child.debugText = "f_RAND"; break;
                }
                /////////////////////////////////////////////
            }
            else {
                child.isFunction = false;
                child.nodeType = rand() % NUM_GHOST_TERMINALS + NUM_FUNCTIONS;

                // If it is a terminal constant, generate the value
                 if (child.nodeType == TERMINAL_CONSTANT) { 
                    child.nodeValue = CONSTANT_LOW + (double)(rand()) / ((double)(RAND_MAX / (CONSTANT_HIGH - CONSTANT_LOW)));
                }
                else { child.nodeValue = UNDEFINED; }
                child.id = currentNodeId;
                currentNodeId++;

                ////////////////// Debug ////////////////////
                switch(child.nodeType) {
                    case 5: child.debugText = std::to_string(child.nodeValue); break;
                    case 6: child.debugText = "t_GHOST_PAC"; break;
                    case 7: child.debugText = "t_GHOST_GHOST"; break;
                }
                /////////////////////////////////////////////
            }

            // Create a new entry in the tree for the child
            Vertex newEntry = add_vertex(child, stateEvalTree);

            // If the child has a parent, add an edge between the two
            if (!oldChildless.empty()) { add_edge(newEntry, parent, stateEvalTree); }

            // If the vertex has two children, remove from the childless queue
            if (!oldChildless.empty() && in_degree(parent, stateEvalTree) == 2) { 
                oldChildless.pop(); 
            }

            // Push the new child vertex into the new childless queue
            if (child.isFunction) { newChildless.push(newEntry); funcVertices.push_back(newEntry); }
            // Keep record of all terminal nodes
            else { evalNodes.push(newEntry); termVertices.push_back(newEntry); }

            // Record of all nodes
            vertices.push_back(newEntry); 
        }
        while (!oldChildless.empty());

        // Update states
        oldChildless = newChildless;
        while (!newChildless.empty()) { newChildless.pop(); }
        // If there are no more children to add, break
        if (oldChildless.empty()) { break; }
    }

    // Zero out initial values
    stateEval = 0;
    finalGameScore = 0;

    return;
}

void GhostController::fillTree(const int& d) {
    // Containers to record which nodes still need children added
    std::stack<Vertex> oldChildless;
    std::stack<Vertex> newChildless;
    for (int i = 0; i < d; i++) {
        bool onlyTerminals = false;
        if (i == d - 1) { onlyTerminals = true; }
        do {
            Node child;
            Vertex parent;
            if (!oldChildless.empty()) { parent = oldChildless.top(); }

            // Child is a function unless at deepest depth
            if (!onlyTerminals) {
                child.isFunction = true;
                child.nodeType = rand() % NUM_FUNCTIONS;
                child.nodeValue = UNDEFINED;
                child.id = currentNodeId;
                currentNodeId++;

                ////////////////// Debug ////////////////////
                switch(child.nodeType) {
                    case 0: child.debugText = "f_ADD"; break;
                    case 1: child.debugText = "f_SUB"; break;
                    case 2: child.debugText = "f_MULT"; break;
                    case 3: child.debugText = "f_DIV"; break;
                    case 4: child.debugText = "f_RAND"; break;
                }
                /////////////////////////////////////////////
            }
            else {
                child.isFunction = false;
                child.nodeType = rand() % NUM_GHOST_TERMINALS + NUM_FUNCTIONS;

                // If it is a terminal constant, generate the value
                if (child.nodeType == TERMINAL_CONSTANT) { 
                    child.nodeValue = CONSTANT_LOW + (double)(rand()) / ((double)(RAND_MAX / (CONSTANT_HIGH - CONSTANT_LOW)));
                }
                else { child.nodeValue = UNDEFINED; }
                child.id = currentNodeId;
                currentNodeId++;

                ////////////////// Debug ////////////////////
                switch(child.nodeType) {
                    case 5: child.debugText = std::to_string(child.nodeValue); break;
                    case 6: child.debugText = "t_GHOST_PAC"; break;
                    case 7: child.debugText = "t_GHOST_GHOST"; break;
                }
                /////////////////////////////////////////////
            }

            // Create a new entry in the tree for the child
            Vertex newEntry = add_vertex(child, stateEvalTree);

            // If the child has a parent, add an edge between the two
            if (!oldChildless.empty()) { add_edge(newEntry, parent, stateEvalTree); }

            // If the vertex has two children, remove from the childless queue
            if (!oldChildless.empty() && in_degree(parent, stateEvalTree) == 2) { 
                oldChildless.pop(); 
            }

            // Push the new child vertex into the new childless queue
            if (child.isFunction) { newChildless.push(newEntry); funcVertices.push_back(newEntry); }
            // Keep record of all terminal nodes
            else { evalNodes.push(newEntry); termVertices.push_back(newEntry); }

            // Record of all nodes
            vertices.push_back(newEntry); 
        }
        while (!oldChildless.empty());

        // Update states
        oldChildless = newChildless;
        while (!newChildless.empty()) { newChildless.pop(); }

        // If there are no more children to add, break
        if (oldChildless.empty()) { break; }
    }

    // Zero out initial values
    stateEval = 0;
    finalGameScore = 0;

    return;
}

// Mutation function
void GhostController::mutate(const int& treeDepthInit) {
    GhostController temp(treeDepthInit, currentNodeId);
    copy_graph(temp.getTree(), stateEvalTree);

    // Vertex to delete
    Vertex randomVertex = getRandomTermVertex();

    // Id of mutated root to link to tree
    int mutatedId = temp.getRoot();
    if (mutatedId != -1) {
        // Get mutated vertex root
        Vertex mutatedVertexRoot;
        VertexIterator vi, vend;
        for (tie(vi, vend) = boost::vertices(stateEvalTree); vi != vend; vi++) {
            if (stateEvalTree[*vi].id == mutatedId) { mutatedVertexRoot = *vi; }
        }

        // Make the connection
        OutEdgeIterator oi, oend;
        for (tie(oi, oend) = boost::out_edges(randomVertex, stateEvalTree); oi != oend; oi++) {
            if (oi != oend) {
                add_edge(mutatedVertexRoot, target(*oi, stateEvalTree), stateEvalTree);
                remove_edge(*oi, stateEvalTree);
                break;
            }
        }

        // Remove the vertex to delete
        remove_vertex(randomVertex, stateEvalTree);
    }

    updateState();

    return;
}

// Sets sensor inputs for the ghosts from the world state
void GhostController::setSensors(const Board& world) {
    GhostSensors sensorInfo = world.getGhostSensors();

    s_GHOST_PAC = std::get<GHOST_PAC_INDEX>(sensorInfo);
    s_GHOST_GHOST = std::get<GHOST_GHOST_INDEX>(sensorInfo);

    return;
}

// Sets final game score for Ghost controller
void GhostController::setFinalGameScore(const double& parsimonyCoefficient, const GhostController& oldGhosts, const int& score) {
    double temp = -1 * (score - parsimonyCoefficient * boost::num_vertices(stateEvalTree));
    if (temp > 0) { temp = 0; }

    // If the controller has played more than one game
    if (oldGhosts.getGamesPlayed() > 1) {
        finalGameScore = (finalGameScore + temp) / oldGhosts.getGamesPlayed();
    }
    else {
        finalGameScore = temp;
    }

    return;
}

// State evaluator function -- traverses the tree and calculates value from sensor input
void GhostController::evaluateState() {
    std::queue<Vertex> temp = evalNodes;
    while (!temp.empty()) {
        Vertex front = temp.front();

        // Get the adjacent vertex and pass up the sensor input
        AdjacencyIterator ai, aend;
        tie(ai, aend) = adjacent_vertices(front, stateEvalTree);

        // If the vertex is a terminal node
        if (!stateEvalTree[front].isFunction) {
            switch(stateEvalTree[front].nodeType) {
                case TERMINAL_SENSOR_GHOST_PAC:
                    //std::cout << "ghost-pac: " << s_GHOST_PAC << std::endl;
                    if (ai == aend) { stateEval = s_GHOST_PAC; return; }
                    stateEvalTree[*ai].ops.push(s_GHOST_PAC);
                    break;
                case TERMINAL_SENSOR_GHOST_GHOST:
                    //std::cout << "ghost-ghost: " << s_GHOST_GHOST << std::endl;
                    if (ai == aend) { stateEval = s_GHOST_GHOST; return; }
                    stateEvalTree[*ai].ops.push(s_GHOST_GHOST); 
                    break;
                case TERMINAL_CONSTANT:
                    //std::cout << "ghost-constant: " << stateEvalTree[front].nodeValue << std::endl;
                    if (ai == aend) { stateEval = stateEvalTree[front].nodeValue; return; }
                    stateEvalTree[*ai].ops.push(stateEvalTree[front].nodeValue);
                    break;
            }
        }
        // If the vertex is a function node -- perform function and then pass value to adjacent vertex
        else {
            // Get the operands to perform function
            double op1, op2, result;
            op1 = stateEvalTree[front].ops.top();
            stateEvalTree[front].ops.pop();
            op2 = stateEvalTree[front].ops.top();
            stateEvalTree[front].ops.pop();

            // Perform the function and pass up
            switch(stateEvalTree[front].nodeType) {
                case FUNCTION_ADD:
                    if (ai == aend) { stateEval = op1 + op2; return; }
                    stateEvalTree[*ai].ops.push(op1 + op2);
                    break;
                case FUNCTION_SUB:
                    if (ai == aend) { stateEval = op1 - op2; return; }
                    stateEvalTree[*ai].ops.push(op1 - op2);
                    break;
                case FUNCTION_MULT:
                    if (ai == aend) { stateEval = op1 * op2; return; }
                    stateEvalTree[*ai].ops.push(op1 * op2);
                    break;
                case FUNCTION_DIV:
                    if (op2 != 0) { result = op1 / op2; }
                    else { result = op1; }
                    
                    if (ai == aend) { stateEval = result; return; }
                    stateEvalTree[*ai].ops.push(result);

                    break;
                case FUNCTION_RAND:
                    if (op2 > op1) { result = op1 + (double)(rand()) / ((double)(RAND_MAX / (op2 - op1))); }
                    else if (op2 < op1) { result = op2 + (double)(rand()) / ((double)(RAND_MAX / (op1 - op2))); }
                    else { result = op1; }

                    if (ai == aend) { stateEval = result; return; }
                    stateEvalTree[*ai].ops.push(result);

                    break;
            }
        }

        // If the adjacent vertex has enough operands to perform its operation, add to evaluation queue
        if (stateEvalTree[*ai].ops.size() == FUNCTION_ARITY) {
            temp.push(*ai);
        }

        temp.pop();
    }

    return;
}

MoveMap GhostController::makeMove(const Board& world) const {
    // Get possible moves for ghosts
    std::map<int, std::set<Coordinate> > moves = world.getGhostMoves();
    MoveMap selectedMove;
    double bestEval = -std::numeric_limits<double>::max();

    // Make a copy of the controller so as not to violate const-ness
    GhostController copy(*this);

    // Make the board
    Board temp(world);

    /* Loop through the possible moves, make the move, and apply the state evaluator function
       Record the move with the highest eval value and return it */
    std::set<Coordinate> ghostOneMoves = moves[1];
    std::set<Coordinate> ghostTwoMoves = moves[2];
    std::set<Coordinate> ghostThreeMoves = moves[3];

    // Loop through all possible ghost states
    MoveMap newMove;
    for (auto ghostOne = ghostOneMoves.begin(); ghostOne != ghostOneMoves.end(); ghostOne++) {
        newMove[1] = *ghostOne;
        for (auto ghostTwo = ghostTwoMoves.begin(); ghostTwo != ghostTwoMoves.end(); ghostTwo++) {
            newMove[2] = *ghostTwo;
            for (auto ghostThree = ghostThreeMoves.begin(); ghostThree != ghostThreeMoves.end(); ghostThree++) {
                newMove[3] = *ghostThree;
                temp.updateGhostLocations(newMove);
                copy.setSensors(temp);
                copy.evaluateState();

                if (copy.getEval() > bestEval) { 
                    bestEval = copy.getEval();
                    selectedMove[1] = newMove[1];
                    selectedMove[2] = newMove[2];
                    selectedMove[3] = newMove[3];
                }
            }
        }
    }

    return selectedMove;
}

MoveMap GhostController::makeRandomMoves(const Board& world) const {
    // Get possible moves for all of the ghosts
    std::map<int, std::set<Coordinate>> moves = world.getGhostMoves();

    // Storage for moves selected
    MoveMap selectedMoves;

    // Randomly selected a move for each ghost
    for (int i = 1; i <= NUM_GHOSTS; i++) {
        auto it = moves.at(i).begin();
        std::advance(it, rand() % moves.at(i).size());
        selectedMoves[i] = *it;
    }

    return selectedMoves;
}