#include <iostream>
#include <string>
#include <sstream>

#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

void main()
{
	//Инициализируем winsock
	WSADATA wsdata;
	WORD ver = MAKEWORD(2, 2);

	int wsok = WSAStartup(ver, &wsdata);
	if (wsok != 0)
	{
		std::cerr << "Can't Initilize winsock! Exit." << std::endl;
		return;
	}

	//Создаем сокет
	SOCKET listening = socket(AF_INET, SOCK_STREAM, NULL);
	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket! Exit." << std::endl;
		return;
	}

	//Привязываем IP и порт к сокету
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;		//может также использовать inet_pton

	bind(listening, (sockaddr*)& hint, sizeof(hint));

	//Сообщаем winsock что socket is listening
	listen(listening, SOMAXCONN);

	fd_set master;
	FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true)
	{
		//make a copy for save
		fd_set copy = master;

		int socketCount = select(0, &copy, NULL, NULL, NULL);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == listening)
			{
				//Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				//Add the new connection to the list of connected clients
				FD_SET(client, &master);

				//Send a welcom message to the new connection
				std::string welcomeMsg = "Welcome to my first TCP IP multiclient chat!\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);


				//TODO: Broadcast we have a new connection

			}
			else
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				//Receive a message 
				int bytesReceived = recv(sock, buf, 4096, 0);
				if (bytesReceived <= 0)
				{
					//Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					//Send message to other clients, and definiately NOT the listening sockets
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock)
						{
							std::ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							std::string strOut = ss.str();

							//Send message 
							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}

	}

	//Cleanup Winsock
	WSACleanup();
}


//single connection code 
//void main()
//{
//	//Ожидание соединения
//	sockaddr_in client;
//	int clientsize = sizeof(client);
//
//	SOCKET clientsocket = accept(listening, (sockaddr*)& client, &clientsize);
//
//	char host[NI_MAXHOST];		//удаленное имя клиента
//	char service[NI_MAXSERV];		//Сервис к которому клиент подключ
//
//	ZeroMemory(host, NI_MAXHOST);
//	ZeroMemory(service, NI_MAXSERV);
//
//	if (getnameinfo((sockaddr*)& client, clientsize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
//	{
//		std::cout << host << "connected on port " << service << std::endl;
//	}
//	else
//	{
//		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
//		std::cout << host << "connected on port " << ntohs(client.sin_port) << std::endl;
//	}
//
//	//Закрываем прослушиваемый сокет
//	closesocket(listening);
//
//	//Пока зациклен: принимает и отправляет echo сообщение обратно клиенту
//	char buf[2048];
//
//	while (true)
//	{
//		ZeroMemory(buf, 2048);
//
//		//ожидаем данных от клиента
//		int byteRecieved = recv(clientsocket, buf, 2048, 0);
//		if (byteRecieved == SOCKET_ERROR)
//		{
//			std::cerr << "Error in recv(). Exiting." << std::endl;
//			break;
//		}
//
//		if (byteRecieved == 0)
//		{
//			std::cout << "Client dissconnected " << std::endl;
//			break;
//		}
//
//		//echo сообщение обратно клиенту
//		send(clientsocket, buf, byteRecieved + 1, 0);
//		//Вывод сообщения в консоль сервера
//		std::cout << "CLIENT> " << buf << std::endl;
//
//	}
//
//	//Закрываем сокет
//	closesocket(clientsocket);
//}