#pragma once
#ifndef FPANEL_AGENT_FILE_SERVER_H
#define FPANEL_AGENT_FILE_SERVER_H
#undef UNICODE

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <thread>
#include <vector>
#include <list>


#include <functional>

#include <errno.h>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>

#include <Logger/Logger.h>
#include <Helpers/Helpers.h>
#include <Config/Config.h>
#include <Crypto/Crypto.h>

#include <Rapidjson/document.h>
#include <Rapidjson/writer.h>
#include <Rapidjson/stringbuffer.h>
#include <list>
#include <fstream>


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

using namespace rapidjson;
class FileServer
{
	private:
	std::string _host;
	int _port;
	Logger _logger;
	Config *_config;
	Crypto _crypto;
	Helpers _helpers;
	std::string panel_conf = "c:/fpanel/etc/panel.conf";

	char* getipaddr(int sock)
	{
		sockaddr_in SockAddr;
		struct sockaddr_in addr;
		socklen_t addr_size = sizeof(struct sockaddr_in);
		int res = getpeername(sock, (struct sockaddr *)&addr, &addr_size);
		char *client_ip = new char[20];
		strcpy(client_ip, inet_ntoa(addr.sin_addr));
		int port = ntohs(addr.sin_port);
		_logger.Debug("IP: %s Connected On PORT: %d", client_ip, port);
		return client_ip;
	}

	void readFile(int client_sock, std::string file_name, std::string file_path, std::string file_type)
	{
		std::string rcv;
		int bytes_received = 0;
		const int MAX_BUF_LENGTH = 4096; // 32KB
		char buffer[MAX_BUF_LENGTH];
		char err[100];

		std::string base_directory = file_path;
		_helpers.ReplaceWithString(base_directory, file_name, "");
		if (!_helpers.IsDirExist(base_directory)) {
			_helpers.CreateDirectory(base_directory);
		}

		std::string write_file_path = base_directory + "/" + file_name;
		std::ofstream fout;
		fout.open(write_file_path.c_str(), std::ios::binary | std::ios::out);

		while (true) {
			bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
			std::cout << "bytes_received: " << bytes_received << std::endl;
			if (bytes_received < 0) {
				_logger.Debug("Receiving data failed");
				break;
			}
			else if (bytes_received == 0) {
				_logger.Debug("Client disconnected");
				break;
			}
			else {
				fout.write(buffer, bytes_received);
				if (bytes_received == 0) {
					break;
				}
			}
		}
		fout.close();
		_logger.Debug("File wrote");
	}

	int connectionHandler(void *socket_desc)
	{
		int client_sock = *(int*)socket_desc;

		std::string rcv;
		int bytes_received = 0;
		const int MAX_BUF_LENGTH = 4096;
		char buffer[MAX_BUF_LENGTH];
		char err[100];

		while (true) {
			bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
			if (bytes_received < 0) {
				_logger.Debug("Receiving data failed");
				break;
			}
			else if (bytes_received == 0) {
				_logger.Debug("Client disconnected");
				break;
			}
			else {
				rcv.append(buffer, bytes_received);
				if (bytes_received < MAX_BUF_LENGTH) {
					break;
				}
			}
		}
		std::string response;
		_helpers.trim(rcv);
		std::string command;
		bool with_auth = true;
		bool validated = false;
		_logger.Debug("Received data: %s", rcv.c_str());
		if (_helpers.IsFileExists(panel_conf)) {
			command = _crypto.Decrypt(rcv, _config->getValue("secret.key"), _config->getValue("secret.iv"));
		}
		else {
			command = _crypto.Decrypt(rcv, _config->getValue("socket.key"), _config->getValue("socket.iv"));
		}
		_logger.Debug("Received Command: %s", command.c_str());

		if (command.length() == 0) {
			_logger.Debug("Command length is 0");
			response = R"({"result":"failed", "message": "Invalid command"})";
			Send(client_sock, response);
			close(client_sock);
			return 1;
		}

		Document d;
		const char *json = command.c_str();
		if (!d.Parse(json).HasParseError()) {
			if (d.HasMember("job")) {
				if (d.HasMember("username") && d.HasMember("password")) {
					if (validateUser(d["username"].GetString(), d["password"].GetString())) {
						std::string job = d["job"].GetString();
						if (std::string(job).compare("file_transfer") == 0) {
							std::string file_name = d["file_name"].GetString();
							std::string file_path = d["file_path"].GetString();
							std::string file_type = d["file_type"].GetString();
							response = R"({"result":"success"})";
							_logger.Debug("Response: %s", response.c_str());
							Send(client_sock, response);
							readFile(client_sock, d["file_name"].GetString(), d["file_path"].GetString(), d["file_type"].GetString());
						}
					}
					else {
						response = R"({"result":"failed", "message": "Invalid credentials"})";
						_logger.Debug("Invalid credential. Received: %s", command.c_str());
						Send(client_sock, response);
					}
				}
			}
			else {
				response = R"({"result":"failed", "message": "Missing parameters"})";
				_logger.Debug("Missing parameters. Received: %s", command.c_str());
				Send(client_sock, response);
			}

		}
		else {
			_logger.Debug("Invalid command. Received: %s", command.c_str());
			response = R"({"result":"failed", "message": "Invalid command"})";
			Send(client_sock, response);
		}

		_logger.Debug("Socket closing");
		_logger.Debug("=============================");
		rcv.clear();
		close(client_sock);
		return 0;
	}

	bool validateUser(std::string username, std::string password)
	{
		return true;
	}

	public:
	FileServer& SetLogger(Logger logger)
	{
		_logger = logger;
		return *this;
	}

	FileServer& SetConfig(Config *config)
	{
		_config = config;
		return *this;
	}

	FileServer& SetHost(std::string host)
	{
		_host = host;
		return *this;
	}

	FileServer& SetPort(int port)
	{
		_port = port;
		return *this;
	}

	void Send(int client_sock, std::string &response)
	{
		std::string crypted_response;
		if (_helpers.IsFileExists(panel_conf)) {
			crypted_response = _crypto.Encrypt(response, _config->getValue("secret.key"), _config->getValue("secret.iv")) + "\n";
		}
		else {
			crypted_response = _crypto.Encrypt(response, _config->getValue("socket.key"), _config->getValue("socket.iv")) + "\n";
		}

		if (send(client_sock, crypted_response.c_str(), strlen(crypted_response.c_str()), 0) < 0) {
			_logger.Error("Send failed to client");
		}
		else {
			_logger.Debug("Response sent");
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

		while (true) {
			try {
				int new_socket = accept(sock, (struct sockaddr *)&client, &sock_size);
				getipaddr(new_socket);
				boost::thread clientThread(&FileServer::connectionHandler, this, (void*)&new_socket);
				clientThread.detach();
			}
			catch (const std::exception& e) {
				_logger.Error("Error : Socket accept. Exception: %s", std::string(e.what()).c_str());
				continue;
			}
		}

		getchar();
		closesocket(sock);
		WSACleanup();
		return 0;
	}
};
#endif//FPANEL_AGENT_FILE_SERVER_H