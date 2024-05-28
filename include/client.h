#ifndef CLIENT_H
#define CLIENT_H

#include <random>
#include <ncurses.h>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>

using namespace std;

class Client
{
public:
    Client(int index, int maxFloor, std::vector<std::vector<std::string>> &screen, int &entitiesOnScreen, bool &elevatorReady, condition_variable &cv, mutex &mtx, int &takenSeats);
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
    condition_variable &cv;

    int &takenSeats;

    mutex m;
    mutex &mtx;
};

#endif /* CLIENT_H */
