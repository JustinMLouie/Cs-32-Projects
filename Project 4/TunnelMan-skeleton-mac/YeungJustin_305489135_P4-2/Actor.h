#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

enum State {
    stable = 1,
    waiting = 2,
    falling = 3,
    dead = 0,
    pickableByTunnelMan = 4,
    pickableByProtesters = 5
};

struct Node {
    Node(int X, int Y) : x(X), y(Y) {}
    Node() {}
    int x;
    int y;
};


// ACTOR CLASS --------------------------------------------------------

class Actor : public GraphObject {
public:
    // Constructors
    Actor(int imageID, int startX, int startY,
          Direction dir, double size, unsigned int depth);
    Actor(StudentWorld* s, int imageID, int startX, int startY,
          Direction dir, double size, unsigned int depth);
    
    StudentWorld* getWorld() { return p; }
    
    // Destructor
    virtual ~Actor() {};
    
    // Other functions
    virtual void doSomething() {};
    virtual bool decreaseHealth(int v) { return false; }
    virtual bool canBeSonarRevealed() const { return false; }
    virtual bool canBeAnnoyed() const { return false; }
    virtual bool canPickItems() const { return false; }
    virtual void pickGold() {}
    virtual bool canActorsPassThrough() const { return true; }
    void setState(State state) { s = state; }
    virtual int getHealth() const { return 0; }
    State getState() { return s; }
    
    // Movement
    void move(Direction);
    void moveForward();
    
    // Checking feasibility
    bool checkBlock(Direction, int, int);
    bool checkBound(Direction dir, int x, int y);
    bool checkEarth(Direction, int, int);
    bool checkBoulder(Direction, int, int);
    
    // Tick handling
    void setCount(int c) { count = c; }
    bool Timeup() {
        return count <= 0;
    }
    void decrementCount() { count--; }
    
private:
    State s;
    StudentWorld* p;
    int count;
};


// EARTH CLASS --------------------------------------------------------

class Earth: public Actor {
public:
    Earth(int startX, int startY);
    // TODO: add any necessary methods
};

// HUMAN CLASS --------------------------------------------------------
class Human : public Actor {
public:
    Human(StudentWorld* world, int startX, int startY, Direction startDir,
          int imageID, unsigned int hitPoints);
    
    virtual int getHealth() const override { return m_hitpoints; }
    void setHealth(int h) { m_hitpoints = h; }
    void removeHuman() { m_hitpoints = 0; }

    virtual bool decreaseHealth(int v) override { m_hitpoints -= v; return true; }
    virtual bool canBeAnnoyed() const override { return true; }
    bool canPickThingsUp() const { return true; }
    
private:
    int m_hitpoints;
};

class TunnelMan : public Human {
public:
    TunnelMan(StudentWorld* world);
    
    void doSomething() override;
    void moveAndDig(Direction dir);
    
    void fireSquirt();
    void useSonar();
    void dropGold();
    virtual void pickGold() override { m_gold++; }

    int getSonar() const { return m_sonar; }
    int getSquirts() const { return m_water; }
    int getGold() const { return m_gold; }
    
    void increaseSonar() { m_sonar++; }
    void increaseSquirts() { m_water += 5; }
    
    int getNextX(Direction dir) const;
    int getNextY(Direction dir) const;
    
    virtual bool decreaseHealth(int v);
    
private:
    int m_water = 5;
    int m_sonar = 1;
    int m_gold = 0;
    
    // TODO: Add any necessary methods
};



// ALL PROTESTERS CLASSES --------------------------------------------------------

class Protester : public Human {
public:
    Protester(StudentWorld* p, int imageID, unsigned int health);
    
//    virtual void doSomething();
    virtual bool decreaseHealth(int v) override;
//    virtual void pickGold() = 0;
    
    void resetRestingTicks(int v) { restingTickCounts = v; }
    bool breadthFirstSearch(int startX, int startY, int endX, int endY, Direction& finalDir, int& legalSteps);
    void initialBehaviour(bool& hasFinishedResting);
    void actionBehaviour();
    void incrementShoutTickCount() { shoutTickCount++; }
    void incrementPerpTurn() { perpTurn++; }
    
private:
    int restingTickCounts;
    int numSquaresToMoveInCurrentDirection;
    int ticksToWaitBetweenMoves;
    int shoutTickCount;
    int perpTurn;
    
    void shoutAtTunnelMan();
    bool visited[64][64]; //"visited" table
    Node* prev[64][64];   //"previous" table
    Node nodes[64][64];   // Nodes table
};

class RegularProtester : public Protester {
public:
    RegularProtester(StudentWorld* p);
    
    virtual void pickGold() override;
    
    virtual bool decreaseHealth(int v) override;
    virtual void doSomething() override;
};

class HardcoreProtester : public Protester {
public:
    HardcoreProtester(StudentWorld* p);
    
    virtual void pickGold() override;
    
    virtual bool decreaseHealth(int v) override;
    virtual void doSomething() override;
    
private:
    int M;
};



// OTHER CLASSES --------------------------------------------------------
class Boulders : public Actor {
public:
    Boulders(StudentWorld* pointer, int startX, int startY);
    virtual void doSomething();
    virtual bool canActorsPassThrough() const { return false; }
    
private:
    int boulderDelay;
};

class Squirt : public Actor {
public:
    Squirt(StudentWorld* pointer, int x, int y, Direction d);
    virtual void doSomething();
};

class Interactable : public Actor {
public:
    Interactable(StudentWorld* world, int startX, int startY,
                 int imageID, bool visible, State s)
    : Actor(world, imageID, startX, startY, right, 1.0, 2) {
        setVisible(visible);
        setState(s);
    }
    
    double distToTunnelMan() {
        int deltaX = getWorld()->getTunnelManPosX() - getX();
        int deltaY = getWorld()->getTunnelManPosY() - getY();
        return sqrt(deltaX * deltaX + deltaY * deltaY);
    }
};

class HiddenObjects : public Interactable {
public:
    HiddenObjects(StudentWorld* world, int x, int y, int imageID, bool visible, State s)
    : Interactable(world, x, y, imageID, visible, s) {}
    
    bool wasDiscovered();
    virtual bool canBeSonarRevealed() const override { return true; }
    
    bool isHidden() { return m_isHidden; }
    void setStatus(bool status) { m_isHidden = status; }
    
private:
    bool m_isHidden;
};

class Barrel : public HiddenObjects {
public:
    Barrel(StudentWorld* p, int x, int y);
    virtual void doSomething();
};

class Gold : public HiddenObjects {
public:
    Gold(StudentWorld* p, int x, int y, State s, bool b);
    virtual void doSomething();
};

class Goodies : public Interactable {
public:
    Goodies(StudentWorld* p, int x, int y, int imageID);
    virtual void doSomething();
};

class SonarKit : public Goodies {
public:
    SonarKit(StudentWorld* p, int x, int y);
    virtual void doSomething();
};

class WaterPool : public Goodies {
public:
    WaterPool(StudentWorld* p, int x, int y);
    virtual void doSomething();
};




#endif // ACTOR_H_
