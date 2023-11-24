#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <string>
#include <filesystem>
#include "AppMonitor.h"
#include "helpers/Process.h"
#include "../helpers/UtilString.h"
#include "../helpers/UtilFile.h"

static Process sServer;
static Process sReserveServer;
static std::string sPort = "0";
bool flag = true;

bool Monitor::init()
{
    m_console.handleCtrlC(Monitor::reset); // if Monitor's execution is aborted via Ctrl+C, reset() cleans up its internal state
    cleanup();
    char cmd[256] = {};
    sprintf(cmd, "Server.exe %s", sPort.c_str());
    bool ok = sServer.create(cmd); // launching Server
    printf(ok ? "monitoring \"%s\"\n" : "error: cannot monitor \"%s\"\n", cmd);
    return ok;
}

bool Monitor::initReserveServer() {
    char cmd[256] = {};
    sprintf(cmd, "Server.exe %s", sPort.c_str());
    Sleep(1000);
    bool ok = sReserveServer.create(cmd); // launching Server
    printf(ok ? "monitoring backup \"%s\" pid:%s\n" : "error: cannot monitor reserve server \"%s\" \n", cmd, sReserveServer.pid().c_str());
    if (ok)
    {
        sReserveServer.stop();
    }
    return ok;
}

bool Monitor::check(std::chrono::system_clock::time_point start)
{
    std::string heartBeatFilePath = std::string("./resources/ALIVE" + sServer.pid());

    bool isGotBeat = fileExists(heartBeatFilePath);

    /* Проверка */

    auto currentTimeAfterDelay = std::chrono::system_clock::now();
    std::time_t endTime = std::chrono::system_clock::to_time_t(currentTimeAfterDelay);
    std::chrono::duration<double> elapsedSeconds = currentTimeAfterDelay - start;

    if (elapsedSeconds.count() > 100 && flag) {
        sServer.terminate();
        flag = false;
    }
    /* Завершение проверки */

    if (isGotBeat) {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        fileAppend("resources\\STATUS", "pid:" + sServer.pid() + " alive - " + std::ctime(&now_c) + "\n");
    }

    if (!isGotBeat || !sServer.wait(3000)) {
        return false;
    }
    return true;
}

bool Monitor::activateReserveServer()
{
    if (sReserveServer.isStopped()) {
        sReserveServer.activate();
        return true;
    }
    return false;
}

void Monitor::resetReserveServer()
{
    activateReserveServer();
    sServer = sReserveServer;
    initReserveServer();
}

void Monitor::deleteResource() {
    std::string directoryPath = "./resources";
    for (const auto& file : std::filesystem::directory_iterator(directoryPath)) {
        if (std::filesystem::is_regular_file(file)) {
            if (file.path().filename().string() != "STATE" && file.path().filename().string() != "STATUS") {
                std::filesystem::remove(file.path());
            }
        }
    }
}

void Monitor::getAndSetPort() {
    std::string path = std::string("./resources/CREATED");
    sPort = split(fileReadStr(path), ",")[0];
}

void Monitor::reset()
{
    sServer.terminate();
    sReserveServer.terminate();
}

void Monitor::cleanup() {
    std::filesystem::remove("./resources/STATE");
    std::filesystem::remove("./resources/STATUS");
}