#include <sys/stat.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>
#include <Helpers.h>
#include <Crypto.h>

Helpers _helpers;
Crypto _crypto;
bool debug = false;
bool encode = false;
bool decode = false;

boost::program_options::variables_map commandLine(int argc, char *argv[])
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
("verbose,v", boost::program_options::bool_switch(&t_given), "Open verbose mode")
("encode,e", boost::program_options::bool_switch(&t_given), "Encode input")
("decode,d", boost::program_options::bool_switch(&t_given), "Decode input")
("in,i", boost::program_options::value<std::string>(), "Input")
;

		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	return vm;
}

int main(int argc, char *argv[])
{
	std::string secret_key = "87f5e1c8c1dd33cfd9dbf2990b60d595";
	std::string secret_iv = "87f5e1c8c1dd33cf";

	boost::program_options::variables_map vm = commandLine(argc, argv);

	if (vm["verbose"].as<bool>()) {
		debug = true;
	}
	if (vm["encode"].as<bool>()) {
		encode = true;
	}
	if (vm["decode"].as<bool>()) {
		decode = true;
	}
	std::string in;

	if (vm.count("in")) {
		std::string word;
		std::string first_key;

		in = vm["in"].as<std::string>();

		if (debug) {
			if (decode) {
				std::cout << "Decoding" << std::endl;
			}
			else if (encode) {
				std::cout << "Encoding" << std::endl;
			}
			std::cout << "Input: " << in << std::endl;
		}
		std::string result;
		if (decode) {
			_helpers.Write("c:\\fpanel\\tmp\\request.txt", in);
			result = _crypto.Decrypt(std::string(in), secret_key, secret_iv);
			_helpers.Write("c:\\fpanel\\tmp\\result.txt", result);
		}
		else if (encode) {
			result = _crypto.Encrypt(std::string(in), secret_key, secret_iv);
		}
		std::cout << result << std::endl;
	}
	return 0;
}
