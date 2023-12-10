#define _CRT_SECURE_NO_WARNINGS
#include "AppClient.h"
#include "helpers/SocketClient.h"
#include "../helpers/UtilString.h"
#include "../helpers/UtilFile.h"
#include <Windows.h>

bool Client::send(const std::string& url, const std::string& msg)
{
    SocketClient s;
    auto start = std::chrono::steady_clock::now();
    while ((!s.init() || !s.connect(url)) && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < 60) {
        Sleep(10);
    }

    printf("sending text message \"%s\"\n", msg.c_str());

    int len;
    if (fileExists(msg)) {
        len = s.sendFile(msg);
    }
    else {
        len = s.sendStr(msg);
    }


    printf("sent %d bytes\n", len);
    return len > 0;
}
