#include <ncurses.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Client.h"

using namespace std;

// Global variables:
vector<vector<string>> screen(5, vector<string>(1, "|   |######"));
int currFloor = 0;
int prevFloor = 0;
bool running = true;
bool elevatorReady = false;
int const maxEntitiesOnScreen = 7;
int entitiesOnScreen = 0;
int currEntityIndex = 0;

mutex m;
condition_variable cv;

// helper constants for screen management
int const ELEVATOR_SLOT_INDEX = 2;
int const LAST_INDEX = screen[0][0].size() - 1;
int const BEFORE_ELEVATOR_ENTER_INDEX = 5;

int const ELEVATOR_CAPACITY = 3;
int takenSeats = 0;

void print_screen()
{
    erase(); // erase() instead of clear
    for (int i = 0; i < screen.size(); ++i)
    {
        for (int j = 0; j < screen[i].size(); ++j)
        {
            string line = screen[i][j];
            for (int k = 0; k < line.size(); ++k)
            {
                mvaddch(i, k, line[k]);
            }
        }
    }
    mvprintw(10, 15, "%d", entitiesOnScreen);
    refresh();
}

void updateScreen()
{
    while (running)
    {
        print_screen();
        this_thread::sleep_for(chrono::milliseconds(25));
    }
}

void elevator()
{
    while (running)
    {
        {
            std::lock_guard<std::mutex> lock(m);
            if (currFloor == 4)
            {
                elevatorReady = true;
                cv.notify_all();
            }
            else
            {
                elevatorReady = false;
            }
        }

        prevFloor = currFloor;
        currFloor = (currFloor + 1) % screen.size();

        {
            std::lock_guard<std::mutex> lock(m);
            screen[prevFloor][0][ELEVATOR_SLOT_INDEX - 1] = ' ';
            screen[prevFloor][0][ELEVATOR_SLOT_INDEX + 1] = ' ';

            screen[currFloor][0][ELEVATOR_SLOT_INDEX - 1] = '[';
            screen[currFloor][0][ELEVATOR_SLOT_INDEX + 1] = ']';
        }

        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void spawnPassenger()
{
    while (running)
    {
        {
            std::lock_guard<std::mutex> lock(m);
            if (entitiesOnScreen < maxEntitiesOnScreen)
            {
                currEntityIndex = (currEntityIndex + 1) % maxEntitiesOnScreen;
                thread clientThread([]()
                                    { Client client(currEntityIndex, 5, screen, entitiesOnScreen, elevatorReady, cv, m, takenSeats); });
                clientThread.detach();
            }
        }
        int interval = rand() % 2000 + 1000;
        this_thread::sleep_for(chrono::milliseconds(interval));
    }
}

int main(int argc, char **argv)
{
    initscr();
    noecho();
    curs_set(0);
    thread initScreenThread(updateScreen);
    thread elevatorThread(elevator);
    thread spawnPassengerThread(spawnPassenger);
    while (true)
    {
        char c = getch();
        if (c == ' ')
        {
            mvprintw(10, 15, "Space pressed - waiting for the program to finish");
            running = false;
            if (elevatorThread.joinable())
            {
                elevatorThread.join();
            }
            if (initScreenThread.joinable())
            {
                initScreenThread.join();
            }
            if (spawnPassengerThread.joinable())
            {
                spawnPassengerThread.join();
            }
            endwin();
            return 0;
        }
    }
}
