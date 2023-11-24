#include "AppServer.h"
#include "../helpers/UtilString.h"
#include "../helpers/UtilFile.h"
#include <string>
#include <process.h>
#include <filesystem>
#include <iostream>

bool Server::init(int port)
{
    if (!m_socket.init(1000) || !m_socket.listen(port))
        return false;

    if (!fileWriteExclusive("resources\\CREATED", toStr(m_socket.port()) + "," + toStr(_getpid())))
        return false;

    printf("server started: port %d, pid %d\n", m_socket.port(), _getpid());

    char* state = fileReadStr("resources\\STATE"); // load state from previous run
    if (state)
    {
        for (std::string& line : split(state, "\n"))
            if (!line.empty())
                m_data.push_back(line);
        delete[] state;
    }

    return true;
}

void Server::run()
{
    while (1)
    {
        fileWriteStr(std::string("resources\\ALIVE") + toStr(_getpid()), ""); // pet the watchdog
        std::shared_ptr<Socket> client = m_socket.accept(); // accept incoming connection
        if (!client->isValid())
            continue;

        int n = client->recv(); // receive data from the connection, if any
        char* data = client->data();
        printf("-----RECV-----\n%s\n--------------\n", n > 0 ? data : "Error");
        const std::vector<std::string>& tokens = split(data, " ");
        if (tokens.size() >= 2 && tokens[0] == "GET") // this is browser's request
        {
            // convert URL to file system path, e.g. request to img/1.png resource becomes request to .\img\1.png file in Server's directory tree
            const std::string& filename = join(split(tokens[1], "/"), "\\");
            if (filename == "\\")
            { // main entry point (e.g. http://localhost:12345/)
                std::string payload = "";
                for (auto s : m_data) {
                    std::string path = '.' + join(split(s, "/"), "\\");
                    if (fileExists(path) && isImageExtension(path)) {
                        payload += "<img src=\"" + s + "\" style=\"max-width: 250px; max-height: 250px;\"><br>";
                    }
                    else if (fileExists(path)) {
                        int size = std::filesystem::file_size(path);
                        //std::string contentType = "text/html";
                        payload += (std::string(fileRead(path), size) + "<br>"); // Send the image content as the response
                    }
                    else {
                        payload += (s + "<br>"); // collect all the feed and send it back to browser
                    }
                }
                client->sendStr("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + toStr(payload.length()) + "\r\n\r\n" + payload);
            }
            else if (filename != "\\") {
                std::string payload = "";
                std::string path = '.' + filename;
                if (fileExists(path) && isImageExtension(path)) {
                    int size = std::filesystem::file_size(path);
                    std::string contentType =
                        "image/" + getExtension(path); // Change the content type according to the image file format
                    client->sendStr(
                        "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " + toStr(size) +
                        "\r\n\r\n");
                    client->send(fileRead(path), size); // Send the image content as the response
                }
                else {
                    payload += "Image not found: " + filename + "<br>";
                    client->sendStr("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + toStr(payload.length()) + "\r\n\r\n" + payload);
                }
            }
            else {
                client->sendStr("HTTP/1.1 404 Not Found\r\n\r\n");
            }
        }
        else if (tokens.size() >= 2 && tokens[0] == "TXT") {
            const int prefix = 4; 
            m_data.push_back(data + prefix);
            fileAppend("resources\\STATE", m_data.back() + "\n");
        }
        else if (tokens.size() >= 2 && tokens[0] == "IMG") {
            std::string name = "/resources/files/" + tokens[1];
            std::string path = ".\\resources\\files\\" + tokens[1];
            int prefix = 4 + tokens[1].length() + 1;
            int size = n - prefix;
            fileWrite(path, data + prefix, size);

            m_data.push_back(name);
            fileAppend("resources\\STATE", m_data.back() + "\n");
        }
    }
}