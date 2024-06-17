#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>
#include <condition_variable>
#include <mutex>

class Client
{
public:
    Client(int index, int maxFloor, std::vector<std::vector<std::string>> &screen, int &entitiesOnScreen, bool &elevatorReady, std::condition_variable &cv, std::mutex &mtx, int &takenSeats, std::vector<bool> &floorsOccupied, std::vector<std::condition_variable> &floorCvs, std::vector<std::mutex> &floorMtxs);
    ~Client();

private:
    int index;
    int currentFloor;
    int destinationFloor;
    double speed;
    bool ready;

    std::vector<std::vector<std::string>> &screen;
    int &entitiesOnScreen;
    bool &elevatorReady;
    std::condition_variable &cv;
    std::mutex &mtx;
    int &takenSeats;

    std::vector<bool> &floorsOccupied;
    std::vector<std::condition_variable> &floorCvs;
    std::vector<std::mutex> &floorMtxs;

    void move();
    void getOnElevator();
    void getOffElevator();
};

#endif // CLIENT_H
