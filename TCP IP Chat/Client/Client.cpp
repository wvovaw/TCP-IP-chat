#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

std::mutex mut;
void SendMsg(char* buf, SOCKET& s)
{
	while (true)
	{
		std::string msg;
		//Prompt the user for some text
		std::cout << "> ";
		std::getline(std::cin, msg);

		if (msg.size() > 0)		//Make sure that user has typed in something
		{
			//Send the text
			int sendResult = send(s, msg.c_str(), msg.size() + 1, 0);
			if (sendResult != SOCKET_ERROR)	ZeroMemory(buf, 4096);
		}
	}
}
void RecieveMsg(char* buf, SOCKET & s)
{
	while (true)
	{
		int byteReceived = recv(s, buf, 4096, 0);
		if (byteReceived > 0)
		{
			//Echo response to console
			mut.lock();
			std::cout << std::string(buf, 0, byteReceived) << std::endl;
			mut.unlock();
		}
	}
}
void main()
{
	std::string ipAddress = "178.44.210.166";		//IP Сервера
	int port = 54000;							//Listening Порт сервера
	//Инициализация Winsock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResilt = WSAStartup(ver, &data);
	if (wsResilt != 0)
	{
		std::cerr << "Can't start winsock, Err #" << wsResilt << std::endl;
		return;
	}

	//Создание socket'ов
	//=====================================================================================
	SOCKET sock_out = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_out == INVALID_SOCKET)
	{
		std::cerr << "Can't create output socket, Err #" << WSAGetLastError() << std::endl;
		return;
	}

	SOCKET sock_in = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_in == INVALID_SOCKET)
	{
		std::cerr << "Can't create input socket, Err #" << WSAGetLastError() << std::endl;
		return;
	}
	//=====================================================================================
	//Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	//Connet to server
	int OutResult = connect(sock_out, (sockaddr*)& hint, sizeof(hint));
	int InResult = connect(sock_in, (sockaddr*)& hint, sizeof(hint));
	if (OutResult == SOCKET_ERROR || InResult == SOCKET_ERROR)
	{
		std::cerr << "Can't connect to server, Err #" << WSAGetLastError << std::endl;
		WSACleanup();
		return;
	}

	char RcvBuf[4096];
	char SndBuf[4096];

	std::thread OutputData(SendMsg, SndBuf, std::ref(sock_out));
	std::thread InputData(RecieveMsg, RcvBuf, std::ref(sock_in));
	OutputData.join();
	InputData.join();
	//Gracefully close down everything
	closesocket(sock_out);
	closesocket(sock_in);
	WSACleanup();
}