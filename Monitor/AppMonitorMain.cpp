#include <windows.h>
#include <iostream>
#include <filesystem>
#include "AppMonitor.h"

int main()
{
    Monitor m;
    if (!m.init()) {
        std::cerr << "Monitor initialization failed!\n";
        if (std::filesystem::exists(std::string("./resources/CREATED"))) {
            std::cerr << "Server initialization failed!\n";
        }
        return 1;
    }
    Sleep(1000);
    Monitor::getAndSetPort();
    Monitor::deleteResource();
    m.initReserveServer();

    auto currentTime = std::chrono::system_clock::now();
    std::time_t startTime = std::chrono::system_clock::to_time_t(currentTime);

    while (1) {
        if (!m.check(currentTime)) {
            m.resetReserveServer();
            Monitor::deleteResource();
        }
        Sleep(3000);
    }
}