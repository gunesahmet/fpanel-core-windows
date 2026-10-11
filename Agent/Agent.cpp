#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Socket/TcpServer.h>
#include <Socket/FileServer.h>
#include <Helpers/Helpers.h>
#include <SysInfo/SystemInfo.h>


enum versions { Win2k16, Win2k16R2, Win2k19, Win2k19R2 };

Logger logger;
Config *_config;

std::string debug_log_file = "c:/fpanel/var/log/fpanel/agent-debug.logs";
std::string error_log_file = "c:/fpanel/var/log/fpanel/agent-error.logs";
Helpers helpers;

enum
{
	SecondLog = 1,
	MaxLogFileSize = 104857600 // 100MB
};

void initLogs()
{
	plog::init(plog::debug, debug_log_file.c_str(), MaxLogFileSize);              // Initialize the default logger instance.
	plog::init<SecondLog>(plog::error, error_log_file.c_str(), MaxLogFileSize);   // Initialize the 2nd logger instance.
}

boost::program_options::variables_map commandLine(int argc, char** argv)
{
	boost::program_options::variables_map vm;

	try {
		bool t_given = false;

		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			// First parameter describes option name/short name
			// The second is parameter to option
			// The third is description
			("help,h", boost::program_options::bool_switch(&t_given), "print usage message")
			("debug,d", boost::program_options::bool_switch(&t_given), "Open debug mode")
			("systemd,D", boost::program_options::bool_switch(&t_given), "Run for systemd")
			("quit,q", boost::program_options::bool_switch(&t_given), "Quit from program")
			;

		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help") && (vm["help"].as<bool>())) {
			std::cout << "FPanel Agent Ver 1.0.0, for Linux (amd64)" << std::endl;
			std::cout << "Copyright (c) 2021 Kovengo Software Limited. All rights reserved." << std::endl;
			std::cout << "" << std::endl;
			std::cout << "FPanel is a registered trademark of Kovengo Software Limited. Other names may be trademarks of their respective owners." << std::endl;
			std::cout << "" << std::endl;
			std::cout << "Allowed options:\n"
				"  --help|-h                Print usage message\n"
				"  --systemd|-D             Run for systemd\n"
				"  --quit|-q                Stop process\n" << std::endl;
			exit(0);
		}

	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	return vm;
}

void CheckOs()
{
	_config = new Config();
	SystemInfo systemInfo;
	bool is_server = systemInfo.isServer();
	bool is_valid = false;
	int system_version = systemInfo.getVersion();

	switch (system_version) {
		case versions::Win2k16:
			is_valid = true;
			break;
		case versions::Win2k16R2:
			is_valid = true;
			break;
		case versions::Win2k19:
			is_valid = true;
			break;
		case versions::Win2k19R2:
			is_valid = true;
			break;
	}

	if (!is_valid) {
		helpers.consoleWrite("Your server version is not correct. Supported versions Windows Server 2016-2019 x64/R2/Core", Helpers::DARKYELLOW, true);
		logger.Error("Your server version is not correct. Supported versions Windows Server 2016-2019 x64/R2/Core.");
		exit(EXIT_FAILURE);
	}
}

void Start(int argc, char** argv)
{
	initLogs();
	boost::program_options::variables_map vm = commandLine(argc, argv);

	CheckOs();

	bool debug = false;
	if (vm.count("debug") && (vm["debug"].as<bool>())) {
		helpers.Println("FPanel Agent starting as debug mode.");
		logger.debug = true;
		debug = true;
	}


	if (vm["debug"].as<bool>()) {
		logger.debug = true;
	}
	else {
		if (!vm["systemd"].as<bool>()) {
			ShowWindow(GetConsoleWindow(), SW_HIDE);
		}
	}


	helpers.ExecuteWindows("netsh advfirewall firewall add rule name=\"TCP Port 8887\" dir=in action=allow protocol=TCP localport=8887", true);


	_config = new Config();
	_config->setLogger(logger);
	_config->init();

	Commands *_commands = new Commands();
	_commands->SetLogger(logger)
		.SetDebug(logger.debug)
		.SetConfig(_config);

	std::string f_sock_addr = _config->getValue("file.socket.server.address");
	std::string f_sock_port = _config->getValue("file.socket.server.port");

	std::string sock_addr = _config->getValue("socket.server.address");
	std::string sock_port = _config->getValue("socket.server.port");
	logger.Debug("FPanel Agent starting as file transfer socket %s:%s", f_sock_addr.c_str(), f_sock_port.c_str());

	FileServer file_server;
	file_server
		.SetConfig(_config)
		.SetLogger(logger)
		.SetHost(f_sock_addr)
		.SetPort(std::stoi(f_sock_port));

	boost::thread* fs_thread = new boost::thread(boost::bind(&FileServer::Start, &file_server));
	fs_thread->detach();

	logger.Debug("FPanel Agent starting %s:%s", sock_addr.c_str(), sock_port.c_str());
	TcpServer tcp_server;
	tcp_server.SetConfig(_config)
		.SetCommands(_commands)
		.SetLogger(logger)
		.SetHost(sock_addr)
		.SetPort(std::stoi(sock_port))
		.Start();

}


int __cdecl main(int argc, char** argv)
{
	Start(argc, argv);
	return 0;
}