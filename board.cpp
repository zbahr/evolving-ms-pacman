//  board.cpp
//  CS 5401 - Assignment 2c - Competitive Coevolutionary Search
//
//  Created by Zachary Bahr on 10/31/18
//  Copyright © 2018 Zachary Bahr. All rights reserved.

#include "board.h"

bool isValid(Board temp, Coordinate startLoc);

// Default constructor
Board::Board() {
    grid = NULL;
    score = 0;
    columns = rows = 0;
    timeRemaining = totalTime = 0;
    fruitConsumed = fruitValue = 0;
    pillsConsumed = totalPills = 0;
}

// Deep copy constructor
Board::Board(const Board& b) {
    grid = NULL;
    score = 0;
    columns = rows = 0;
    timeRemaining = totalTime = 0;
    fruitConsumed = fruitValue = 0;
    pillsConsumed = totalPills = 0;
    *this = b;
}

// Destructor
Board::~Board() {
    if (grid != NULL) {
        for (int i = 0; i < rows; i++) {
            delete [] grid[i];
        }
        delete [] grid;
    }
}

// Initialize valid world state based on parameters
Board::Board(int columns, int rows, float pillDensity, float wallDensity, int timeMultiplier, int fScore) {
    // Set the dimensions of the board
    setDimensions(columns, rows);

    // Set all spaces to "empty"
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            emptySet.insert(std::make_pair(j, i));
        }
    }

    // Set Ms. Pac-Man location
    pacLocation = std::make_pair(0, rows - 1);
    emptySet.erase(pacLocation);
    grid[rows - 1][0].isPac = true;

    // Set ghost locations
    for (int i = 1; i <= NUM_GHOSTS; i++) {
        ghostLocations[i] = std::make_pair(columns - 1, 0);
    }
    emptySet.erase(std::make_pair(columns - 1, 0));
    grid[0][columns - 1].isGhost = true;
    grid[0][columns - 1].ghostCount = NUM_GHOSTS;


    /* Interval at which to check if board is still valid.
       Values were chosen after many test runs. With this wall check variable, 
       an 80x80 valid board is generated every ~10 s ; originally ~2.5 minutes */
    const int WALL_CHECK = std::max(8.0, columns * rows * .0035); 

    ////// Set walls ("genotype") ///////
    bool validWalls = true;
    int expWallCount = std::max(1.0, wallDensity * .01 * (columns * rows - 2));
    
    // Used as storage for the most recent walls added (so they can be removed if board becomes invalid)
    std::vector<Coordinate> tempWallRecord;
    do {
        for (int i = 1; i <= expWallCount;) {
            std::set<Coordinate>::iterator it;
            int x = rand() % columns;
            int y = rand() % rows;

            /////////// Get a random coordinate from ordered set emptySet in O(logn) ////////////
            if (!emptySet.empty()) {
                // Use a dummy insert to get at a random value in the set
                std::pair<std::set<Coordinate>::iterator, bool> temp = emptySet.insert(std::make_pair(x, y));
        
                // If the insert was successful
                if (temp.second) {
                    if (temp.first != --(emptySet.end())) {
                        auto it = temp.first;
                        it++;
                        x = it->first;
                        y = it->second;

                        emptySet.erase(it);
                        emptySet.erase(temp.first);
                    }
                    else {
                        x = emptySet.begin()->first;
                        y = emptySet.begin()->second;

                        emptySet.erase(emptySet.begin());
                        emptySet.erase(temp.first);
                    }
                }
                // If insert was unsuccessful --> coordinate already in set
                else {
                    x = temp.first->first;
                    y = temp.first->second;

                    emptySet.erase(temp.first);
                }
            }
            ////////////////////////////////////////////////////////////////////////////////////
            tempWallRecord.push_back(std::make_pair(x, y));

            /* After every WALL_CHECK walls are placed, check if the board is still valid
              (This is to decrease the calls to isValid() to increase efficiency) */
            if (tempWallRecord.size() % WALL_CHECK == 0 || tempWallRecord.size() == expWallCount - i + 1 ) {
                for (auto it = tempWallRecord.begin(); it != tempWallRecord.end(); it++) {
                    wallSet.insert(*it);
                }

                // Check if last few wall placements are valid
                validWalls = isValid(Board(*this), pacLocation) && isValid(Board(*this), ghostLocations[1]);

                // If the wall placements are invalid, undo the last WALL_CHECK number of walls
                if (validWalls) { 
                    tempWallRecord.clear();
                    i += WALL_CHECK;
                }
                else {
                    while (!tempWallRecord.empty()) {
                        Coordinate wallToErase = tempWallRecord.back();
                        wallSet.erase(wallToErase);
                        emptySet.insert(wallToErase);
                        tempWallRecord.pop_back();
                    }
                }
            }
        }
    }
    while (!validWalls);

    // Set walls ("phenotype")
    for (auto it = wallSet.begin(); it != wallSet.end(); it++) {
        grid[it->second][it->first].isWall = true;
    }

    // Update empty spaces
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if (wallSet.find(std::make_pair(j, i)) == wallSet.end()) {
                emptySet.insert(std::make_pair(j, i));
            }
        }
    }

    // Set pills
    totalPills = std::max(1.0, pillDensity * .01 * (columns * rows - wallSet.size() - 1));
    for (int i = 0; i < totalPills; i++) {
        auto it = emptySet.begin();
        std::advance(it, rand() % emptySet.size());

        // Ensure that Ms. Pac-Man's starting space does not contain a pill
        while (it->first == 0 && it->second == rows - 1) {
            it = emptySet.begin();
            std::advance(it, rand() % emptySet.size());
        }
        Coordinate c = *it;

        // Update sets
        emptySet.erase(it);
        pillSet.insert(c);
        grid[c.second][c.first].isPill = true;
    }

    // Set location of fruit (no fruit === (-1, -1))
    fruitLocation = std::make_pair(-1, -1);

    // Set score
    score = 0;

    // Set time
    timeRemaining = totalTime = columns * rows * timeMultiplier;

    // Set pills consumed
    pillsConsumed = 0;

    // Set fruit score
    fruitValue = fScore;

    // Set fruits consumed
    fruitConsumed = 0;
}

// Set the dimensions of the grid.
// Pre: x = number of columns, y = number of rows
void Board::setDimensions(int x, int y) {
    grid = new Cell*[y];
    for (int i = 0; i < y; i++) {
        grid[i] = new Cell[x];
    }

    columns = x;
    rows = y;

    return;
}

// Sets a wall on the board
void Board::setWall(Coordinate c) {
    wallSet.insert(c);
    grid[c.second][c.first].isWall = true;

    return;
}

// Adjust the score if Ms. Pac-Man eats a pill or fruit
void Board::adjustScore() {
    // Increment score if Ms. Pac-Man is on a space with a pill
    auto it = pillSet.find(pacLocation);
    if (it != pillSet.end()) {
        pillsConsumed++;
        emptySet.insert(*it);
        pillSet.erase(it);
        grid[it->second][it->first].isPill = false;
    }

    // Increment score if Ms. Pac-Man is on a space with a fruit
    if (pacLocation == fruitLocation) {
        fruitConsumed++;
        emptySet.insert(fruitLocation);
        grid[fruitLocation.second][fruitLocation.first].isFruit = false;
        fruitLocation = std::make_pair(-1, -1);
    }

    // Update the score
    score = static_cast<float>(pillsConsumed) / totalPills * 100 + fruitConsumed * fruitValue;

    return;
}

// Decrement the time
void Board::decreaseTime() {
    timeRemaining--;
    return;
}

// Spawns a fruit onto the game board in a random, empty cell
void Board::spawnFruit() {
    // Get a random empty cell
    auto it = emptySet.begin();
    std::advance(it, rand() % emptySet.size());

    // Ensure the fruit does not spawn at Ms. Pac-Man's location
    while (it->first == pacLocation.first && it->second == pacLocation.second) {
        it = emptySet.begin();
        std::advance(it, rand() % emptySet.size());
    }

    // Place the fruit
    fruitLocation = *it;
    grid[it->second][it->first].isFruit = true;
    emptySet.erase(it);

    return;
}

// Moves Ms. Pac-Man to the location specified in newLoc
void Board::updateMsPacLocation(const Coordinate& newLoc) {
    // Remove old grid state
    grid[pacLocation.second][pacLocation.first].isPac = false;

    // Set new state
    pacLocation = newLoc;
    grid[pacLocation.second][pacLocation.first].isPac = true;

    return;
}

// Moves Ms. Pac-Man to the location specified in newLoc
void Board::updateMsPacLocation(const MoveMap& newLoc) {
    // Remove old grid state
    grid[pacLocation.second][pacLocation.first].isPac = false;

    // Set new state
    pacLocation = newLoc.at(1);
    grid[pacLocation.second][pacLocation.first].isPac = true;

    return;
}

// Moves every ghost to their individual locations specified in newLocs
void Board::updateGhostLocations(const MoveMap& newLocs) {
    for (int i = 1; i <= NUM_GHOSTS; i++) {
        // Remove old grid state
        grid[ghostLocations[i].second][ghostLocations[i].first].isGhost = false;
        grid[ghostLocations[i].second][ghostLocations[i].first].ghostCount--;

        // Set new state
        ghostLocations[i] = newLocs.at(i);
        grid[newLocs.at(i).second][newLocs.at(i).first].isGhost = true;
        grid[newLocs.at(i).second][newLocs.at(i).first].ghostCount++;
    }

    return;
}

// Determines if there is no fruit on the board
bool Board::noFruit() const {
    if (fruitLocation == std::make_pair(-1, -1)) {
        return true;
    }
    
    return false;
}

// Determines if ghosts caused game over
bool Board::ghostGameOver(const Coordinate& oldMsPac, const MoveMap& oldGhosts) const {
    bool gameOver = false;

    // For every ghost . . .
    for (int i = 1; i <= NUM_GHOSTS; i++) {

        // Condition for if ghost and Ms. Pac-Man are in the same cell
        if (ghostLocations.at(i) == pacLocation) { 
            gameOver = true; 
            break; 
        }

        // Condition for if ghost and Ms. Pac-Man swapped cells
        if (ghostLocations.at(i) == oldMsPac && pacLocation == oldGhosts.at(i)) {
            gameOver = true;
            break;
        }

    }

    return gameOver;
}

// Determines if time or pills caused game over
bool Board::otherGameOver() {
    bool gameOver = false;

    // Condition for if all pills have been removed
    if (pillSet.empty()) {
        gameOver = true;

        // Increment score by percentage of time remaining
        score += static_cast<float>(timeRemaining) / totalTime;
    }

    // Condition for if time limit has been reached
    if (timeRemaining == 0) {
        gameOver = true;
    }

    return gameOver;
}

// Returns the sensory information for Pac-Man
PacSensors Board::getPacSensors() const {
    // Sensor information storage
    int distance = 1;
    int pillDistance, fruitDistance, ghostDistance;
    bool pillFound, fruitFound, ghostFound;
    pillFound = ghostFound = false;
    if (fruitLocation != std::make_pair(-1, -1)) { fruitFound = false; }
    else { fruitFound = true; fruitDistance = 0; }

    // Auxiliary storage containers
    std::set<Coordinate> exploredSet;
    std::queue<Coordinate> oldNeighbors = senseNeighbors(pacLocation);
    std::queue<Coordinate> newNeighbors;

    // Base Case: Ms Pac-Man is on a space with a pill, ghost, or fruit
    exploredSet.insert(pacLocation);
    if (pillSet.find(pacLocation) != pillSet.end()) { pillFound = true; pillDistance = 0; }
    if (fruitLocation == pacLocation) { fruitFound = true; fruitDistance = 0; }
    for (auto it = ghostLocations.begin(); it != ghostLocations.end(); it++) {
        if (it->second == pacLocation) { ghostFound = true; ghostDistance = 0; }
    }

    // Keep track of where we have explored
    exploredSet.insert(oldNeighbors.front());

    // Starting from Pac-Man location, "swarm" outward to find objects
    do {
        Coordinate top = oldNeighbors.front();

        // If the neighbor is a pill and a pill has not already been found
        if (!oldNeighbors.empty() && !pillFound && pillSet.find(top) != pillSet.end()) {
            pillDistance = distance;
            pillFound = true;
        }
        // If the neighbor is a fruit and a fruit has not already been found
        else if (!oldNeighbors.empty() && !fruitFound && top == fruitLocation) {
            fruitDistance = distance;
            fruitFound = true;
        }
        // If the neighbor is a ghost and a ghost has not already been found
        else if (!oldNeighbors.empty() && !ghostFound) {
            for (auto it = ghostLocations.begin(); it != ghostLocations.end(); it++) {
                if (top == it->second) {
                    ghostDistance = distance;
                    ghostFound = true;
                }
            }
        }

        // Get the neighbors of the neighbor and store them
        std::queue<Coordinate> temp = senseNeighbors(top);
        while (!temp.empty()) { 
            if (exploredSet.find(temp.front()) == exploredSet.end()) { 
                exploredSet.insert(temp.front());
                newNeighbors.push(temp.front()); 
            }
            temp.pop();
        }

        // Update queue
        oldNeighbors.pop();

        if (oldNeighbors.empty()) {
            distance++;
            oldNeighbors = newNeighbors;

            while (!newNeighbors.empty()) { newNeighbors.pop(); }
        }
    }
    while ((!pillFound || !ghostFound || !fruitFound) && !oldNeighbors.empty());

    // Tuple of sensor information to return
    PacSensors sensorInfo(ghostDistance, pillDistance, senseWalls(pacLocation), fruitDistance);

    return sensorInfo;
}

// Returns the sensory information for the ghosts
GhostSensors Board::getGhostSensors() const {
    // Sensor information storage
    int minPacDistance, minGhostDistance;
    minPacDistance = minGhostDistance = INT_MAX;

    // For each ghost, calculate distances
    for (auto it = ghostLocations.begin(); it != ghostLocations.end(); it++) {
        // Sensor information for each ghost
        int distance = 1;
        int pacDistance, ghostDistance;
        bool pacFound, ghostFound;
        pacFound = ghostFound = false;

        // Auxiliary storage containers
        std::set<Coordinate> exploredSet;
        std::queue<Coordinate> oldNeighbors = senseNeighbors(it->second);
        std::queue<Coordinate> newNeighbors;

        // Base Case: Ghost is on a space with a ghost or Ms. Pac-Man
        exploredSet.insert(it->second);
        if (pacLocation == it->second) { pacFound = true; pacDistance = 0; }
        for (auto it2 = ghostLocations.begin(); it2 != ghostLocations.end(); it2++) {
            if (it2->second == it->second && it != it2) { 
                ghostFound = true; 
                ghostDistance = 0; 
                break;
            }
        }

        // Keep track of where we have explored
        exploredSet.insert(oldNeighbors.front());

        // Starting from the ghost location, "swarm" outward to find objects
        do {
            Coordinate top = oldNeighbors.front();

            // If the neighbor is Ms. Pac-Man and Ms. Pac-Man has not already been found
            if (!oldNeighbors.empty() && !pacFound && pacLocation == top) {
                pacDistance = distance;
                pacFound = true;
            }
            // If the neighbor is a ghost and a ghost has not already been found
            else if (!oldNeighbors.empty() && !ghostFound) {
                for (auto it = ghostLocations.begin(); it != ghostLocations.end(); it++) {
                    if (top == it->second) {
                        ghostDistance = distance;
                        ghostFound = true;
                    }
                }
            }

            // Get the neighbors of the neighbor and store them
            std::queue<Coordinate> temp = senseNeighbors(top);
            while (!temp.empty()) { 
                if (exploredSet.find(temp.front()) == exploredSet.end()) { 
                    exploredSet.insert(temp.front());
                    newNeighbors.push(temp.front()); 
                }
                temp.pop();
            }

            // Update queue
            oldNeighbors.pop();

            if (oldNeighbors.empty()) {
                distance++;
                oldNeighbors = newNeighbors;

                while (!newNeighbors.empty()) { newNeighbors.pop(); }
            }
        }
        while ((!pacFound || !ghostFound) && !oldNeighbors.empty());

        // Update global distances if a smaller distance has been found
        if (pacDistance < minPacDistance) { minPacDistance = pacDistance; }
        if (ghostDistance < minGhostDistance) { minGhostDistance = ghostDistance; }
    }

    GhostSensors sensorInfo(minPacDistance, minGhostDistance);

    return sensorInfo;
}

// Gets the neighboring spaces of a coordinate (excluding walls and starting spaces)
// getNeighbors() used primarily for means of generating a valid board state during initialization
std::stack<Coordinate> Board::getNeighbors(const Coordinate& c) const {
    int x = c.first;
    int y = c.second;
    std::stack<Coordinate> neighbors;

    if (x + 1 < columns && (x + 1 != columns - 1 || y != 0) && wallSet.find(std::make_pair(x + 1, y)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x + 1, y)); 
    }
    if (x - 1 >= 0 && (x - 1 != 0 || y != rows - 1) && wallSet.find(std::make_pair(x - 1, y)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x - 1, y)); 
    }
    if (y + 1 < rows && (x != 0 || y + 1 != rows - 1) && wallSet.find(std::make_pair(x, y + 1)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x, y + 1)); 
    }
    if (y - 1 >= 0 && (x != columns - 1 || y - 1 != 0) && wallSet.find(std::make_pair(x, y - 1)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x, y - 1)); 
    }

    return neighbors;
}

// Gets the neighboring spaces of a coordinate (excluding walls)
// senseNeighbors() used primarily for means of sensor collection for Ms. Pac-Man and ghosts
std::queue<Coordinate> Board::senseNeighbors(const Coordinate& c) const {
    int x = c.first;
    int y = c.second;
    std::queue<Coordinate> neighbors;

    if (x + 1 < columns && wallSet.find(std::make_pair(x + 1, y)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x + 1, y)); 
    }
    if (x - 1 >= 0 && wallSet.find(std::make_pair(x - 1, y)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x - 1, y)); 
    }
    if (y + 1 < rows && wallSet.find(std::make_pair(x, y + 1)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x, y + 1)); 
    }
    if (y - 1 >= 0 && wallSet.find(std::make_pair(x, y - 1)) == wallSet.end()) { 
        neighbors.push(std::make_pair(x, y - 1)); 
    }

    return neighbors;
}

// Gets the number of adjacent walls to c
// senseWalls() used primarily for means of sensor collection for Ms. Pac-Man
int Board::senseWalls(const Coordinate& c) const {
    int x = c.first;
    int y = c.second;
    int numWalls = 0;

    if (x + 1 < columns && wallSet.find(std::make_pair(x + 1, y)) != wallSet.end()) { numWalls++; }
    if (x - 1 >= 0 && wallSet.find(std::make_pair(x - 1, y)) != wallSet.end()) { numWalls++; }
    if (y + 1 < rows && wallSet.find(std::make_pair(x, y + 1)) != wallSet.end()) { numWalls++; }
    if (y - 1 >= 0 && wallSet.find(std::make_pair(x, y - 1)) != wallSet.end()) { numWalls++; }

    return numWalls;
}

// Gets the valid moves for Ms. Pac-Man and returns as a set to the controller that is calling
std::set<Coordinate> Board::getMsPacMoves() const {
    int x = pacLocation.first;
    int y = pacLocation.second;
    std::set<Coordinate> msPacMoves;

    if (x + 1 < columns && wallSet.find(std::make_pair(x + 1, y)) == wallSet.end()) {
        msPacMoves.insert(std::make_pair(x + 1, y));
    }
    if (x - 1 >= 0 && wallSet.find(std::make_pair(x - 1, y)) == wallSet.end()) {
        msPacMoves.insert(std::make_pair(x - 1, y));
    }
    if (y + 1 < rows && wallSet.find(std::make_pair(x, y + 1)) == wallSet.end()) {
        msPacMoves.insert(std::make_pair(x, y + 1));
    }
    if (y - 1 >= 0 && wallSet.find(std::make_pair(x, y - 1)) == wallSet.end()) {
        msPacMoves.insert(std::make_pair(x, y - 1));
    }

    // Option to not move at all
    msPacMoves.insert(std::make_pair(x, y));
    
    return msPacMoves;
}

// Gets the valid moves for all of the ghosts and returns as a map to the controller that is calling
std::map<int, std::set<Coordinate>> Board::getGhostMoves() const {
    std::map<int, std::set<Coordinate>> ghostMoves;
    for (int i = 1; i <= NUM_GHOSTS; i++) {
        int x = ghostLocations.at(i).first;
        int y = ghostLocations.at(i).second;
        std::set<Coordinate> oneGhostMoves;

        if (x + 1 < columns && wallSet.find(std::make_pair(x + 1, y)) == wallSet.end()) {
            oneGhostMoves.insert(std::make_pair(x + 1, y));
        }
        if (x - 1 >= 0 && wallSet.find(std::make_pair(x - 1, y)) == wallSet.end()) {
            oneGhostMoves.insert(std::make_pair(x - 1, y));
        }
        if (y + 1 < rows && wallSet.find(std::make_pair(x, y + 1)) == wallSet.end()) {
            oneGhostMoves.insert(std::make_pair(x, y + 1));
        }
        if (y - 1 >= 0 && wallSet.find(std::make_pair(x, y - 1)) == wallSet.end()) {
            oneGhostMoves.insert(std::make_pair(x, y - 1));
        }

        ghostMoves[i] = oneGhostMoves;
    }

    return ghostMoves;
}

// Overloaded operator= for the Board class
const Board& Board::operator=(const Board& rhs) {
    if (this != &rhs) {
        if (grid != NULL) {
            for (int i = 0; i < rows; i++) {
                delete [] grid[i];
            }
            delete [] grid;
        }

        columns = rhs.getColumns();
        rows = rhs.getRows();

        grid = new Cell*[rows];
        for (int i = 0; i < rows; i++) {
            grid[i] = new Cell[columns];
        }
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                grid[i][j] = rhs.grid[i][j];
            }
        }

        // Update locations 
        pacLocation = rhs.getPacLocation();
        ghostLocations = rhs.getGhostLocations();
        wallSet = rhs.getWallSet();
        pillSet = rhs.getPillSet();
        fruitLocation = rhs.getFruitLocation();

        // Update global game variables
        score = rhs.getScore();
        fruitValue = rhs.getfruitValue();
        fruitConsumed = rhs.getFruitConsumed();
        timeRemaining = rhs.getTimeRemaining();
        totalTime = rhs.getTotalTime();
        pillsConsumed = rhs.getPillsConsumed();
        totalPills  = rhs.getTotalPills();
    }

    return *this;
}

std::ostream& operator<<(std::ostream& out, const Cell& c) {
    std::string display;
    if (c.isPac) { display = "P"; }
    else if (c.isGhost) { display = std::to_string(c.ghostCount); }
    else if (c.isWall) { display = "*"; }
    else if (c.isPill) { display = "·"; }
    else if (c.isFruit) { display = "F"; }
    else { display = " "; }

    out << display;

    return out;
}

std::ostream& operator<<(std::ostream& out, const Coordinate& c) {
    out << "(" << c.first << ", " << c.second << ")";

    return out;
}

std::ostream& operator<<(std::ostream& out, const std::set<Coordinate> setToPrint) {
    int i = 1;
    out << "\t";
    for (auto it = setToPrint.begin(); it != setToPrint.end(); it++) {
        if (i % 6 == 0) { out << std::endl << "\t"; }
        out << std::setw(4) << *it << "\t";
        i++;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const Board& b) {
    out << b.columns << std::endl << b.rows << std::endl;

    ////////////////////////////// FOR DEBUGGING ///////////////////////////////
   for (int i = b.rows; i > 0; i--) {
        for (int j = 0; j < b.columns; j++) {
            out << "|" << b.grid[i - 1][j];
        }

        out << "|" << std::endl;
    }
    out << "Score: " << b.getScore() << std::endl;
    out << "Time Remaining: " << b.getTimeRemaining() << std::endl;
    out << "Number of walls: " << b.getWallSet().size() << std::endl;
    out << "Number of pills: " << b.getPillSet().size() << std::endl;
    out << "Coordinates of Ms. Pac-Man: " << b.pacLocation << std::endl;
    out << "Coordinates of ghosts: " << b.ghostLocations.at(1) << " " << b.ghostLocations.at(2)
        << " " << b.ghostLocations.at(3) << std::endl;
    /*out << "Wall coordinates: " << std::endl;
    out << b.getWallSet() << std::endl;
    out << "Pill coordinates:" << std::endl;
    out << b.getPillSet() << std::endl;
    out << "Empty coordinates:" << std::endl;
    out << b.getEmptySet() << std::endl;*/
    out << "Fruit Location: " << b.fruitLocation << std::endl;
    //////////////////////////////////////////////////////////////////////////

    return out;
}