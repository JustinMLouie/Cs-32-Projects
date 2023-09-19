#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_


#include "GameWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
/*
 Inherited functions from GameWorld:
 
 unsigned int getLives() const;
 void decLives();
 void incLives();
 unsigned int getScore() const;
 unsigned int getLevel() const;
 void increaseScore(unsigned int howMuch);
 void setGameStatText(string text);
 bool getKey(int& value); void playSound(int soundID);
 
 getLives() can be used to determine how many lives the player has left.
 decLives() reduces the number of player lives by one.
 incLives() increases the number of player lives by one.
 getScore() can be used to determine the player’s current score
 getLevel() can be used to determine the player’s current level number.
 
 */

class Earth;
class TunnelMan;
class Boulders;
class Actor;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();
    
    virtual int init();
    
    virtual int move();
    
    virtual void cleanUp();
    
    Earth* getGameboard(int x, int y) const {return gameboard[x][y];}
    void RemoveEarth(int x, int y);
    void CreateSquirt(int x, int y, GraphObject::Direction dir);
    int getTunnelManPosX();
    int getTunnelManPosY();
    int getTunnelManHealth();
    void decreaseBarrels();
    void decreaseNumProtesters() { numProtesters--;} ;
    void increaseTunnelManGold();
    void increaseSonar();
    void increaseSquirts();
    void hitTunnelMan();
    bool checkBoulder(int, int, GraphObject::Direction);
    void revealHiddenObjects(int, int);
    void dropGold(int, int);
    bool goldPickedByProtesters(int x, int y);
    bool annoyProtesters(int x, int y);
    bool checkForBoulders(int x1, int x2, int y1, int y2);
    bool checkEarthArea(int, int, int, int);
    void hitProtesters(int, int);
    bool checkIfEmpty(int x, int y);
    
private:
    TunnelMan* p_tunnelman;
    Actor* actor;
    std::list<Actor*> actorsList;
    Earth* gameboard[64][64];
    void setDisplayText();
    int m_barrels;
    int ticksToAddProtesters;
    int numProtesters;
    int G; // chance of adding goodies
    int T; // ticks of adding protesters
    int P; // number of maximum protesters
    void addProtester();
};

#endif // STUDENTWORLD_H_
