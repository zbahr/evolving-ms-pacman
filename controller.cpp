//  controller.cpp
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#include "controller.h"

// Add a vertex to the state eval tree
Vertex Controller::addVertex(Node n) {
    n.id = currentNodeId;
    currentNodeId++;

    return add_vertex(n, stateEvalTree);
}

// Add an edge between vertices in the state eval tree
void Controller::addEdge(const Vertex& source, const Vertex& destination) {
    add_edge(source, destination, stateEvalTree);

    return;
}

// Links the sub-tree that has been crossed over
void Controller::setNewSubTree(const Vertex& source, const Vertex& adjDestination) {
    AdjacencyIterator ai, aend;
    for (tie(ai, aend) = adjacent_vertices(adjDestination, stateEvalTree); ai != aend; ai++) {
        if (ai != aend) {
            addEdge(source, *ai);
        }
    }

    return;
}

// Prunes the tree starting from the pruneRoot vertex
void Controller::prune(const Vertex& pruneRoot) {
    AdjacencyIterator ai, aend;
    for (tie(ai, aend) = adjacent_vertices(pruneRoot, stateEvalTree); ai != aend; ai++) {
        if (ai != aend) {
            remove_edge(pruneRoot, *ai, stateEvalTree);
            break;
        }
    }

    std::queue<Vertex> verticesToRemove;
    std::vector<Node> nodesToRemove;
    verticesToRemove.push(pruneRoot);
    nodesToRemove.push_back(stateEvalTree[pruneRoot]);

    // Remove the edges first
    do {
        if (stateEvalTree[verticesToRemove.front()].isFunction) {
            InEdgeIterator ei, eend;
            for (tie(ei, eend) = in_edges(verticesToRemove.front(), stateEvalTree); ei != eend; ei++) {
                if (ei != eend) {
                    verticesToRemove.push(source(*ei, stateEvalTree));
                    nodesToRemove.push_back(stateEvalTree[source(*ei, stateEvalTree)]);
                    remove_edge(*ei, stateEvalTree);
                }
                break;
            }
            for (tie(ei, eend) = in_edges(verticesToRemove.front(), stateEvalTree); ei != eend; ei++) {
                if (ei != eend) {
                    verticesToRemove.push(source(*ei, stateEvalTree));
                    nodesToRemove.push_back(stateEvalTree[source(*ei, stateEvalTree)]);
                    remove_edge(*ei, stateEvalTree);
                }
                break;
            }
        }

        verticesToRemove.pop();
    }
    while (!verticesToRemove.empty());

    // Remove vertices (nodes) next
    for (auto it = nodesToRemove.begin(); it != nodesToRemove.end(); it++) {
        VertexIterator vi, vend;
        for (boost::tie(vi, vend) = boost::vertices(stateEvalTree); vi != vend; vi++) {
            if (stateEvalTree[*vi].id == it->id) {
                remove_vertex(*vi, stateEvalTree);
                break;
            }
        }
    }

    return;
}

// After the mating has finished, update states
void Controller::updateState() {
    finalGameScore = 0;

    std::queue<Vertex> newEvalNodes;
    std::vector<Vertex> newTermVertices;
    std::vector<Vertex> newFuncVertices;
    std::vector<Vertex> newVertices;

    VertexIterator vi, vend;
    for (tie(vi, vend) = boost::vertices(stateEvalTree); vi != vend; vi++) {
        if (stateEvalTree[*vi].isFunction) { newFuncVertices.push_back(*vi); }
        else if (!stateEvalTree[*vi].isFunction) {
            newEvalNodes.push(*vi);
            newTermVertices.push_back(*vi);
        }
        newVertices.push_back(*vi);
    }

    evalNodes = newEvalNodes;
    termVertices = newTermVertices;
    funcVertices = newFuncVertices;
    vertices = newVertices;

    return;
}

// Gets a random terminal node (crossover point) 
Vertex Controller::getRandomTermVertex() const {
    auto it = termVertices.begin();
    std::advance(it, rand() % evalNodes.size());

    return *it;
}

// Gets a random function node (crossover point) 
Vertex Controller::getRandomFuncVertex() const {
    auto it = funcVertices.begin();
    std::advance(it, rand() % funcVertices.size());

    return *it;
}

// Gets a random node (crossover point)
Vertex Controller::getRandomVertex() const {
    auto it = vertices.begin();
    std::advance(it, rand() % vertices.size());

    return *it;
}

// Gets the id of the root of the tree
int Controller::getRoot() const {
    VertexIterator vi, vend;
    for (tie(vi, vend) = boost::vertices(stateEvalTree); vi != vend; vi++) {
        OutEdgeIterator ei, eend;
        tie(ei, eend) = boost::out_edges(*vi, stateEvalTree);
        if (ei == eend) { return stateEvalTree[*vi].id; }
    }

    return -1;
}

// Puts the neighboring vertices of v into the queue
void Controller::getNeighbors(const Vertex& v, std::queue<Vertex>& verticesToAdd) const {
    InEdgeIterator ei, eend;
    for (tie(ei, eend) = in_edges(v, stateEvalTree); ei != eend; ei++) {
        verticesToAdd.push(source(*ei, stateEvalTree));
    }

    return;
}

// Return the in degree of the vertex v
int Controller::inDegree(const Vertex& v) const {
    return in_degree(v, stateEvalTree);
}

// Determines whether v is a function or terminal vertex
bool Controller::isFunction(const Vertex& v) const
{ 
    if (stateEvalTree[v].isFunction) { return true; } 
    return false; 
}

// Debugging function
void Controller::print() const {
    /*boost::write_graphviz(std::cout, stateEvalTree, boost::make_label_writer(boost::get(&Node::debugText, stateEvalTree)));
    std::cout << finalGameScore << std::endl << std::endl;*/

    return;
}