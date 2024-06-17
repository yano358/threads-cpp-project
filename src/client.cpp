#include "Client.h"
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std;

Client::Client(int index, int maxFloor, vector<vector<string>> &screen, int &entitiesOnScreen, bool &elevatorReady, std::condition_variable &cv, mutex &mtx, int &takenSeats, std::vector<bool> &floorsOccupied, std::vector<std::condition_variable> &floorCvs, std::vector<std::mutex> &floorMtxs)
    : index(index), currentFloor(0), screen(screen), entitiesOnScreen(entitiesOnScreen), elevatorReady(elevatorReady), cv(cv), mtx(mtx), takenSeats(takenSeats), floorsOccupied(floorsOccupied), floorCvs(floorCvs), floorMtxs(floorMtxs)
{
    // Generate random destination floor (excluding floor 0)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, maxFloor - 1);
    destinationFloor = dis(gen);

    // Generate random speed (0.5 to 1.1 seconds)
    std::uniform_real_distribution<> speedDis(0.5, 1.1);
    speed = speedDis(gen);
    ready = false;

    {
        std::lock_guard<std::mutex> lock(mtx);
        entitiesOnScreen++;
    }

    move();
}

Client::~Client()
{
    std::lock_guard<std::mutex> lock(mtx);
    entitiesOnScreen--;
}

void Client::move()
{
    int speedNew = speed * 1000;
    for (int y = 12; y >= 5; --y)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            screen[currentFloor][0][y] = '0' + index;
            screen[currentFloor][0][y + 1] = '#';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));
    }
    ready = true;

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]
                { return elevatorReady && takenSeats != 3; });
    }

    getOnElevator();
}

void Client::getOnElevator()
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        takenSeats++;
    }

    for (int y = 0; y <= destinationFloor; ++y)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
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
    {
        std::lock_guard<std::mutex> lock(mtx);
        takenSeats--;
    }

    int64_t speedNew = speed * 1000;
    {
        std::lock_guard<std::mutex> lock(mtx);
        screen[destinationFloor][0][2] = ' ';
    }

    // Move to the first tile of the destination floor
    {
        std::lock_guard<std::mutex> lock(mtx);
        screen[destinationFloor][0][6] = '0' + index;
    }
    floorsOccupied[destinationFloor] = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));

    for (int y = 7; y <= 12; ++y)
    {
        // Check if there is a client on the floor above and wait if there is
        if (destinationFloor < floorsOccupied.size() - 1 && floorsOccupied[destinationFloor + 1])
        {
            std::unique_lock<std::mutex> lock(floorMtxs[destinationFloor + 1]);
            floorCvs[destinationFloor + 1].wait(lock, [this]
                                                { return !floorsOccupied[destinationFloor + 1]; });
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            screen[destinationFloor][0][y] = '0' + index;
            screen[destinationFloor][0][y - 1] = '#';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(speedNew));
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        screen[destinationFloor][0][12] = '#';
    }

    // Notify clients on the floor below
    if (destinationFloor > 1)
    {
        std::lock_guard<std::mutex> lock(floorMtxs[destinationFloor]);
        floorsOccupied[destinationFloor] = false;
        floorCvs[destinationFloor].notify_all();
    }
}
