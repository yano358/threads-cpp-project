#include <ncurses.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <queue>

#include <mutex>
#include <condition_variable>

#include "client.h"

using namespace std;

// MARK: todo
// TODO: threads for clients, mutex instead of pthread mutex, add elevator print

// Global variables:
vector<vector<string>> screen(5, vector<string>(1, "|   |######"));
int currFloor = 0;
int prevFloor = 0;
bool running = true;
bool elevatorReady = false;
int const maxEntitiesOnScreen = 4;
int entitiesOnScreen = 0;
int currEntityIndex = 0;

mutex m;

// helper constants for screen management
int const ELEVATOR_SLOT_INDEX = 2;
int const LAST_INDEX = screen[0][0].size() - 1;
int const BEFORE_ELEVATOR_ENTER_INDEX = 5;

void print_screen()
{
    clear(); // erase() instead of clear

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
        if (currFloor == 4)
        {
            elevatorReady = true;
        }
        else
        {
            elevatorReady = false;
        }
        prevFloor = currFloor;
        currFloor = (currFloor + 1) % screen.size();

        screen[prevFloor][0][ELEVATOR_SLOT_INDEX - 1] = ' ';
        screen[prevFloor][0][ELEVATOR_SLOT_INDEX + 1] = ' ';

        screen[currFloor][0][ELEVATOR_SLOT_INDEX - 1] = '[';
        screen[currFloor][0][ELEVATOR_SLOT_INDEX + 1] = ']';
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void spawnPassenger()
{
    while (running)
    {
        m.lock();
        if (entitiesOnScreen < maxEntitiesOnScreen)
        {
            currEntityIndex = (currEntityIndex + 1) % maxEntitiesOnScreen;
            thread clientThread([]()
                                { Client client(currEntityIndex, 5, screen, entitiesOnScreen, elevatorReady); });
            clientThread.detach();
            int interval = rand() % 5000 + 1000;
            this_thread::sleep_for(chrono::milliseconds(interval));
        }
        m.unlock();
    }
}

int main(int srgc, char **argv)
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
            else
            {
                mvprintw(10, 15, "Elevator thread is not joinable");
            }
            if (initScreenThread.joinable())
            {
                initScreenThread.join();
            }
            else
            {
                mvprintw(10, 15, "Init screen thread is not joinable");
            }
            if (spawnPassengerThread.joinable())
            {
                spawnPassengerThread.join();
            }
            else
            {
                mvprintw(10, 15, "Spawn passenger thread is not joinable");
            }
            endwin();
            return 0;
        }
    }
}
