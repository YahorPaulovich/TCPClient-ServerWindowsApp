#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

int main()
{
	std::cout << "\t\t------TCP Client-------" << std::endl;
	std::cout << std::endl;
	WSADATA Winsockdata;
	int iWsaStartup;
	int iWsaCleanup;

	SOCKET TCPClientSocket;
	int iCloseSocket;
	struct sockaddr_in TCPServerAdd;
	int iConnect;

	int iSend;
	char SenderBuffer[512] = "Hello from client!";
	int iSenderBuffer = strlen(SenderBuffer) + 1;
	int iRecv;
	char RecvBuffer[512];
	int iRecvBuffer = strlen(RecvBuffer) + 1;

	//Step-1 WSAStartup Fun------------------------------------

	iWsaStartup = WSAStartup(MAKEWORD(2, 2), &Winsockdata);
	if (iWsaStartup != 0)
	{
		std::cout << "WSAStartup Failed" << std::endl;
	}
	std::cout << "WSAStartup Success" << std::endl;

	//Step -2 Socket Creation------------------------------------

	TCPClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPClientSocket == INVALID_SOCKET)
	{
		std::cout << "TCP Client Socket Creation failed" << WSAGetLastError() << std::endl;
	}
	std::cout << "TCP client socket creation success" << std::endl;

	// STEP -3 Fill the structure-------------------------------

	TCPServerAdd.sin_family = AF_INET;
	TCPServerAdd.sin_addr.s_addr = inet_addr("127.0.0.1");
	TCPServerAdd.sin_port = htons(8000);

	// STEP-4 Connect fun---------------------------------------------

	iConnect = connect(TCPClientSocket, (SOCKADDR*)&TCPServerAdd, sizeof(TCPServerAdd));
	if (iConnect == SOCKET_ERROR)
	{
		std::cout << "Connection failed & Error No ->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Connection success" << std::endl;

	// STEP -5 Recv Data from Server

	iRecv = recv(TCPClientSocket, RecvBuffer, iRecvBuffer, 0);
	if (iRecv == SOCKET_ERROR)
	{
		std::cout << "Receiving Failed & Error No->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Receive fun success" << std::endl;
	std::cout << "Data Received -> " << RecvBuffer << std::endl;

	// STEP-6 Send Data to the server

	iSend = send(TCPClientSocket, SenderBuffer, iSenderBuffer, 0);
	if (iSend == SOCKET_ERROR)
	{
		std::cout << "Sending Failed & Error No->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Data sending success" << std::endl;

	//STEP - 7 Close Socket

	iCloseSocket = closesocket(TCPClientSocket);
	if (iCloseSocket == SOCKET_ERROR)
	{
		std::cout << "Closing Failed & Error No->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Closing Socket success" << std::endl;

	system("PAUSE");
	return 0;
}