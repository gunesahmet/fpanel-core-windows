#pragma once
#ifndef FPANEL_TCP_CLIENT_H
#define FPANEL_TCP_CLIENT_H

#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <Logger/Logger.h>
#include <Helpers/Helpers.h>
#include <Config/Config.h>
#include <Crypto/Crypto.h>

class TcpClient {
private:
    Crypto _crypto;
    Logger _logger;
    Config *_config;
    Helpers _helpers;
	SOCKET sock_server = INVALID_SOCKET;
    std::string srv_addr;
    std::string _response;
    int srv_port;

public:
    void SetLogger(Logger logger)
    {
        _logger = logger;
    }

    void SetConfig(Config *config)
    {
        _config = config;
    }

    void SetServer(std::string _s_addr, int _s_port)
    {
		srv_addr = _s_addr;
		srv_port = _s_port;
    }

    bool Send(std::string command)
    {
		int iResult;
        std::string request = command;
        request += "\n";

		// Send an initial buffer
		iResult = send(sock_server, request.c_str(), (int)strlen(request.c_str()), 0);
		if (iResult < 0) {
			_logger.Error("Send message failed to licence server");
			closesocket(sock_server);
			WSACleanup();
			return false;
		}
		else {
			_logger.Debug("Send successful message: " + command);
		}
        return true;
    }

    void RecvResponse()
    {
		std::string rcv;
		int bytesReceived = 0;
		const int MAX_BUF_LENGTH = 256;
		char buffer[MAX_BUF_LENGTH];

		while (true) {
			bytesReceived = recv(sock_server, buffer, sizeof(buffer), 0);
			if (bytesReceived < 0) {
				_logger.Debug("Receiving data failed from licence server");
				break;
			}
			else if (bytesReceived == 0) {
				_logger.Debug("Connection disconnected from licence server");
				break;
			}
			else {
				rcv.append(buffer, bytesReceived);
				if (bytesReceived < MAX_BUF_LENGTH) {
					break;
				}
			}
		}

        _response = rcv;
    }

    std::string Response()
    {
//    std::string response = _crypto.Decrypt(_response, _config->getValue("secret.key"), _config->getValue("secret.iv"));
        std::string response = _response;
        _response.clear();
        return response;
    }

    bool Close()
    {
		int iResult;
		iResult = shutdown(sock_server, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(sock_server);
			WSACleanup();
		}
		_logger.Debug("Closed socket");
		return true;
    }

    bool Connect(std::string request)
    {
		WSADATA wsaData;

		struct addrinfo hints = {}, *addrs;

		int iResult;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			_logger.Debug("Burda 2 false");
			return false;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port

		struct hostent *host;
		if ((host = gethostbyname(srv_addr.c_str())) == nullptr)
		{
			//More descriptive error message?
			perror(srv_addr.c_str());
			return false;
		}

		iResult = getaddrinfo(srv_addr.c_str(), std::to_string(srv_port).c_str(), &hints, &addrs);
		if (iResult != 0)
		{
			fprintf(stderr, "%s: %s\n", srv_addr, gai_strerror(iResult));
			abort();
		}


		// Create a SOCKET for connecting to server
		sock_server = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
		if (sock_server == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return false;
		}

		// Connect to server.
		iResult = connect(sock_server, addrs->ai_addr, (int)addrs->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(sock_server);
			sock_server = INVALID_SOCKET;
			return false;
		}


		if (sock_server == INVALID_SOCKET) {
			printf("Unable to connect to server!\n");
			WSACleanup();
			return false;
		}
		return true;
    }
};


#endif //FPANEL_TCP_CLIENT_H
