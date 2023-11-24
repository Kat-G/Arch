#pragma once
#include "helpers/Console.h"

// Monitor app that manages Server's lifecycle and monitors its state
class Monitor
{
public:
    Monitor() = default;
    virtual ~Monitor() = default;
    bool init(); // launches Server
    bool check(std::chrono::system_clock::time_point start); // checks Server state
    static void reset(); // terminates irresponsive Server
    void cleanup();

    bool initReserveServer();
    bool activateReserveServer();
    void resetReserveServer();

    static void deleteResource();
    static void getAndSetPort();

private:
    Console m_console;
};
