
#include "StudentWorld.h"
#include <string>

#include "Actor.h"
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iomanip>

using namespace std;

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp


GameWorld* createStudentWorld(string assetDir) {
    return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(std::string assetDir): GameWorld(assetDir) {
    
}

StudentWorld::~StudentWorld() { }

int StudentWorld::init() {
    // initialize and set up all the pieces on the board
    ticksToAddProtesters = 0;
    numProtesters = 0;
    G = getLevel() * 25 + 300;
    
    int n_t = 200 - getLevel();
    T = max(25, n_t);
    
    int n_p = 2 + getLevel() * 1.5;
    P = min(15, n_p);
    
    p_tunnelman = new TunnelMan(this);
    
    // create earth on board
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 60; j++) {
            gameboard[i][j] = new Earth(i, j);
        }
    }
    
    // open top row
    for (int i = 0; i < 64; i++) {
        for (int j = 60; j < 64; j++) {
            gameboard[i][j] = nullptr;
        }
    }
    
    // open center col
    for (int i = 30; i < 34; i++) {
        for (int j = 4; j < 60; j++) {
            delete gameboard[i][j];
            gameboard[i][j] = nullptr;
        }
    }
    
    // Adding Boulders
    int n_b = getLevel() / 2 + 2;
    int B = min(n_b, 9);
    for (int i = 0; i < B;) {
        int a = rand() % 61;
        int b = rand() % 37 + 20;
        if (a > 26 && a < 34 && b > 0) continue;
        bool canAdd = true;
        for (auto p : actorsList) {
            if (sqrt((p->getX() - a) * (p->getX() - a) + (p->getY() - b) * (p->getY() - b)) <= 6.0) {
                canAdd = false;
                break;
            }
        }
        if (canAdd) {
            Boulders* boulder = new Boulders(this, a, b);
            actorsList.push_back(boulder);
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (gameboard[a + i][b + j] != nullptr) {
                        delete gameboard[a + i][b + j];
                        gameboard[a + i][b + j] = nullptr;
                    }
                }
            }
            i++;
        }
    }
    
    // Adding Gold Nuggets
    int n_g = 5 - getLevel() / 2;
    int G = max(n_g, 2);
    for (int i = 0; i < G;) {
        int a = rand() % 61;
        int b = rand() % 57;
        if (a > 26 && a < 34 && b > 0) continue;
        bool canAdd = true;
        for (auto p : actorsList) {
            if (sqrt((p->getX() - a) * (p->getX() - a) + (p->getY() - b) * (p->getY() - b)) <= 6.0) {
                canAdd = false;
                break;
            }
        }
        if (canAdd) {
            Gold* gold = new Gold(this, a, b, pickableByTunnelMan, false);
            actorsList.push_back(gold);
            i++;
        }
    }
    
    // Adding Barrels of Oil
    int n_l = 2 + getLevel();
    int L = min(n_l, 21);
    m_barrels = L;
    for (int i = 0; i < L;) {
        int a = rand() % 61;
        int b = rand() % 57;
        if (a > 26 && a < 34 && b > 0) continue;
        bool canAdd = true;
        for (auto p : actorsList) {
            if (sqrt((p->getX() - a) * (p->getX() - a) + (p->getY() - b) * (p->getY() - b)) <= 6.0) {
                canAdd = false;
                break;
            }
        }
        if (canAdd) {
            Barrel* barrel = new Barrel(this, a, b);
            actorsList.push_back(barrel);
            i++;
        }
    }
    
    
    
    addProtester();
    
    return GWSTATUS_CONTINUE_GAME;
}


int StudentWorld::move() {
    setDisplayText();
    
    // player has died
    if (p_tunnelman->getHealth() <= 0) {
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    
    // player has won a level
    if (m_barrels == 0) {
        playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }
    
    // tunnelman moves for a tick
    p_tunnelman->doSomething();
    
    // have each actor do something for thier tick
    for (auto p : actorsList) {
        p->doSomething();
    }
    
    // spawn new items (sonar and water)
    if (rand() % G == 0) {
        if (rand() % 5 == 0) {
            SonarKit* sonar = new SonarKit(this, 0, 60);
            actorsList.push_back(sonar);
        } else {
            int x = rand() % 61;
            int y = rand() % 61;
            while (!checkIfEmpty(x, y)) {
                x = rand() % 61;
                y = rand() % 61;
            }
            WaterPool* water = new WaterPool(this, x, y);
            actorsList.push_back(water);
        }
    }
    
    // if not max protesters and enough time has passed, add a protester
    if (numProtesters < P && ticksToAddProtesters >= T) {
        addProtester();
    }
    
    // remove dead actors
    actorsList.remove_if([](Actor* a) {
        if (a->getState() == dead) {
            delete a;
            return true;
        }
        return false;
    });
    
    ticksToAddProtesters++;
    return GWSTATUS_CONTINUE_GAME;
}

// remove all actors to clean up
void StudentWorld::cleanUp() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 60; j++) {
            if (gameboard[i][j] != nullptr) {
                delete gameboard[i][j];
            }
        }
    }
    delete p_tunnelman;
    
    for (auto p : actorsList) {
        delete p;
    }
    actorsList.clear();
}

void StudentWorld::setDisplayText() {
    int level = getLevel();
    int lives = getLives();
    int health = p_tunnelman->getHealth() * 10;
    int squirts = p_tunnelman->getSquirts();
    int gold = p_tunnelman->getGold();
    int barrelsLeft = m_barrels;
    int sonar = p_tunnelman->getSonar();
    int score = getScore();
    
    // display string at top of the board
    string displayString =
    "Lvl: " + string(2 - to_string(level).length(), ' ') + to_string(level) +
    "  Lives: " + to_string(lives) +
    "  Hlth: " + string(3 - to_string(health).length(), ' ') + to_string(health) + "%" +
    "  Wtr: " + string(2 - to_string(squirts).length(), ' ') + to_string(squirts) +
    "  Gld: " + string(2 - to_string(gold).length(), ' ') + to_string(gold) +
    "  Oil Left: " + string(2 - to_string(barrelsLeft).length(), ' ') + to_string(barrelsLeft) +
    "  Sonar: " + string(2 - to_string(sonar).length(), ' ') + to_string(sonar) +
    "  Scr: " + string(6 - to_string(score).length(), '0') + to_string(score);
    
    setGameStatText(displayString);
}


void StudentWorld::RemoveEarth(int x, int y) {
    delete gameboard[x][y];
    gameboard[x][y] = nullptr;
}

void StudentWorld::CreateSquirt(int x, int y, GraphObject::Direction dir) {
    actor = new Squirt (this, x, y, dir);
    actorsList.push_back(actor);
}

int StudentWorld::getTunnelManPosX() {
    return p_tunnelman->getX();
}

int StudentWorld::getTunnelManPosY() {
    return p_tunnelman->getY();
}

int StudentWorld::getTunnelManHealth() {
    return p_tunnelman->getHealth();
}

void StudentWorld::decreaseBarrels() {
    m_barrels--;
}

void StudentWorld::increaseTunnelManGold() {
    p_tunnelman->pickGold();
}

void StudentWorld::increaseSonar() {
    p_tunnelman->increaseSonar();
}

void StudentWorld::increaseSquirts() {
    p_tunnelman->increaseSquirts();
}

void StudentWorld::hitTunnelMan() {
    p_tunnelman->decreaseHealth(2);
}

// checks if a boulder is in nearby in a specific direction
bool StudentWorld::checkBoulder(int x, int y, GraphObject::Direction dir) {
    for (const auto& actor : actorsList) {
        if (!actor->canActorsPassThrough()) { // boulder
            int xDistance = abs(actor->getX() - x);
            int yDistance = abs(actor->getY() - y);
            int diffX = actor->getX() - x;
            int diffY = actor->getY() - y;
//            if (distance == 1) {
//                playSound(SOUND_PROTESTER_ANNOYED);
//                continue;
//            }
            
//            cout << "Boulder x: " << actor->getX() << endl;
//            cout << "Player x: " << x << endl << endl;
//            cout << "Actor y: " << actor->getY() << endl;
//            cout << "Player y: " << y << endl << endl;
//            cout << "Diff X: " << diffX << endl;
//            cout << "Diff Y: " << diffY << endl;
//
            if (xDistance <= 4) {
               
                if (dir == GraphObject::right) {
//                    cout << "Right" << endl;
                    if (diffX <= 4 && diffX >=0 && abs(diffY) < 4) {
                        return true;
                    }
                }
                    
                if (dir == GraphObject::left) {
//                    cout << "Left" << endl;
                    if (diffX >= -4 && diffX <= 0  && abs(diffY) < 4) {
                        return true;
                    }
                }
            }
            if (yDistance < 4) {
                if (dir == GraphObject::up) {
//                    cout << "Up" << endl;
                    if (abs(diffX) < 4 && diffY > 0) {
                        return true;
                    }
                }
                
                if (dir == GraphObject::down) {
//                    cout << "Down" << endl;
                    if (abs(diffX) < 4 && diffY > -4 && diffY <=0 ) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


// sets revealed objects to visible
void StudentWorld::revealHiddenObjects(int x, int y) {
    for (const auto& actor : actorsList) {
        if (actor->canBeSonarRevealed()) {
            double distance = sqrt(pow(actor->getX() - x, 2) + pow(actor->getY() - y, 2));
            if (distance < 12.0) {
                actor->setVisible(true);
            }
        }
    }
}

void StudentWorld::dropGold(int x, int y) {
    actor = new Gold(this, x, y, pickableByProtesters, true);
    actorsList.push_back(actor);
}

bool StudentWorld::goldPickedByProtesters(int x, int y) {
    for (const auto& actor : actorsList) {
        if (actor->canPickItems()) {
            double distance = sqrt(pow(actor->getX() - x, 2) + pow(actor->getY() - y, 2));
            if (distance <= 3.0) {
                actor->pickGold();
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::annoyProtesters(int x, int y) {
    bool annoyed = false;
    for (const auto& actor : actorsList) {
        if (actor->canBeAnnoyed()) {
            double distance = sqrt(pow(actor->getX() - x, 2) + pow(actor->getY() - y, 2));
            if (distance <= 3.0) {
                actor->decreaseHealth(2);
                if (actor->getHealth() > 0) {
                    playSound(SOUND_PROTESTER_ANNOYED);
                }
                annoyed = true;
            }
        }
    }
    return annoyed;
}

// checks a region for boulders
bool StudentWorld::checkForBoulders(int x1, int x2, int y1, int y2) {
    for (const auto& actor : actorsList) {
        if (!actor->canActorsPassThrough()) {
            int actorX = actor->getX();
            int actorY = actor->getY();
            if (actorX > x1 && actorX < x2 && actorY > y1 && actorY < y2) {
                return true;
            }
        }
    }
    return false;
}

// check if an area has earth
bool StudentWorld::checkEarthArea(int x1, int x2, int y1, int y2) {
    for (int i = x1; i < x2; i++) {
        for (int j = y1; j < y2; j++) {
            if (gameboard[i][j] != nullptr) {
                return true;
            }
        }
    }
    return false;
}

void StudentWorld::hitProtesters(int x, int y) {
    for (const auto& actor : actorsList) {
        if (actor->canBeAnnoyed() && actor->getState() != falling) {
            double distance = sqrt(pow(actor->getX() - x, 2) + pow(actor->getY() - y, 2));
            if (distance <= 3.0) {
                actor->decreaseHealth(100);
            }
        }
    }
    
    if (pow(p_tunnelman->getX() - x, 2) + pow(p_tunnelman->getY() - y, 2) <= 3.0) {
        p_tunnelman->decreaseHealth(100);
    }
}


// checks if board is empty
bool StudentWorld::checkIfEmpty(int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (gameboard[x + i][y + j] != nullptr) {
                return false;
            }
        }
    }
    return true;
}

// adds a protester according to probabilities of reg vs hardcore
void StudentWorld::addProtester() {
    int hardCoreChance = getLevel() * 10 + 30;
    int probabilityOfHardcore = min(90, hardCoreChance);
    int r = rand() % 100 + 1;
    
    if (r < probabilityOfHardcore) {
        HardcoreProtester* hardpro = new HardcoreProtester(this);
        actorsList.push_back(hardpro);
        numProtesters++;
    } else {
        RegularProtester* regpro = new RegularProtester(this);
        actorsList.push_back(regpro);
        numProtesters++;
    }
    
    ticksToAddProtesters = 0;
}
