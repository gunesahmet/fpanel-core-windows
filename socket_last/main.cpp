#define WIN32_LEAN_AND_MEAN 1

#include <ctime>
#include <cstdio>
#include <memory>
#include <cstdint>
#include <string>
#include <thread>
#include <list>
#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>

#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <atlstr.h>

#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet.lib")

#include <ws2tcpip.h>
#include <intrin.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include <vector>

int connectionHandler(void *socket_desc)
{
	int clientSock = *(int*)socket_desc;
	std::string rcv;
	int bytesReceived = 0;
	const int MAX_BUF_LENGTH = 4096;
	//char buffer[MAX_BUF_LENGTH];

	std::vector<char> buffer(MAX_BUF_LENGTH);

	while (true) {
		bytesReceived = recv(clientSock, buffer.data(), buffer.size(), 0);
		//bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
		if (bytesReceived == -1) {
			closesocket(clientSock);
			return -1;
		}
		else {
			rcv.append(buffer.cbegin(), buffer.cend());
		}
	}


	//std::string command = _crypto.Decrypt(rcv, _config->getValue("secret.key"), _config->getValue("secret.iv"));
	//if (command.length() == 0) {
	//	_logger.Debug("Command: " + rcv);
	//	_logger.Debug("Command length is 0");
	//}
	//else if (_helpers.commandIsValid(command)) {
	//	_logger.Debug("Command is valid");
	//	_logger.Debug("Command: " + rcv);
	//	//initializeRequest(command, clientSock);
	//}
	//else {
	//	_logger.Debug("Not valid Command: " + command);
	//	Document d;
	//	const char *json = command.c_str();
	//	if (!d.Parse(json).HasParseError()) {
	//		if (d.HasMember("job")) {
	//			std::string job = d["job"].GetString();
	//			if (d.HasMember("domain_name")) {
	//				if (std::string(job).compare("add-domain") == 0) {
	//					_logger.Debug("Domain adding");
	//					std::string ip_addr = std::string(d["ip_addr"].GetString());
	//					std::string domain_name = std::string(d["domain_name"].GetString());
	//					_domain_map->addDomain(domain_name, ip_addr);
	//				}
	//				else if (std::string(job).compare("delete-domain") == 0) {
	//					std::string domain_name = std::string(d["domain_name"].GetString());
	//					_domain_map->removeDomain(domain_name);
	//				}
	//			}
	//		}
	//	}
	//	else {
	//		_logger.Debug("Recived: " + rcv);
	//		_logger.Debug("Invalid command");
	//	}
	//}

	//_logger.Debug("=============================");
	//_logger.Debug("=============================");
	//_logger.Debug("=============================");
	//rcv.clear();
	//_logger.Debug("Socket client closing");

	buffer.assign(buffer.size(), 0);
	rcv.clear();
	closesocket(clientSock);
	WSACleanup();
	//_logger.Debug("Socket client closed");
	return 0;
}

int main(int argc, char** argv)
{
	WSADATA wsa;
	SOCKET sock, new_socket;
	struct sockaddr_in server;
	int sock_size;
	char *message;
	memset(&server, 0, sizeof(server));

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	//Create a socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(8889);

	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
		return 1;
	}
	struct sockaddr clientAddress;
	socklen_t sizeOfClientAddress = sizeof(clientAddress);

	listen(sock, 5);
	sock_size = sizeof(struct sockaddr_in);

	while (1) {
		int new_socket = accept(sock, (struct sockaddr *)&clientAddress, &sizeOfClientAddress);
		std::thread t(connectionHandler, (void*)&new_socket);
		t.detach();
	}

	getchar();
	closesocket(sock);
	WSACleanup();
	return 0;
}
