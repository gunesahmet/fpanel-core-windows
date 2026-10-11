#pragma once
#ifndef FPANEL_CORE_TCPSERVER_H
#define FPANEL_CORE_TCPSERVER_H
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <Crypto/Crypto.h>
#include <Commands/Commands.h>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Maps/Domains.h>
#include <boost/thread.hpp>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512

class TcpServer {
private:
    std::string _host;
    int _port;
	Logger _logger;
	Config *_config;
	Crypto _crypto;
	Helpers _helpers;
	Commands *_commands;
	Domains *_domain_map;

    std::string getipaddr(int newfd)
    {
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        int res = getpeername(newfd, (struct sockaddr *)&addr, &addr_size);
        char *clientip = new char[20];
        strcpy(clientip, inet_ntoa(addr.sin_addr));
        int port = ntohs(addr.sin_port);
		std::cout << "[IP: " + std::string(clientip) + " Connected On PORT: " + std::to_string(port) + "]" << std::endl;
        return std::string(clientip);
    }

    int connectionHandler(void *socket_desc)
    {
		int clientSock = *(int*)socket_desc;
		std::string rcv;
		int bytesReceived = 0;
		char buffer[DEFAULT_BUFLEN];
		char err[100];

		while (true) {
			bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
			if (bytesReceived < 0) {
				sprintf(err, "Receiving data failed\n");
				_logger.Error(err);
				return CloseSocket(clientSock);
				break;
			}
			else if (bytesReceived == 0) {
				sprintf(err, "Client disconnected\n");
				//_logger.Debug(err);
				break;
			}
			else {
				rcv.append(buffer, bytesReceived);
				if (bytesReceived < DEFAULT_BUFLEN) {
					//sprintf(err, "End of data\n");
					//_logger.Debug(err);
					break;
				}
			}
		}
		std::string command = _crypto.Decrypt(rcv, _config->getValue("secret.key"), _config->getValue("secret.iv"));
		if (command.length() == 0) {
			_logger.Error("Command length is 0");
			CloseSocket(clientSock);
			return 1;
		}

		if (_helpers.commandIsValid(command)) {
			_logger.Debug("Command is valid");
			initializeRequest(command, clientSock);
		}
		else {
			_logger.Error("Not valid Command: " + command);
			Document d;
			const char *json = command.c_str();
			if (!d.Parse(json).HasParseError()) {
				if (d.HasMember("job")) {
					std::string job = d["job"].GetString();
					if (d.HasMember("domain_name")) {
						if (std::string(job).compare("add-domain") == 0) {
							_logger.Debug("Domain adding");
							std::string ip_addr = std::string(d["ip_addr"].GetString());
							std::string domain_name = std::string(d["domain_name"].GetString());
							//_domain_map->addDomain(domain_name, ip_addr);
						}
						else if (std::string(job).compare("delete-domain") == 0) {
							std::string domain_name = std::string(d["domain_name"].GetString());
							//_domain_map->removeDomain(domain_name);
						}
					}
				}
			}
			else {
				_logger.Error("Invalid command. Recived: " + rcv);
			}
		}
		_logger.Debug("Socket closed");
		_logger.Debug("Message cleared");
		_logger.Debug("=============================");
		_logger.Debug("=============================");
		_logger.Debug("=============================");
		rcv.clear();
		_logger.Debug("Socket client closing");
		CloseSocket(clientSock);
		_logger.Debug("Socket client closed");

		return 0;
    }

	int CloseSocket(int clientSock)
	{
		int iResult;
		iResult = shutdown(clientSock, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			char err[100];
			sprintf(err, "shutdown failed with error: %d\n", WSAGetLastError());
			_logger.Error(err);
			closesocket(clientSock);
			WSACleanup();
		}
		return 0;
	}

    void initializeRequest(std::string command, int clientSock)
    {
		std::string response = _commands->Run(command);
        Send(clientSock, response);
    }
public:

	TcpServer& SetLogger(Logger logger)
	{
		_logger = logger;
		return *this;
	}

	TcpServer& SetConfig(Config *config)
	{
		_config = config;
		return *this;
	}

	TcpServer& SetHost(std::string host)
	{
		_host = host;
		return *this;
	}

	TcpServer& SetDomainsMap(Domains *domain_map)
	{
		_domain_map = domain_map;
		return *this;
	}

	TcpServer& SetCommands(Commands *commands)
	{
		_commands = commands;
		return *this;
	}

	TcpServer& SetPort(int port)
	{
		_port = port;
		return *this;
	}

    void Send(int clientSock, std::string &response)
    {
        std::string crypted_response = _crypto.Encrypt(response, _config->getValue("secret.key"), _config->getValue("secret.iv")) + "\n";
        if(send(clientSock, crypted_response.c_str(), strlen(crypted_response.c_str()), 0) < 0) {
            _logger.Error("Send failed to client");
        }
    }

    int Start()
    {
		WSADATA wsa;
		SOCKET sock;
		struct sockaddr_in server, client;
		int sock_size;
		char err[100];
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			sprintf(err, "Failed. Error Code : %d", WSAGetLastError());
			_logger.Error(err);
			return 1;
		}

		//Create a socket
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			sprintf(err, "Could not create socket : %d", WSAGetLastError());
			_logger.Error(err);
		}

		server.sin_family = AF_INET;
		server.sin_addr.s_addr = inet_addr(_host.c_str());

		//server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(_port);

		if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
			sprintf(err, "Bind failed with error code : %d", WSAGetLastError());
			_logger.Error(err);
		}
		listen(sock, 3);
		sock_size = sizeof(struct sockaddr_in);

		while (1) {
			int new_socket = accept(sock, (struct sockaddr *)&client, &sock_size);
			std::string connected_ip = getipaddr(new_socket);
			if (connected_ip.compare(_config->getValue("socket.allowed.address")) == 0 || connected_ip.compare("127.0.0.1") == 0) {
				_logger.Debug("Client allowed");
				boost::thread clientThread(&TcpServer::connectionHandler, this, (void*)&new_socket);
				clientThread.detach();
			}
			else {
				CloseSocket(new_socket);
				_logger.Debug("Client disallowed");
			}
		}

		getchar();
		closesocket(sock);
		WSACleanup();
		return 0;
	}
};
#endif