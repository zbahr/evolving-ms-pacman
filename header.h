//  header.h
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#ifndef header_h
#define header_h

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <string>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp> 
#include <boost/graph/copy.hpp>
using namespace boost;

// General board definitions
const int NUM_GHOSTS = 3;
const int PROGRESS_VAR = 25;
typedef std::pair<int, int> Coordinate;
typedef std::map<int, Coordinate> MoveMap;

// Pac-Man sensor definitions
typedef std::tuple<int, int, int, int> PacSensors;
const int PAC_GHOST_INDEX = 0;
const int PAC_PILL_INDEX = 1;
const int PAC_WALLS = 2;
const int PAC_FRUIT = 3;

// Ghost sensor definitions
typedef std::tuple<int, int> GhostSensors;
const int GHOST_PAC_INDEX = 0;
const int GHOST_GHOST_INDEX = 1;

// Population definitions
const int MS_PAC_MAN = 0;
const int GHOSTS = 1;

// Tree definitions
const int NUM_FUNCTIONS = 5;
const int NUM_GHOST_TERMINALS = 3;
const int NUM_TERMINALS = 5;
const int FUNCTION_ARITY = 2;
const double CONSTANT_HIGH = 50;
const double CONSTANT_LOW = -50;
typedef struct { 
    int id;
    int isFunction; 
    int nodeType; 
    std::string debugText; 
    std::stack<double> ops;
    double nodeValue; 
} Node;
typedef boost::adjacency_list<listS, vecS, bidirectionalS, Node> Tree; 
typedef boost::graph_traits<Tree>::vertex_descriptor Vertex;
typedef boost::graph_traits<Tree>::edge_descriptor Edge;
typedef boost::graph_traits<Tree>::in_edge_iterator InEdgeIterator;
typedef boost::graph_traits<Tree>::out_edge_iterator OutEdgeIterator;
typedef boost::graph_traits<Tree>::vertex_iterator VertexIterator;
typedef boost::graph_traits<Tree>::adjacency_iterator AdjacencyIterator;
#define FUNCTION_ADD                     0
#define FUNCTION_SUB                     1
#define FUNCTION_MULT                    2
#define FUNCTION_DIV                     3
#define FUNCTION_RAND                    4
#define TERMINAL_CONSTANT                5
#define TERMINAL_SENSOR_GHOST_PAC        6
#define TERMINAL_SENSOR_GHOST_GHOST      7
#define TERMINAL_SENSOR_PAC_GHOST        6
#define TERMINAL_SENSOR_PAC_PILL         7
#define TERMINAL_SENSOR_NUM_WALLS_ADJ    8
#define TERMINAL_SENSOR_PAC_FRUIT        9
#define FUNCTION_NODE                    0
#define TERMINAL_NODE                    1
#define UNDEFINED                        -1

// Parsing index constants
const int COLUMNS_INDEX = 0;
const int ROWS_INDEX = 1;
const int PILL_DENSITY_INDEX = 2;
const int WALL_DENSITY_INDEX = 3;
const int FRUIT_SPAWN_INDEX = 4;
const int FRUIT_SCORE_INDEX = 5;
const int TIME_MULT_INDEX = 6;
const int SEED_INDEX = 7;
const int RUNS_INDEX = 8;
const int EVALS_INDEX = 9;
const int PAC_POP_INDEX = 10;
const int PAC_OFFSPRING_INDEX = 11;
const int PAC_MUTATE_INDEX = 12;
const int PAC_K_INDEX = 13;
const int PAC_P_INDEX = 14;
const int PAC_PARENT_STRAT_INDEX = 15;
const int PAC_SURVIVAL_STRAT_INDEX = 16;
const int GHOSTS_POP_INDEX = 17;
const int GHOSTS_OFFSPRING_INDEX = 18;
const int GHOSTS_MUTATE_INDEX = 19;
const int GHOSTS_K_INDEX = 20;
const int GHOSTS_P_INDEX = 21;
const int GHOSTS_PARENT_STRAT_INDEX = 22;
const int GHOSTS_SURVIVAL_STRAT_INDEX = 23;
const int TREE_INIT_DEPTH_INDEX = 24;
const int TERMINATION_STRAT_INDEX = 25;
const int TERM_CRIT_INDEX = 26;
const int LOG_INDEX = 27;
const int WORLD_INDEX = 28;
const int PAC_CONTROLLER_INDEX = 29;
const int GHOSTS_CONTROLLER_INDEX = 30;

struct coordHash {
    inline std::size_t operator()(const std::pair<int,int>& v) const {
        return v.first * 31 + v.second;
    }
};

#endif