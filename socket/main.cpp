#define WIN32_LEAN_AND_MEAN 1

#include <csignal>
#include <sys/stat.h>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>
#include "Logger/Logger.h"
#include "Config/Config.h"
#include "Socket/TcpServer.h"
#include "Maps/Licence.h"
#include "Helpers/Helpers.h"

Logger logger;
Config *config;

std::string debug_log_file = "c:/fpanel/var/log/fpanel/debug.logs";
std::string error_log_file = "c:/fpanel/var/log/fpanel/error.logs";
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
		bool b_given = false;

		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			// First parameter describes option name/short name
			// The second is parameter to option
			// The third is description
//                ("help,h", boost::program_options::bool_switch(&t_given), "print usage message")
			("debug,d", boost::program_options::bool_switch(&t_given), "Open debug mode")
			("server,s", boost::program_options::bool_switch(&t_given), "Start as Server")
			("config,c", boost::program_options::value<std::string>(), "Configuration file location")
			;

		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help") && vm.count("debug")) {
			//            std::cout << desc << std::endl;
			exit(1);
		}

	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	return vm;
}


void Start(int argc, char** argv)
{
	config = new Config();
	boost::program_options::variables_map vm = commandLine(argc, argv);

	if (vm["debug"].as<bool>()) {
		logger.debug = true;
	}
	else {
		
	}
	// Initialize Logger
	initLogs();
	config->setLogger(logger);

	config->init();

	if (vm["server"].as<bool>()) {
		Licence *_licence_map = new Licence();
		Domains *_domain_map = new Domains();
		_domain_map->loadDomains(config);

		Commands commands;
		commands.SetLogger(logger)
			.SetConfig(config)
			.SetLicenceMap(_licence_map)
			.SetDomainsMap(_domain_map);
		//helpers.CmdOutput(config->getValue("root_bin_path") + "networkmng --init");

		logger.Debug("Server starting " + config->getValue("socket.server.address") + ":" + config->getValue("socket.server.port"));
		//commands.Run("licencemng --force-check");
		//commands.BackendLoops();
		TcpServer tcpServer;
		tcpServer.SetConfig(config)
			.SetLogger(logger)
			.SetHost(config->getValue("socket.server.address"))
			.SetPort(std::stoi(config->getValue("socket.server.port")))
			.SetCommands(commands)
			.SetDomainsMap(_domain_map)
			.Start();
	}
}

int main(int argc , char *argv[])
{
	Start(argc, argv);
	return 0;
}