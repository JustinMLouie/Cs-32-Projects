#include "Actor.h"
#include "StudentWorld.h"

#include <math.h>
#include <algorithm>
#include <queue>
#include <stack>

using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp


// ACTOR FUNCTIONS --------------------------------------------------------

Actor::Actor(int imageID, int startX, int startY, Direction dir,
             double size, unsigned int depth):GraphObject(imageID,startX, startY, dir, size, depth) {
    setVisible(true);
}

Actor::Actor(StudentWorld* s,int imageID, int startX, int startY,
             Direction dir = right, double size = 1.0,
             unsigned int depth = 0): GraphObject(imageID,
             startX, startY, dir, size, depth) {
    setVisible(true);
    p=s;
}

void Actor::move(Direction d) {
    // move in a given direction
    int newX = getX();
    int newY = getY();
    
    // update new coordinates
    if (d == down) {
        newY--;
    } else if (d == up) {
        newY++;
    } else if (d == left) {
        newX--;
    } else if (d == right) {
        newX++;
    }
    
    // move character if nothing in they way
    if (!checkBlock(d, newX, newY)) {
        setDirection(d);
        moveTo(newX, newY);
    }
}

void Actor::moveForward() {
    // move using the current direction
    Direction d = getDirection();
    int newX = getX();
    int newY = getY();
    
    if (d == down) {
        newY--;
    } else if (d == up) {
        newY++;
    } else if (d == left) {
        newX--;
    } else if (d == right) {
        newX++;
    }
    
    // move character if nothing in they way
    if (!checkBlock(d, newX, newY)) {
        moveTo(newX, newY);
    }
}


bool Actor::checkBlock(Direction dir, int x, int y) {
    // checks if earth, boulders, or boundaries are in the way
    if (checkBound(dir, x, y) || checkEarth(dir, x, y) || checkBoulder(dir, x, y))  {
        return true; // cannot move, already something there
    } else {
        return false; // can move there
    }
}

bool Actor::checkBound(Direction dir, int x, int y) {
    // checks if actor is at a boundary and the
    // next move in that direction would go out of bounds
    if (dir == right && x > 60) {
        return true;
    }
    if (dir == left && x < 0) {
        return true;
    }
    if (dir == up && y > 60) {
        return true;
    }
    if (dir == down && y < 0) {
        return true;
    }
    // within bounds
    return false;
}

bool Actor::checkEarth(Direction dir, int x, int y) {
    // checks if earth is in the corresponding direction
    if (dir==right) {
        for (int j = 0; j < 4 ;j++) {
            if (getWorld()->getGameboard(x+4, y+j)!=nullptr)
                return true;
        }
        return false;
    } else if (dir == left) {
        for (int j = 0; j < 4; j++) {
            if (getWorld()->getGameboard(x-1, y+j) != nullptr) {
                return true;
            }
        }
        return false;
    } else if (dir == up) {
        for (int i = 0; i < 4; i++) {
            if (getWorld()->getGameboard(x+i, y+4) != nullptr) {
                return true;
            }
        }
        return false;
    } else {
        for (int i = 0; i < 4; i++) {
            if (getWorld()->getGameboard(x+i, y-1)!=nullptr) {
                return true;
            }
        }
        return false;
    }
}

bool Actor::checkBoulder(Direction dir, int x, int y) {
    // checks if boulder is in corresponding direction
    if (dir == left) {
        if (getWorld()->checkBoulder(x + 1, y, left)) {
            return true;
        } else {
            return false;
        }
    }
    
    if (dir == right) {
        if (getWorld()->checkBoulder(x - 1, y, right)) {
            return true;
        } else {
            return false;
        }
    }
    
    if (dir == up) {
        if (getWorld()->checkBoulder(x, y + 1, up)) {
            return true;
        } else {
            return false;
        }
    }
    
    if (dir == down) {
        if (getWorld()->checkBoulder(x, y - 1, down)) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

// TUNNELMAN FUNCTIONS --------------------------------------------------------
// Used for tunnelman and protesters --> regularprotesters and hardcoreprotestors

Human::Human(StudentWorld* world, int startX, int startY, Direction startDir,
      int imageID, unsigned int hitPoints): Actor (world, imageID, startX, startY, startDir, 1.0, 0) {
          m_hitpoints = hitPoints;
}

// TUNNELMAN FUNCTIONS --------------------------------------------------------

TunnelMan::TunnelMan(StudentWorld *world): Human(world, 30, 60, right, TID_PLAYER, 10) {
    m_water = 5;
    m_sonar = 1;
    m_gold = 0;
}

void TunnelMan::doSomething() {
    // if health is all out, kill tunnelman
    if (getHealth() <= 0) {
        setState(dead);
        return;
    }
    
    int keyPressed;
    
    // not a valid key
    if (!getWorld()->getKey(keyPressed)) {
        return;
    }
    
    // valid action entered
    switch (keyPressed) {
            // movements
        case KEY_PRESS_LEFT:
            if (!checkBoulder(left, getX(), getY())) {
                moveAndDig(left);
            }
            break;
        case KEY_PRESS_RIGHT:
            if (!checkBoulder(right, getX(), getY())) {
                moveAndDig(right);
            }
            break;
            
        case KEY_PRESS_UP:
            if (!checkBoulder(up, getX(), getY())) {
                moveAndDig(up);
            }
            break;
        case KEY_PRESS_DOWN:
            if (!checkBoulder(down, getX(), getY())) {
                moveAndDig(down);
            }
            break;
            
            // reset round, lose a life
        case KEY_PRESS_ESCAPE:
            removeHuman();
            break;
            
            // Item related actions
        case KEY_PRESS_SPACE:
            fireSquirt();
            break;
        case 'z':
            useSonar();
            break ;
        case 'Z':
            useSonar();
            break ;
        case KEY_PRESS_TAB:
            dropGold();
            break;
    }
}

void TunnelMan::moveAndDig(Direction dir) {
    // moves tunnelman and removes all corresponding earth
    
    bool soundHasPlayed = false;
    
    // updates direction
    if (getDirection() != dir) {
        setDirection(dir);
        return;
    }
     
    // Calculate next position
    int nextX = getNextX(dir);
    int nextY = getNextY(dir);
    
    // Check if next position is within bounds
    if (!checkBound(dir, nextX, nextY)) {
        // checks if not a boulder
        if (!getWorld()->checkBoulder(getX(), getY(), getDirection())) {
            moveTo(nextX, nextY); // actually move tunnelman
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    int x = nextX + i;
                    int y = nextY + j;
                    // check each square in movement direction to remove earth
                    if (getWorld()->getGameboard(x, y) != nullptr) {
                        getWorld()->RemoveEarth(x, y);
                        if (soundHasPlayed == false) {
                            // only play sound once
                            getWorld()->playSound(SOUND_DIG);
                        }
                        soundHasPlayed = true;
                    }
                }
            }
            
        }
    }
}

void TunnelMan::fireSquirt() {
    // fire squirt in corresponding dir
    if (m_water > 0) {
        m_water--;
        getWorld()->playSound(SOUND_PLAYER_SQUIRT);
        getWorld()->CreateSquirt(getNextX(getDirection()), getNextY(getDirection()), getDirection());
    }
}

void TunnelMan::useSonar() {
    // use sonar
    if (m_sonar > 0) {
        m_sonar--;
        getWorld()->revealHiddenObjects(getX(), getY());
        getWorld()->playSound(SOUND_SONAR);
    }
}

bool TunnelMan::decreaseHealth (int v) {
    // tunnelman has been hurt
    Human::decreaseHealth(v);
    if (getHealth() <= 0) {
        setState(dead);
        getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
    }
    return true;
}

void TunnelMan::dropGold() {
    // drop gold and increment gold count
    if (m_gold > 0) {
        m_gold--;
        getWorld()->dropGold(getX(), getY());
    }
}

int TunnelMan::getNextX(Direction dir) const {
    // gets x position in front of player
    if (dir == left) {
        return getX() - 1;
    } else if (dir == right) {
        return getX() + 1;
    } else {
        return getX();
    }
}

int TunnelMan::getNextY(Direction dir) const {
    // gets y position in front of player
    if (dir == up) return getY() + 1;
    if (dir == down) return getY() - 1;
    return getY();
}


// PROTESTER FUNCTIONS --------------------------------------------------------

Protester::Protester(StudentWorld *p, int imageID, unsigned int health): Human(p, 60, 60, left, imageID, health ) {
    setState(waiting);//ie, resting state
    setVisible(true);
    shoutTickCount = 15;
    numSquaresToMoveInCurrentDirection = rand() % (60 - 8 + 1) + 8;
    int ticksBetweenMoves = 3 - getWorld()->getLevel() / 4;
    ticksToWaitBetweenMoves = max(0, ticksBetweenMoves);
    restingTickCounts = ticksToWaitBetweenMoves;
    perpTurn = 0;
}

void Protester::initialBehaviour(bool& hasFinishedResting) {
    // encompasses all of Protester's doSomething until M is considered in hardcore
    Direction bfsDir = none;
    
    // protester is dead, remove from board
    if (getState() == dead) {
        return;
    }

    // protester is waiting between moves
    if (getState() == waiting) {
        if (restingTickCounts > 0) {
            restingTickCounts--;
            hasFinishedResting = false;
            return;
        } else {
            hasFinishedResting = true;
            restingTickCounts = ticksToWaitBetweenMoves;
        }
    }

    // leave the gameboard state
    if (getState() == falling) {
        if (getX() == 60 && getY() == 60) {
            getWorld()->decreaseNumProtesters();
            setState(dead);
            return;
        }
        
        int numSteps;
        bool pathFound = breadthFirstSearch(60, 60, getX(), getY(), bfsDir, numSteps);
        
        if (restingTickCounts > 0) {
            restingTickCounts--;
            hasFinishedResting = false;
            return;
        } else {
            hasFinishedResting = true;
            restingTickCounts = ticksToWaitBetweenMoves;
        }
        
        // move protester
        if (pathFound) {
            setDirection(bfsDir);
            if (bfsDir == left) {
                moveTo(getX() - 1, getY());
            }
            
            if (bfsDir == right) {
                moveTo(getX() + 1, getY());
            }
            
            if (bfsDir == up) {
                moveTo(getX(), getY() + 1);
            }
            
            if (bfsDir == down) {
                moveTo(getX(), getY() - 1);
            }
            return;
        }
        return;
    }

    // shout at tunnelman
    if (shoutTickCount >= 15) {
        int dx = getX() - getWorld()->getTunnelManPosX();
        int dy = getY() - getWorld()->getTunnelManPosY();

        if (abs(dx) <= 4 && abs(dy) <= 4) {
            Direction facingDir = getDirection();
//            cout << "direction: " << facingDir << endl;
//            cout << dx << endl;
//            cout << dy << endl;
            
            
            if (facingDir == left && dx >= 0 && abs(dy) < 4) {
                shoutAtTunnelMan();
                shoutTickCount = 0;
            }

            if (facingDir == right && dx <= 0 && abs(dy) < 4) {
                shoutAtTunnelMan();
                shoutTickCount = 0;
            }
            
            if (facingDir == up && dy <= 0 && abs(dx) < 4) {
                shoutAtTunnelMan();
                shoutTickCount = 0;
            }
            
            if (facingDir == down && dy >= 0 && abs(dx) < 4) {
                shoutAtTunnelMan();
                shoutTickCount = 0;
            }
        }
    }
    return;
}

void Protester::actionBehaviour() {
    // encompasses all of Protester's doSomething after M is considered in hardcore
    bool hasDirectPath=false;
    Direction newDir = none;
    
    // if close enough to player, chase
    if (sqrt(pow(getWorld()->getTunnelManPosX() - getX(), 2) + pow(getWorld()->getTunnelManPosY() - getY(), 2)) > 4) {
        // The character and TunnelMan are within a 4 unit radius of each other
        int xDis = abs(getX() - getWorld()->getTunnelManPosX());
        if(xDis < 4) { // vertical alignment
            if (getY() > (getWorld()->getTunnelManPosY() + 4)) { // protester above tunnelman
                if (!getWorld()->checkEarthArea(getX(), getX() + 4, getWorld()->getTunnelManPosY()+4, getY())) {
                    if (!getWorld()->checkForBoulders(getX() - 4, getX() + 4, getWorld()->getTunnelManPosY() + 1, getY() - 1)) {
                        hasDirectPath = true;
                    }
                }
                newDir = down;
            }
            
            if (getY() < (getWorld()->getTunnelManPosY() - 4)) { // protester below
                if (!getWorld()->checkEarthArea(getX(), getX() + 4, getY() + 4, getWorld()->getTunnelManPosY())) {
                    if (!getWorld()->checkForBoulders(getX() - 4, getX() + 4, getY() + 1,getWorld()->getTunnelManPosY() - 1))
                        hasDirectPath = true;
                }
                newDir = up;
            }
        }
        int yDis = abs(getY() - getWorld()->getTunnelManPosY());
        if (yDis < 4) { // horizontal alignment
            if (getX() > (getWorld()->getTunnelManPosX() + 4)) { // protester to the right
                if (!getWorld()->checkEarthArea(getWorld()->getTunnelManPosX() + 4, getX(), getY(), getY() + 4)) {
                    if (!getWorld()->checkForBoulders(getWorld()->getTunnelManPosX() + 1, getX() - 1, getY() - 4, getY() + 4)) {
                        hasDirectPath = true;
                    }
                }
                newDir = left;
            }
            
            if (getX() < (getWorld()->getTunnelManPosX() - 4)) { // protester to the left
                if (!getWorld()->checkEarthArea(getX() + 4, getWorld()->getTunnelManPosX(), getY(), getY() + 4)) {
                    if (!getWorld()->checkForBoulders(getX() + 1, getWorld()->getTunnelManPosX() - 1, getY() - 4, getY() + 4)) {
                        hasDirectPath = true;
                    }
                }
                newDir = right;
            }
        }
    }
    
    // actually moves protester forward
    if (hasDirectPath == true) {
        setDirection(newDir);
        moveForward();
        numSquaresToMoveInCurrentDirection = 0;
        shoutTickCount++;
        perpTurn++;
        return ;
    }
    
    numSquaresToMoveInCurrentDirection--;
    
    // new direction to move
    if (numSquaresToMoveInCurrentDirection <= 0) {
        int randomDirNum = rand() % 4 + 1; // shift these to have the same vals as enum Direction
        Direction randDir = none;
        bool blocked = true;
        
        // select non-blocked path randomly
        while (blocked) {
            if (randomDirNum == 1) {
                if (!checkBlock(up, getX(), getY() + 1)) {
                    randDir = up;
                    blocked = false;
                } else {
                    randomDirNum = rand() % 4 + 1;
                }
            } else if (randomDirNum == 2) {
                if (!checkBlock(down, getX(), getY() - 1)) {
                    randDir = down;
                    blocked = false;
                } else {
                    randomDirNum = rand() % 4 + 1;
                }
            } else if (randomDirNum == 3) {
                if (!checkBlock(left, getX() - 1, getY())) {
                    randDir = left;
                    blocked = false;
                } else {
                    randomDirNum = rand() % 4 + 1;
                }
            } else if (randomDirNum == 4) {
                if (!checkBlock(right, getX() + 1, getY())) {
                    randDir = right;
                    blocked = false;
                } else {
                    randomDirNum = rand() % 4 + 1;
                }
            } else {
                randomDirNum = rand() % 4 + 1;
            }
        }
        
        setDirection(randDir);
        
        numSquaresToMoveInCurrentDirection = rand() % (60 - 8 + 1) + 8;
        
    } else { // numSquaresToMoveInCurrentDirection has not reached 0
        if (perpTurn >= 200) {
            /*
             Is sitting at an intersection where it could turn and move at least one
             square in a perpendicular direction from its currently facing direction
             (e.g., it is A Regular Protester cannot move to a location that is within
             a radius of 3 (<= 3.0) units of a Boulder. currently facing left, and is
             at a junction where it could turn and move one step either upward/downward
             without being blocked by Earth or a Boulder), and b. The Regular Protester
             hasn’t made a perpendicular turn in the last 200 non-resting ticks.
             */
            
            Direction dir = getDirection();
            if (dir == right || dir == left) { // facing left/right
                if (!checkBlock(up, getX(), getY() + 1) || !checkBlock(down, getX(), getY()) - 1) {
                    if (!checkBlock(up, getX(), getY() + 1)) { // up is not blocked
                        setDirection(up);
                    } else { // if up is blocked
                        if (!checkBlock(down, getX(), getY() - 1)) { //down is not blocked
                            setDirection(down);
                        } else {
                            int b = rand() % 2;
                            if (b==0) {
                                setDirection(up);
                            } else {
                                setDirection(down);
                            }
                        }
                    }
                    numSquaresToMoveInCurrentDirection = rand() % (60 - 8 + 1) + 8;
                    perpTurn = 0;
                }
            } else { // facing up/down
                if (!checkBlock(left, getX() - 1, getY()) || !checkBlock(right, getX() + 1, getY())) {
                    if (!checkBlock(left, getX(), getY())) { // left is not blocked
                        setDirection(left);
                    } else { // if left is blocked
                        if (!checkBlock(right, getX() + 1, getY())) { // right is not blocked
                            setDirection(right);
                        } else {
                            int b = rand() % 2;
                            if (b == 0) {
                                setDirection(left);
                            } else  {
                                setDirection(right);
                            }
                        }
                    }
                    numSquaresToMoveInCurrentDirection = rand() % (60 - 8 + 1) + 8;
                    perpTurn = 0;
                }
            }
        }
        
        /*
         If the Regular Protester is for some reason blocked from
         taking a step in its currently facing direction, it will
         set the numSquaresToMoveInCurrentDirection to zero, resulting in a
         new direction being chosen during the Regular Protester’s next non-resting
         tick (but not the current tick – the Regular Protester
         must do nothing during the current tick).
         */
        
        // blocked
        Direction tempDir = getDirection();

        if ((tempDir == left && checkBlock(left, getX() - 1, getY())) || (tempDir == right && checkBlock(right, getX() + 1, getY())) || (tempDir == up && checkBlock(up, getX(), getY() + 1)) || (tempDir == down && checkBlock(down, getX(), getY() - 1))) {
            numSquaresToMoveInCurrentDirection = 0;
            shoutTickCount++;
            perpTurn++;
            return;
        }
        
        /*
         Finally, the Regular Protester will then attempt to take one step in its
         currently facing direction (which, by the way, might have just been changed
         by one of the last few steps above).
         */
        
        moveForward();//take one step in its currently facing direction
        shoutTickCount++;
        perpTurn++;
        return;
    }
    return;
}

void Protester::shoutAtTunnelMan() {
    // protester shouts at tunnelman
    getWorld()->playSound(SOUND_PROTESTER_YELL);
    getWorld()->hitTunnelMan();
    return;
}

bool Protester::decreaseHealth(int v) {
    // protester has been hit by something
    Human::decreaseHealth(v); // use human's decreasehealth for actual hitpoint mods
    
    if (getHealth() <= 0) {
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
        restingTickCounts = 0;
        setState(falling);
        if (v == 100) { // hit by boulder
            getWorld()->increaseScore(500);
        }
    } else {
        // stun protesters
        getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
        setState(waiting);
        int N = std::max(50, static_cast<int>(100 - getWorld()->getLevel() * 10));
        restingTickCounts = N;
    }
    return true;
}

bool Protester::breadthFirstSearch(int startX, int startY, int endX, int endY, Direction& finalDir, int& legalSteps) {
    // BFS using queues to send protesters to a specified location
    
    // initialize array tracking movements
    int stepsArray[64][64];
    bool pathFound = false;
    for (int i = 0; i != 64; i++) {
        for(int j = 0; j != 64; j++){
            stepsArray[i][j] = 9999;
        }
    }
    
    // queue to hold actual decisions
    std::queue<Node> exitPath;
    
    exitPath.push(Node(startX, startY));
    stepsArray[startX][startY] = 0;
    
    int currentStepCount = 0;
    int minSteps;
    
    while (!exitPath.empty()) { // until a path has been found
        int currentX = exitPath.front().x;
        int currentY = exitPath.front().y;
        exitPath.pop();
        
        if (currentX == endX && currentY == endY) {
            pathFound = true;
            break;
        }
        
        currentStepCount = stepsArray[currentX][currentY];
        currentStepCount++;
        
        // give each step a number
        if (!checkBlock(right, currentX, currentY) && stepsArray[currentX + 1][currentY] == 9999) {
            exitPath.push(Node(currentX + 1, currentY));
            stepsArray[currentX + 1][currentY] = currentStepCount;
            
        }
        
        if (!checkBlock(left, currentX, currentY) && stepsArray[currentX - 1][currentY] == 9999) {
            exitPath.push(Node(currentX - 1, currentY));
            stepsArray[currentX - 1][currentY] = currentStepCount;
            
        }
        
        if (!checkBlock(up, currentX, currentY) && stepsArray[currentX][currentY + 1] == 9999) {
            exitPath.push(Node(currentX, currentY + 1));
            stepsArray[currentX][currentY + 1] = currentStepCount;
            
        }
        
        if (!checkBlock(down, currentX, currentY) && stepsArray[currentX][currentY - 1] == 9999) {
            exitPath.push(Node(currentX, currentY - 1));
            stepsArray[currentX][currentY - 1] = currentStepCount;
        }
        
        // decisions of how to turn and move
        if (endX == 0 && endY == 0) {
            minSteps = min(stepsArray[endX+1][endY], stepsArray[endX][endY+1]);
            if(minSteps == stepsArray[endX+1][endY]) {
                finalDir = right;
            } else {
                finalDir = up;
            }
            
        } else if (endX == 0) {
            minSteps = min(stepsArray[endX+1][endY], min(stepsArray[endX][endY+1], stepsArray[endX][endY-1]));
            if(minSteps == stepsArray[endX+1][endY]) {
                finalDir = right;
            } else if(minSteps == stepsArray[endX][endY + 1]) {
                finalDir = up;
            } else {
                finalDir = down;
            }
        } else if (endY ==0) {
            minSteps = min(stepsArray[endX][endY + 1], min(stepsArray[endX - 1][endY], stepsArray[endX + 1][endY]));
            if (minSteps == stepsArray[endX + 1][endY]) {
                finalDir = right;
            } else if (minSteps == stepsArray[endX][endY + 1]) {
                finalDir = up;
            } else {
                finalDir = left;
            }
        } else {
            minSteps = min(min(stepsArray[endX][endY + 1], stepsArray[endX][endY-1]), min(stepsArray[endX - 1][endY], stepsArray[endX + 1][endY]));
            if (minSteps == stepsArray[endX+1][endY]) {
                finalDir = right;
            } else if (minSteps == stepsArray[endX][endY + 1]) {
                finalDir = up;
            } else if (minSteps == stepsArray[endX-1][endY]) {
                finalDir = left;
            } else {
                finalDir = down;
            }
        }
        
    }
    
    legalSteps = minSteps;
    return pathFound;
}

RegularProtester::RegularProtester(StudentWorld *p): Protester(p, TID_PROTESTER, 5) { }

void RegularProtester::pickGold() {
    // protester picked up gold and leaves field
    getWorld()->increaseScore(25);
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    setState(dead);
}

bool RegularProtester::decreaseHealth(int v) {
    Protester::decreaseHealth(v); // utilize protester's decreaseHealth func
    
    if (getHealth() <= 0 && v == 2) {
        // protester killed, increase points
        getWorld()->increaseScore(100);
    }
    
    return true;
}

void RegularProtester::doSomething() {
    bool hasFinishedResting;
    initialBehaviour(hasFinishedResting);
    if (hasFinishedResting && getState() != falling) { // prevents players from moving every tick
        actionBehaviour();
    }
}

HardcoreProtester::HardcoreProtester(StudentWorld *p): Protester(p, TID_HARD_CORE_PROTESTER, 20) {
    // represents number of spaces away before player's location is auto revealed
    M = 16 + getWorld()->getLevel() * 2;
}

void HardcoreProtester::pickGold() {
    // increases score, prevents hardcore protester from moving for t ticks
    getWorld()->increaseScore(50);
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    setState(waiting);
    int t = 100 - getWorld()->getLevel() * 10;
    resetRestingTicks(std::max(50, t));
}

bool HardcoreProtester::decreaseHealth(int v) {
    Protester::decreaseHealth(v);  // utilize protester's decreaseHealth func
    if (getHealth() <= 0 && v == 2) {
        // protester killed, increase points
        getWorld()->increaseScore(250);
    }
    return true;
}

void HardcoreProtester::doSomething() {
    int numSteps;
    Direction bfsDir = none;
    bool hasFinishedResting;
    initialBehaviour(hasFinishedResting);
    
    if (hasFinishedResting) { // prevents players from moving every tick
        bool pathFound = breadthFirstSearch(getWorld()->getTunnelManPosX(), getWorld()->getTunnelManPosY(), getX(), getY(), bfsDir, numSteps);
        if (pathFound && numSteps <= M) { // chase tunnelman
            setDirection(bfsDir);
            incrementShoutTickCount();
            if (bfsDir == left) {
                moveTo(getX() - 1, getY());
            }
            
            if (bfsDir == right) {
                moveTo(getX() + 1, getY());
            }
            
            if (bfsDir == up) {
                moveTo(getX(), getY() + 1);
            }
            
            if (bfsDir == down) {
                moveTo(getX(), getY() - 1);
            }
            return;
        }
        
        if (getState() != falling) { // not returning to spawn spot
            // if not within proximity and is allowed to move this tick, move
            actionBehaviour();
        }
    }
}


// EARTH FUNCTIONS --------------------------------------------------------

Earth::Earth(int startX, int startY): Actor(TID_EARTH, startX, startY, right, 0.25, 3) {
    setVisible(true);
}


// SQUIRT FUNCTIONS --------------------------------------------------------

Squirt::Squirt(StudentWorld* pointer, int x, int y, Direction d): Actor(pointer, TID_WATER_SPURT, x, y, d, 1.0, 1) {
    setCount(4);
    setState(stable);
    if (checkBlock(d, x, y)) {
        setState(dead);
    }
}

void Squirt::doSomething() {
    // squirt water in front of player
    if (getWorld()->annoyProtesters(getX(), getY())) { // hit protester
        setState(dead);
        return;
    } else if (Timeup()) { // time ran out, disappears
        setState(dead);
        return;
    } else if (checkBlock(getDirection(), getX(), getY())) { // hit obstacle
        setState(dead);
        return;
    } else { // launch squirt forward
        move(getDirection());
        decrementCount();
        return;
    }
}


// ITEM FUNCTIONS --------------------------------------------------------

bool HiddenObjects::wasDiscovered() {
    // reveal items, set them to visible
    if (!isHidden()) {
        if (distToTunnelMan() <= 4.0) {
            setVisible(true);
            setStatus(true);
            return true;
        }
    }
    return false;
}


Barrel::Barrel(StudentWorld* p, int x, int y): HiddenObjects(p, x, y, TID_BARREL, false, stable) {
    setStatus(false);
}

void Barrel::doSomething() {
    if (getState() == dead) { // was collected
        return;
    } else if (wasDiscovered()) { // make it visible
        return;
    }
    // tunnelman has collected barrel
    if (distToTunnelMan() <= 3.0) {
        setState(dead);
        getWorld()->playSound(SOUND_FOUND_OIL);
        getWorld()->increaseScore(1000);
        getWorld()->decreaseBarrels();
        return;
    }
}

Boulders::Boulders(StudentWorld* pointer, int startx, int starty)
: Actor(pointer, TID_BOULDER, startx, starty, down, 1.0, 1) {
    setState(stable);
    setCount(30);
    boulderDelay = 1;
}

void Boulders::doSomething() {
    
    if (getState() == stable) { // stable
        if (checkEarth(down, getX(), getY())) {
            return;
        } else { // about to fall
            setState(waiting);
            return;
        }
    } else if (getState() == waiting) { // the wait before the rock falls
        if (!Timeup()) {
            decrementCount();
            return;
        } else {
            setState(falling);
            getWorld()->playSound(SOUND_FALLING_ROCK);
            return;
        }
    } else if (getState() == falling) { // the fall
        // boulder underneath
        if (getWorld()->checkForBoulders(getX()-3, getX() + 3, getY(), getY() + 4)) {
            setState(dead);
            return;
        }
//        if (checkBoulder(down, getX(), getY())) {
//            setState(dead);
//            return;
//        }
        // end of board
        if (checkBound(down, getX(), getY())) {
            setState(dead);
            return;
        }
        // earth
        if (checkEarth(down, getX(), getY())) {
            if (boulderDelay != 0) {
                moveForward();
                boulderDelay--;
                return;
            } else {
                setState(dead);
                return;
            }
        } else if (abs(getWorld()->getTunnelManPosY() - getY()) < 4) {
//            cout << "Diff boulder and player: " << endl;
//            cout << abs(getWorld()->getTunnelManPosX() - getX()) << endl;
            if (abs(getWorld()->getTunnelManPosX() - getX()) <= 4) {
                while (getWorld()->getTunnelManHealth() > 0) {
                    getWorld()->hitTunnelMan();
                }
            }
            moveTo(getX(), getY() - 1);
            return;
        } else {
            getWorld()->hitProtesters(getX(), getY());
            moveTo(getX(), getY() - 1);
            return;
        }
    }
    if (checkEarth(down, getX(), getY())) {
        return;
    } else {
        setState(waiting);
        return;
        
    }
}


Gold::Gold(StudentWorld* p, int x, int y, State s, bool b): HiddenObjects(p, x, y, TID_GOLD, b, s) {
    setVisible(b);
    if (s == pickableByProtesters) {
        setCount(100);
    }
}

void Gold::doSomething() {
    if (getState() == dead) { // remove from board
        return;
    }
    if (getState() == pickableByTunnelMan) {
        if (wasDiscovered()) {
            // tunnelMan near gold
            return;
        }
        if (distToTunnelMan() <= 3.0) {
            // tunnelMan picked up gold
            setState(dead);
            getWorld()->playSound(SOUND_GOT_GOODIE);
            getWorld()->increaseScore(10);
            getWorld()->increaseTunnelManGold();
            return;
        }
    } else if (getState() == pickableByProtesters) {
        if (Timeup()) {
            // sink back into Earth
            setState(dead);
        } else if (getWorld()->goldPickedByProtesters(getX(), getY())) {
            // protester picked up gold
            setState(dead);
            getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
        }
        decrementCount();
    }
}


WaterPool::WaterPool(StudentWorld* p, int x, int y): Goodies(p, x, y, TID_WATER_POOL) {}

void WaterPool::doSomething() {
    Goodies::doSomething();
    if (distToTunnelMan() <= 3.0) {
        // tunnelman refilled water
        getWorld()->increaseScore(100);
        getWorld()->increaseSquirts();
    }
}

Goodies::Goodies(StudentWorld* p, int x, int y, int imageID)
: Interactable(p, x, y, imageID, true, waiting) {
    setVisible(true);
    int n_t = 300 - 10 * (getWorld()->getLevel());
    setCount(max(100, n_t));
}

void Goodies::doSomething() {
    if (getState() == dead) return;
    if (Timeup()) {
        setState(dead);
        return;
    }
    if (distToTunnelMan() <= 3.0) {
        setState(dead);
        getWorld()->playSound(SOUND_GOT_GOODIE);
    }
    decrementCount();
}

SonarKit::SonarKit(StudentWorld* p, int x, int y): Goodies(p, x, y, TID_SONAR) { }

void SonarKit::doSomething() {
    Goodies::doSomething();
    if (distToTunnelMan() <= 3.0) {
        // tunnelman picks up sonar packet
        getWorld()->increaseScore(75);
        getWorld()->increaseSonar();
    }
}
