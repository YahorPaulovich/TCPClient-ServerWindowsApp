#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

int main()
{
	std::cout << "\t\t------TCP Server-------" << std::endl;
	std::cout << std::endl;
	WSADATA Winsockdata;
	int iWsaStartup;
	int iWsaCleanup;

	SOCKET TCPServerSocket;
	int iCloseSocket;
	struct sockaddr_in TCPServerAdd;
	struct sockaddr_in TCPClientAdd;
	int iTCPClientAdd = sizeof(TCPClientAdd);
	int iBind;
	int iListen;
	SOCKET sAcceptSocket;
	int iSend;
	char SenderBuffer[512] = "Hello from server!";
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

	// STEP -2 Fill the structure-------------------------------

	TCPServerAdd.sin_family = AF_INET;
	TCPServerAdd.sin_addr.s_addr = inet_addr("127.0.0.1");
	TCPServerAdd.sin_port = htons(8000);

	//Step -3 Socket Creation------------------------------------

	TCPServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPServerSocket == INVALID_SOCKET)
	{
		std::cout << "TCP Server Socket Creation failed" << WSAGetLastError() << std::endl;
	}

	//Step -4 bind fun------------------------------------------

	iBind = bind(TCPServerSocket, (SOCKADDR*)&TCPServerAdd, sizeof(TCPServerAdd));
	if (iBind == SOCKET_ERROR)
	{
		std::cout << "Binding Failed &Error No->" << WSAGetLastError() << std::endl;

	}
	std::cout << "Binding success" << std::endl;

	//STEP-5 Listen fun------------------------------------------

	iListen = listen(TCPServerSocket, 2);
	if (iListen == SOCKET_ERROR)
	{
		std::cout << "Listen fun failed &error No->" << WSAGetLastError();
	}
	std::cout << "Listen fun success" << std::endl;

	// STEP-6 Accept---------------------------------------------

	sAcceptSocket = accept(TCPServerSocket, (SOCKADDR*)&TCPClientAdd, &iTCPClientAdd);
	if (sAcceptSocket == INVALID_SOCKET)
	{
		std::cout << "Accept failed & Error No ->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Accept fun success" << std::endl;

	// STEP-7 Send Data to the client

	iSend = send(sAcceptSocket, SenderBuffer, iSenderBuffer, 0);
	if (iSend == SOCKET_ERROR)
	{
		std::cout << "Sending Failed & Error No->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Send fun success" << std::endl;

	// STEP -8 Recv Data from Client

	iRecv = recv(sAcceptSocket, RecvBuffer, iRecvBuffer, 0);
	if (iRecv == SOCKET_ERROR)
	{
		std::cout << "Receiving Failed & Error No->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Receive fun success" << std::endl;
	std::cout << "Data Received -> " << RecvBuffer << std::endl;

	//STEP - 9 Close Socket

	iCloseSocket = closesocket(TCPServerSocket);

	if (iCloseSocket == SOCKET_ERROR)
	{
		std::cout << "Closing Failed & Error No->" << WSAGetLastError() << std::endl;
	}
	std::cout << "Cleanup fun success" << std::endl;
	system("PAUSE");
}