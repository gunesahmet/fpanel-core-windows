#pragma once
#ifndef FPANEL_CORE_TCPSERVER_H
#define FPANEL_CORE_TCPSERVER_H
#undef UNICODE
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
#include <boost/thread.hpp>

#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")


class TcpServer {
private:
    std::string _host;
    int _port;
	Logger _logger;
	Config *_config;
	Crypto _crypto;
	Helpers _helpers;
	Commands *_commands;

    char* getipaddr(int newfd)
    {
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        int res = getpeername(newfd, (struct sockaddr *)&addr, &addr_size);
        char *clientip = new char[20];
        strcpy(clientip, inet_ntoa(addr.sin_addr));
        int port = ntohs(addr.sin_port);
		std::cout << "[IP: " + std::string(clientip) + " Connected On PORT: " + std::to_string(port) + "]" << std::endl;
        return clientip;
    }

    int connectionHandler(void *socket_desc)
    {
		int clientSock = *(int*)socket_desc;
		std::string rcv;
		int bytesReceived = 0;
		const int MAX_BUF_LENGTH = 1024;
		char buffer[MAX_BUF_LENGTH];
		char err[100];

		while (true) {
			bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
			if (bytesReceived < 0) {
				_logger.Debug("Receiving data failed");
				break;
			}
			else if (bytesReceived == 0) {
				_logger.Debug("Client disconnected");
				break;
			}
			else {
				rcv.append(buffer, bytesReceived);
				if (bytesReceived < MAX_BUF_LENGTH) {
					break;
				}
			}
		}
		_helpers.trim(rcv);
		std::string command;
		bool with_auth = true;
		bool validated = false;
		//if (_config->getValue("secret.key").length() > 0) {
		//	command = _crypto.Decrypt(rcv, _config->getValue("secret.key"), _config->getValue("secret.iv"));
		//	with_auth = false;
		//	validated = true;
		//}
		//else {
		//	command = _crypto.Decrypt(rcv, _config->getValue("socket.key"), _config->getValue("socket.iv"));
		//}
		command = _crypto.Decrypt(rcv, _config->getValue("socket.key"), _config->getValue("socket.iv"));

		if (command.length() == 0) {
			_logger.Error("Command length is 0");
			CloseSocket(clientSock);
			return 1;
		}

		Document d;
		const char *json = command.c_str();
		if (!d.Parse(json).HasParseError()) {
			if (d.HasMember("job")) {
				if (with_auth && d.HasMember("username") && d.HasMember("password")) {
					if (validateUser(d["username"].GetString(), d["password"].GetString()) == true) {
						validated = true;
					}
				}
				if (validated) {
					std::string job = d["job"].GetString();
					if (std::string(job).compare("exec") == 0) {
						if (d.HasMember("command")) {
							_logger.Debug("Executing command");
							initializeRequest(d["command"].GetString(), d["parameters"].GetString(), clientSock);
						}
					}
				}
				else {
					_logger.Debug("Authentication failed");
				}
			}
		}
		else {
			_logger.Error("Invalid command. Received: " + command);
		}


		_logger.Debug("=============================");
		_logger.Debug("=============================");
		_logger.Debug("=============================");
		rcv.clear();
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

    void initializeRequest(std::string command, std::string parameters, int clientSock)
    {
		// 1. SystemInfo
		// 2. Download
		// 3. Install
		// 4. Get Configs
		// 5. Reboot Machine
		// 6. Init
		std::string response = _commands->Run(command, parameters);
        Send(clientSock, response);
    }

	//bool validateUser(TCHAR * username, TCHAR * password)
	bool validateUser(std::string username, std::string password)
	{
		try {
			std::string domain = "LocalService";
			HANDLE phToken;
			bool status = false;
			status = LogonUserA(username.c_str(), domain.c_str(), password.c_str(), LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &phToken);
			return status;
		}
		catch (...) {
			return false;
		}


	/*	TCHAR * tstrDomain = "LocalService";
		DWORD dwLogonType = LOGON32_LOGON_INTERACTIVE;
		DWORD dwLogonProvider = LOGON32_PROVIDER_DEFAULT;
		HANDLE hToken = NULL;
		int result = LogonUser(username, NULL, password, dwLogonType, dwLogonProvider, &hToken);

		std::cout << "result: " << result << std::endl;
		if (!result) {
			return false;
		}
		return true;*/
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
		//if (_config->getValue("secret.key").length() > 0) {
		//	crypted_response = _crypto.Encrypt(response, _config->getValue("secret.key"), _config->getValue("secret.iv")) + "\n";
		//}
		//else {
		//	crypted_response = _crypto.Encrypt(response, _config->getValue("socket.key"), _config->getValue("socket.iv")) + "\n";
		//}
		std::string crypted_response = _crypto.Encrypt(response, _config->getValue("socket.key"), _config->getValue("socket.iv")) + "\n";

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
			getipaddr(new_socket);
			boost::thread clientThread(&TcpServer::connectionHandler, this, (void*)&new_socket);
			clientThread.detach();
		}

		getchar();
		closesocket(sock);
		WSACleanup();
		return 0;
	}
};
#endif