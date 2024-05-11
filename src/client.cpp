#include "Client.h"
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
using namespace std;

Client::Client(int index, int maxFloor, vector<vector<string>> &screen, int &entitiesOnScreen, bool &elevatorReady) : index(index), currentFloor(0), screen(screen), entitiesOnScreen(entitiesOnScreen), elevatorReady(elevatorReady)
{
    // Generate random destination floor (excluding floor 0)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, maxFloor - 1);
    destinationFloor = dis(gen);

    // Generate random speed (0.5 to 1.5 seconds)
    std::uniform_real_distribution<> speedDis(0.5, 1.5);
    speed = speedDis(gen);
    ready = false;
    mutex m;

    entitiesOnScreen++;

    move();

    return;
}

Client::~Client()
{
    m.lock();
    entitiesOnScreen--;
    m.unlock();
}

void Client::move()
{
    int speedNew = speed * 1000;
    for (int y = 12; y >= 5; --y)
    {
        m.lock();
        screen[currentFloor][0][y] = '0' + index;
        screen[currentFloor][0][y + 1] = '#';
        m.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));
    }
    ready = true;
    while (!elevatorReady)
    {
        continue;
    }
    getOnElevator();
}

void Client::getOnElevator()
{
    for (int y = 0; y <= destinationFloor; ++y)
    {
        m.lock();
        screen[y][0][2] = '0' + index;
        screen[y][0][5] = '#';
        m.unlock();
        if (y != 0)
        {
            m.lock();
            screen[y - 1][0][2] = ' ';
            m.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    getOffElevator();
}

void Client::getOffElevator()
{
    m.lock();
    int64_t speedNew = speed * 1000;
    screen[destinationFloor][0][2] = ' ';
    m.unlock();
    for (int y = 6; y <= 12; ++y)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));
        m.lock();
        screen[destinationFloor][0][y] = '0' + index;
        screen[destinationFloor][0][y - 1] = '#';
        m.unlock();
    }
    this_thread::sleep_for(chrono::milliseconds(speedNew));
    m.lock();
    screen[destinationFloor][0][12] = '#';
    m.unlock();
}
