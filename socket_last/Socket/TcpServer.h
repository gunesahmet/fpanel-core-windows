#include <thread>
#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet.lib")
#include <ws2tcpip.h>

#include <Logger/Logger.h>
#include <Helpers/Helpers.h>
#include <Config/Config.h>
#include <Crypto/Crypto.h>
#include <Commands/Commands.h>

#include <Rapidjson/document.h>
#include <Rapidjson/writer.h>
#include <Rapidjson/stringbuffer.h>

using namespace rapidjson;

class TcpServer {
private:
    std::string _host;
    int _port;
    Logger _logger;
    Config *_config;
    Crypto _crypto;
    Helpers _helpers;
    Commands _commands;
    Domains *_domain_map;

    char* getipaddr(int newfd)
    {
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        int res = getpeername(newfd, (struct sockaddr *)&addr, &addr_size);
        char *clientip = new char[20];
        strcpy(clientip, inet_ntoa(addr.sin_addr));
        int port = ntohs(addr.sin_port);
        _logger.Info("[IP: " + std::string(clientip) + " Connected On PORT: " + std::to_string(port) + "]");
        return clientip;
    }

    int connectionHandler(void *socket_desc)
    {
		int clientSock = *(int*)socket_desc;
		std::string rcv;
		int bytesReceived = 0;
		const int MAX_BUF_LENGTH = 4096;
		char buffer[MAX_BUF_LENGTH];

		while (true) {
			bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
			if (bytesReceived < 0) {
				_logger.Debug("Receiving data failed");
				closesocket(clientSock);
				WSACleanup();
				return -1;
			}
			else if (bytesReceived == 0) {
				_logger.Debug("End of data");
				break;
			}
			else {
				rcv.append(buffer, bytesReceived);
				if (bytesReceived < MAX_BUF_LENGTH) {
					_logger.Debug("End of data");
					break;
				}
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
		
		closesocket(clientSock);
		WSACleanup();
		_logger.Debug("Socket client closed");
		return 0;
    }

    void initializeRequest(std::string command, int clientSock)
    {
//        boost::uuids::uuid client_uuid = boost::uuids::random_generator()();
//        std::string client_id = boost::uuids::to_string(client_uuid);
//        _helpers.SecSleep(5);
        std::string response = _commands.Run(command.c_str());
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

    TcpServer& SetCommands(Commands commands)
    {
        _commands = commands;
        return *this;
    }

    TcpServer& SetDomainsMap(Domains *domain_map)
    {
        _domain_map = domain_map;
        return *this;
    }

    TcpServer& SetPort(int port)
    {
        _port = port;
        return *this;
    }

    void Send(int clientSock, std::string &response)
    {
        unsigned int error_code;
        unsigned int error_code_size = sizeof(error_code);
        std::string crypted_response = _crypto.Encrypt(response, _config->getValue("secret.key"), _config->getValue("secret.iv")) + "\n";
        if(send(clientSock, crypted_response.c_str(), strlen(crypted_response.c_str()), 0) < 0) {
            _logger.Error("Send failed to client");
        }
    }

    SOCKET Start()
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
			return -1;
		}

		// For Non-Blocking
		//u_long arg = 1;
		//ioctlsocket(sock, FIONBIO, &arg);

		server.sin_family = PF_INET;
		server.sin_addr.s_addr = inet_addr(_host.c_str());
		//server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(_port);

		if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
			printf("Bind failed with error code : %d", WSAGetLastError());
			return -1;
		}
		listen(sock, 5);
		sock_size = sizeof(struct sockaddr_in);

		
		return sock;
	}
};
