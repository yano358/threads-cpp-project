#ifndef CLIENT_H
#define CLIENT_H

#include <random>
#include <ncurses.h>
#include <chrono>
#include <vector>
#include <mutex>

using namespace std;

class Client
{
public:
    Client(int index, int maxFloor, std::vector<std::vector<std::string>> &screen, int &entitiesOnScreen, bool &elevatorReady);
    ~Client();

    void move();
    void getOnElevator();
    void getOffElevator();

    void clearPrevious(int x, int y);

    int index;
    int currentFloor;
    int destinationFloor;
    int &entitiesOnScreen;
    vector<vector<string>> &screen;
    double speed;
    bool ready;
    bool &elevatorReady;

    mutex m;
};

#endif /* CLIENT_H */
