#include <winsock2.h>
#include <ws2tcpip.h> // for inet_pton()
#include <windows.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <lmcons.h>
#include <atlbase.h>
#include <atlsecurity.h>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

#define DEFAULT_BUFLEN 512

/// <summary>
/// The function returns true if the user who is running the application is a member of the Administrators group, 
/// which does not necessarily mean the process has admin privileges.
/// </summary>
/// <param name="rHr"></param>
/// <returns></returns>
bool IsAdministrator(HRESULT& rHr)
{
    bool bIsAdmin = false;

    try
    {
        // Open the access token of the current process.
        ATL::CAccessToken aToken;
        if (!aToken.GetProcessToken(TOKEN_QUERY))
        {
            throw MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32,
                ::GetLastError());
        }


        // Query for the access token's group information.
        ATL::CTokenGroups groups;
        if (!aToken.GetGroups(&groups))
        {
            throw MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32,
                ::GetLastError());
        }

        // Iterate through the access token's groups
        // looking for a match against the builtin admins group.
        ATL::CSid::CSidArray groupSids;
        ATL::CAtlArray<DWORD> groupAttribs;
        groups.GetSidsAndAttributes(&groupSids, &groupAttribs);
        for (UINT i = 0; !bIsAdmin && i < groupSids.GetCount(); ++i)
        {
            bIsAdmin = groupSids.GetAt(i) == ATL::Sids::Admins();
        }
        rHr = S_OK;
    }
    catch (HRESULT hr)
    {
        rHr = hr;
    }

    return bIsAdmin;
}

BOOL IsRunAsAdministrator()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:

    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fIsRunAsAdmin;
}

std::string BoolToString(bool _bool)
{
    if (_bool) return "true";
    return "false";
}

std::string WideStringToString(const WCHAR* wideStr)
{
    int length = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);

    std::string result(length, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &result[0], length, nullptr, nullptr);

    char last_char = result.back();
    int symbol_value = static_cast<int>(last_char);
    if (symbol_value == '\0')
    {
        result.erase(result.size() - 1);
    }

    return result;
}

std::string GetCurrentPath()
{
    WCHAR path[500];
    DWORD size = GetModuleFileNameW(NULL, path, 500);
    //std::wcout << "The current path to the program file: \n" << path << std::endl;
    std::string currentPath = WideStringToString(path);

    return currentPath;
}

std::string GetCurrentUserName()
{
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    GetUserName((TCHAR*)username, &size);
    //std::wcout << "The current user name: " << username << std::endl;
    std::wstring usernameWideString(username);
    std::string usernameString(usernameWideString.begin(), usernameWideString.end());
    //std::string usernameString(getenv("USERNAME"));
    return usernameString;
}

std::pair<std::string, std::string> TokenizeString(const std::string& s, char del)
{
    std::stringstream ss(s);
    std::string part1, part2;
    std::getline(ss, part1, del);
    std::getline(ss, part2);

    return std::make_pair(part1, part2);
}

struct User
{
    std::string filePath;
    std::string username;
    bool isAdmin;
};

int main(int argc, char* argv[]) // TCP-CPlusPlusClient 127.0.0.1:1000 // TCP-CPlusPlusClient 127.0.0.1:8000
{
    if (argc < 2)
    {
        std::cout << "Usage: TCP-CPlusPlusClient <server_ip_address>" << std::endl;
        return 1;
    }
    if (argc == 2)
    {
        std::pair<std::string, std::string> serverIpAddressAndPort = TokenizeString(argv[1], ':');
        const char* ipAddress = serverIpAddressAndPort.first.c_str();
        unsigned short port = static_cast<unsigned short>(std::stoi(serverIpAddressAndPort.second));

        int connectionAttempts = 0;
        bool hasDataBeenSent = false;
        while (!hasDataBeenSent)
        {
            std::cout << "\t\t------TCP Client-------" << std::endl;
            std::cout << std::endl;           

            HRESULT hr;
            // Data in JSON format to send to the server.
            User user { GetCurrentPath(), GetCurrentUserName(), (bool)IsAdministrator(hr) };
            nlohmann::json jsonData{};
            jsonData["filePath"] = user.filePath;
            jsonData["username"] = user.username;
            jsonData["isAdmin"] = user.isAdmin;
            // Convert the JSON object to a string.
            std::string jsonString = jsonData.dump();

            // STEP - 1 | Initialize Windows Sockets API.
            WSADATA wsaData;
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result != 0)
            {
                std::cout << "Windows Sockets API initialization failed" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(30));
                ++connectionAttempts;
                continue;
            }
            std::cout << "1| Windows Sockets API initialization succeeded" << std::endl;

            // Step - 2 | TCP socket creation.
            SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            if (sock == INVALID_SOCKET) 
            {
                std::cout << "TCP client socket creation failed" << WSAGetLastError() << std::endl;
                WSACleanup();
                std::this_thread::sleep_for(std::chrono::seconds(30));
                ++connectionAttempts;
                continue;
            }
            std::cout << "2| TCP client socket creation succeeded" << std::endl;

            // STEP - 3 | Filling the structure.
            sockaddr_in server_address;
            server_address.sin_family = AF_INET;
            inet_pton(AF_INET, ipAddress, &server_address.sin_addr);
            server_address.sin_port = htons(port);

            // STEP - 4 | Connecting to the Node.JS server.
            if (connect(sock, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR) 
            {
                std::cout << "Connection Failed & Error No ->" << WSAGetLastError() << std::endl;
                WSACleanup();
                std::this_thread::sleep_for(std::chrono::seconds(30));
                ++connectionAttempts;
                continue;
            }
            std::cout << "3| Connection was successful" << std::endl;

            // STEP - 5 | Sending data to the server.
            if (send(sock, jsonString.c_str(), jsonString.size(), 0) == SOCKET_ERROR)
            {
                std::cout << "Sending Failed & Error No ->" << WSAGetLastError() << std::endl;
                closesocket(sock);
                WSACleanup();
                std::this_thread::sleep_for(std::chrono::seconds(30));
                ++connectionAttempts;
                continue;
            }
            std::cout << "4| Data sent successfully" << std::endl;

            // STEP - 6 | Receiving a confirmation message from the server.
            char recvBuffer[DEFAULT_BUFLEN];
            int bytes_received = recv(sock, recvBuffer, sizeof(recvBuffer), 0);
            if (bytes_received == SOCKET_ERROR)
            {
                std::cout << "Receiving Failed & Error No ->" << WSAGetLastError() << std::endl;
                closesocket(sock);
                WSACleanup();
                std::this_thread::sleep_for(std::chrono::seconds(30));
                ++connectionAttempts;
                continue;
            }
            recvBuffer[bytes_received] = '\0';
            std::cout << "5| Receiving a confirmation message from the server was successful! \n\tReceived message from server: \"" << recvBuffer << "\"" << std::endl;

            // STEP - 7 | Closing the socket.
            if (closesocket(sock) == SOCKET_ERROR)
            {
                std::cout << "Closing Failed & Error No ->" << WSAGetLastError() << std::endl;
            }
            std::cout << "6| Socket successfully closed" << std::endl;
            
            WSACleanup();
            hasDataBeenSent = true;
        }
    }

    return 0;
}
