//  msPacController.cpp
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#include "msPacController.h"

// Default constructor
MsPacController::MsPacController() {
    currentNodeId = stateEval = finalGameScore = gamesPlayed = 0;
}

// Copy constructor from base class
MsPacController::MsPacController(const MsPacController& toCopy) {
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
MsPacController::MsPacController(const int& d, const int& nodeId) {
    currentNodeId = nodeId;
    if (rand() % 2 == 0) { growTree(d); }
    else { fillTree(d); }

    // Debug
    /*boost::write_graphviz(std::cout, stateEvalTree, boost::make_label_writer(boost::get(&Node::debugText, stateEvalTree)));
    std::cout << std::endl;*/
}

///// Construct the tree up to a maximum depth of m (GROW) /////
void MsPacController::growTree(const int& m) {
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
                child.nodeType = rand() % NUM_TERMINALS + NUM_FUNCTIONS;

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
                    case 6: child.debugText = "t_PAC_GHOST"; break;
                    case 7: child.debugText = "t_PAC_PILL"; break;
                    case 8: child.debugText = "t_PAC_WALL"; break;
                    case 9: child.debugText = "t_PAC_FRUIT"; break;
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

///// Construct the tree up to depth d (FULL) /////
void MsPacController::fillTree(const int& d) {
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
                child.nodeType = rand() % NUM_TERMINALS + NUM_FUNCTIONS;

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
                    case 6: child.debugText = "t_PAC_GHOST"; break;
                    case 7: child.debugText = "t_PAC_PILL"; break;
                    case 8: child.debugText = "t_PAC_WALL"; break;
                    case 9: child.debugText = "t_PAC_FRUIT"; break;
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
void MsPacController::mutate(const int& treeDepthInit) {
    MsPacController temp(treeDepthInit, currentNodeId);
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

// Sets sensor inputs for Ms. Pac-Man from the world state
void MsPacController::setSensors(const Board& world) {
    PacSensors sensorInfo = world.getPacSensors();

    s_PAC_GHOST = std::get<PAC_GHOST_INDEX>(sensorInfo);
    s_PAC_PILL = std::get<PAC_PILL_INDEX>(sensorInfo);
    s_PAC_WALL = std::get<PAC_WALLS>(sensorInfo);
    s_PAC_FRUIT = std::get<PAC_FRUIT>(sensorInfo);

    return;
}

// Sets final game score for Ms. Pac-Man controller
void MsPacController::setFinalGameScore(const double& parsimonyCoefficient, const MsPacController& oldPac, const int& score) {
    double temp = score - parsimonyCoefficient * boost::num_vertices(stateEvalTree);

    // If the controller has played more than one game
    if (oldPac.getGamesPlayed() > 1) {
        finalGameScore = (finalGameScore + temp) / oldPac.getGamesPlayed();
    }
    else {
        finalGameScore = temp;
    }

    return;
}

// State evaluator function -- traverses the tree and calculates value from sensor input
void MsPacController::evaluateState() {
    std::queue<Vertex> temp = evalNodes;
    while (!temp.empty()) {
        Vertex front = temp.front();

        // Get the adjacent vertex and pass up the sensor input
        AdjacencyIterator ai, aend;
        tie(ai, aend) = adjacent_vertices(front, stateEvalTree);

        // If the vertex is a terminal node
        if (!stateEvalTree[front].isFunction) {

            switch(stateEvalTree[front].nodeType) {
                case TERMINAL_SENSOR_PAC_GHOST:
                    //std::cout << "pac-ghost: " << s_PAC_GHOST << std::endl;
                    if (ai == aend) { stateEval = s_PAC_GHOST; return; }
                    stateEvalTree[*ai].ops.push(s_PAC_GHOST);
                    break;
                case TERMINAL_SENSOR_PAC_PILL:
                    //std::cout << "pac-pill: " << s_PAC_PILL << std::endl;
                    if (ai == aend) { stateEval = s_PAC_PILL; return; }
                    stateEvalTree[*ai].ops.push(s_PAC_PILL); 
                    break;
                case TERMINAL_SENSOR_NUM_WALLS_ADJ:
                    //std::cout << "pac-wall: " << s_PAC_WALL << std::endl;
                    if (ai == aend) { stateEval = s_PAC_WALL; return; }
                    stateEvalTree[*ai].ops.push(s_PAC_WALL); 
                    break;
                case TERMINAL_SENSOR_PAC_FRUIT:
                    //std::cout << "pac-fruit: " << s_PAC_FRUIT << std::endl;
                    if (ai == aend) { stateEval = s_PAC_FRUIT; return; }
                    stateEvalTree[*ai].ops.push(s_PAC_FRUIT);
                    break;
                case TERMINAL_CONSTANT:
                    //std::cout << "pac-constant: " << stateEvalTree[front].nodeValue << std::endl;
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

MoveMap MsPacController::makeMove(const Board& world) const {
    // Get possible moves for Ms. Pac-Man 
    std::set<Coordinate> moves = world.getMsPacMoves();
    Coordinate bestMove;
    double bestEval = -std::numeric_limits<double>::max();

    // Make a copy of the controller so as not to violate const-ness
    MsPacController copy(*this);

    // Make a copy of the board
    Board temp(world);

    /* Loop through the possible moves, make the move, and apply the state evaluator function
       Record the move with the highest eval value and return it */
    for (auto it = moves.begin(); it != moves.end(); it++) {
        temp.updateMsPacLocation(*it);
        copy.setSensors(temp);
        copy.evaluateState();

        if (copy.getEval() > bestEval) { 
            bestEval = copy.getEval();
            bestMove = *it;
        }
    }

    // Storage for move (to be consistent with how ghosts return moves)
    MoveMap selectedMove;
    selectedMove[1] = bestMove;

    return selectedMove;
}

// Assignment 2a movement function
Coordinate MsPacController::makeRandomMove(const Board& world) {
    // Get possible moves for Ms. Pac-Man 
    std::set<Coordinate> moves = world.getMsPacMoves();

    // Choose a random move
    auto it = moves.begin();
    std::advance(it, rand() % moves.size());

    return *it;
}