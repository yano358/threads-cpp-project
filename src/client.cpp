#include "Client.h"
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std;

Client::Client(int index, int maxFloor, vector<vector<string>> &screen, int &entitiesOnScreen, bool &elevatorReady, std::condition_variable &cv, mutex &mtx, int &takenSeats)
    : index(index), currentFloor(0), screen(screen), entitiesOnScreen(entitiesOnScreen), elevatorReady(elevatorReady), cv(cv), mtx(mtx), takenSeats(takenSeats)
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

    {
        std::lock_guard<std::mutex> lock(m);
        entitiesOnScreen++;
    }

    move();
}

Client::~Client()
{
    std::lock_guard<std::mutex> lock(m);
    entitiesOnScreen--;
}

void Client::move()
{
    int speedNew = speed * 1000;
    for (int y = 12; y >= 5; --y)
    {
        {
            std::lock_guard<std::mutex> lock(m);
            screen[currentFloor][0][y] = '0' + index;
            screen[currentFloor][0][y + 1] = '#';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));
    }
    ready = true;

    {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [this]
                { return elevatorReady && takenSeats != 3; });
    }

    getOnElevator();
}

void Client::getOnElevator()
{
    m.lock();
    takenSeats++;
    m.unlock();
    for (int y = 0; y <= destinationFloor; ++y)
    {
        {
            std::lock_guard<std::mutex> lock(m);
            screen[y][0][2] = '0' + index;
            screen[y][0][5] = '#';
            if (y != 0)
            {
                screen[y - 1][0][2] = ' ';
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    getOffElevator();
}

void Client::getOffElevator()
{
    m.lock();
    takenSeats--;
    m.unlock();
    int64_t speedNew = speed * 1000;
    {
        std::lock_guard<std::mutex> lock(m);
        screen[destinationFloor][0][2] = ' ';
    }
    for (int y = 6; y <= 12; ++y)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));
        {
            std::lock_guard<std::mutex> lock(m);
            screen[destinationFloor][0][y] = '0' + index;
            screen[destinationFloor][0][y - 1] = '#';
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));
    {
        std::lock_guard<std::mutex> lock(m);
        screen[destinationFloor][0][12] = '#';
    }
}
