#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Socket/TcpServer.h>
#include <Maps/Licence.h>
#include <Helpers/Helpers.h>

Logger logger;
Config *_config;
Commands *_commands;
Licence *_licence_map;
LicenceRequests *_licence_request_map;

std::string debug_log_file = "c:/fpanel/var/log/fpanel/core-debug.logs";
std::string error_log_file = "c:/fpanel/var/log/fpanel/core-error.logs";
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
		bool flag = false;

		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			// First parameter describes option name/short name
			// The second is parameter to option
			// The third is description	
			("help", "Print usage message")
			("debug,d", boost::program_options::bool_switch(&flag)->default_value(false), "Open debug mode")
			("server,s", boost::program_options::bool_switch(&flag)->default_value(false), "Start as Server")
			("config,c", boost::program_options::value<std::string>(), "Configuration file location");

		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help") && (vm["help"].as<bool>())) {
			std::cout << "FPanel Core Ver 1.0.0, for Windows (amd64)" << std::endl;
			std::cout << "Copyright (c) 2021 Kovengo Software Limited. All rights reserved." << std::endl;
			std::cout << "" << std::endl;
			std::cout << "FPanel is a registered trademark of Kovengo Software Limited. Other names may be trademarks of their respective owners." << std::endl;
			std::cout << "" << std::endl;
			std::cout << "Allowed options:\n"
				"  --help                Print usage message\n"
				"  --server              Start as Server\n"
				"  --config              Configuration file location\n"<< std::endl;
			exit(1);
		}

	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	return vm;
}

void licenceRequestLoop()
{
	int min_time = 21600;
	int max_time = 43200;

	std::string request = _commands->GenerateLicenceRequest();

	while (1) {
		int rand_time = rand() % (max_time - min_time + 1) + min_time;
		_commands->LicenceRequest(request);
		helpers.SecSleep(rand_time);
	}
}

int __cdecl main(int argc, char** argv)
{
	_config = new Config();
	boost::program_options::variables_map vm = commandLine(argc, argv);

	if (vm.count("debug") && (vm["debug"].as<bool>())) {
		logger.debug = true;
	}
	else {
		ShowWindow(GetConsoleWindow(), SW_HIDE);
	}

	initLogs();
	_config->init();

	_licence_map = new Licence();
	LicenceRequests *_licence_request_map = new LicenceRequests();
	//Domains *_domain_map = new Domains();
	//_domain_map->loadDomains(_config);

	logger.Debug("Network initializing");
	helpers.CmdOutput(_config->getValue("root_bin_path") + "networkmng --init");

	_commands = new Commands();
	_commands->SetLogger(logger)
		.SetConfig(_config)
		.SetLicenceMap(_licence_map)
		.SetLicenceRequests(_licence_request_map);
		//.SetDomainsMap(_domain_map);

	logger.Debug("Licence checking");
	_commands->Run("licencemng --force-check");
	_commands->BackendLoops();

	logger.Debug("Licence loop starting");
	//licenceRequestLoop();
	std::thread tlq(licenceRequestLoop);
	tlq.detach();

	logger.Debug("Server starting " + _config->getValue("socket.server.address") + ":" + _config->getValue("socket.server.port"));
	TcpServer tcpServer;
	tcpServer.SetConfig(_config)
		.SetCommands(_commands)
		.SetLogger(logger)
		//.SetDomainsMap(_domain_map)
		.SetHost(_config->getValue("socket.server.address"))
		.SetPort(std::stoi(_config->getValue("socket.server.port")))
		.Start();
	return 0;
}