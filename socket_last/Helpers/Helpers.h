#pragma once
#ifndef LICENCE_HELPERS_H
#define LICENCE_HELPERS_H

#include <iostream>
#include <sys/stat.h>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <sys/timeb.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <initializer_list>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <vector>
#include <iomanip>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <random>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include <Helpers/md5.h>


#define DELIM "."

using namespace boost::asio;
using namespace boost::asio::ip;
class Helpers {
private:
public:

    unsigned int RandomChar()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        return dis(gen);
    }

    std::string GenerateHex(const unsigned int len)
    {
        std::stringstream ss;
        for (auto i = 0; i < len; i++) {
            const auto rc = RandomChar();
            std::stringstream hexstream;
            hexstream << std::hex << rc;
            auto hex = hexstream.str();
            ss << (hex.length() < 2 ? '0' + hex : hex);
        }
        return ss.str();
    }

    std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        return ltrim(rtrim(str, chars), chars);
    }

    bool ReplaceWithString(std::string& str, const std::string& oldstr, const std::string& newstr)
    {
        size_t start_pos = str.find(oldstr);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, oldstr.length(), newstr);

        size_t start_pos_s = str.find(oldstr);
        if(start_pos_s != std::string::npos)
            return ReplaceWithString(str, oldstr, newstr);
        return true;
    }

    bool CmdOptionExists(char **begin, char **end, const std::string& option)
    {
        return find(begin, end, option) != end;
    }

    bool IsDirExist(const std::string& path)
    {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            return false;
        }
        return (info.st_mode & S_IFDIR) != 0;
    }

    std::vector<std::string> strSplit(std::string strToSplit, char delimeter)
    {
        std::stringstream ss(strToSplit);
        std::string item;
        std::vector<std::string> splittedStrings;
        while (std::getline(ss, item, delimeter)) {
            splittedStrings.push_back(std::string(item));
        }
        return splittedStrings;
    }

    uint64_t Milliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    uint64_t Microseconds()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    uint64_t Nanoseconds()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    void MilliSleep(int millis)
    {
        if(millis == 0) {
            millis = 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }

    void SecSleep(int seconds)
    {
        if(seconds == 0) {
            seconds = 1;
        }
        seconds = seconds * 1000;
        std::this_thread::sleep_for(std::chrono::milliseconds(seconds));
    }

    void Println(int t)
    {
        std::cout << t <<std::endl;
    }

    void Println(char t)
    {
        std::cout << t <<std::endl ;
    }

    void Println(std::string t)
    {
        std::cout << t <<std::endl ;
    }

    void Print(int t)
    {
        std::cout << t;
    }

    void Print(char t)
    {
        std::cout << t;
    }

    void Print(std::string t)
    {
        std::cout << t;
    }

    size_t Write(char* ptr, size_t size, size_t nmemb, void *f)
    {
        FILE *file = (FILE *)f;
        return fwrite(ptr, size, nmemb, file);
    };

    void Split(std::vector<std::string> &tokens, std::string str, std::string delimiter)
    {
        boost::split(tokens, str, boost::is_any_of(delimiter.c_str()));
    }

    void RemoveLastChar(std::string &str)
    {
        trim(str, "\t\n\v\f\r ");
        str = str.substr(0, str.size()-1);
    }

    void ToLower(std::string &str)
    {
        boost::algorithm::to_lower(str);
    }

    const char* GenerateUniqueKey()
    {
        size_t length = 4;
        auto randchar = []() -> char {
            const char charset[] =
                    "0123456789"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        uint64_t current_nanosecond = Nanoseconds();
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        return str.append(std::to_string(current_nanosecond)).c_str();
    }

    int RandomInteger(int min, int max)
    {
        return min + (rand() % static_cast<int>(max - min + 1));
    }

    int valid_digit(char *ip_str)
    {
        while (*ip_str) {
            if (*ip_str >= '0' && *ip_str <= '9')
                ++ip_str;
            else
                return 0;
        }
        return 1;
    }

    int IsV4(char *ip_str)
    {
        int num, dots = 0;
        char *ptr;

        if (ip_str == NULL)
            return 0;

        // See following link for strtok()
        // http://pubs.opengroup.org/onlinepubs/009695399/functions/strtok_r.html
        ptr = strtok(ip_str, DELIM);

        if (ptr == NULL)
            return 0;

        while (ptr) {

            /* after parsing string, it must contain only digits */
            if (!valid_digit(ptr))
                return 0;

            num = atoi(ptr);

            /* check for valid IP */
            if (num >= 0 && num <= 255) {
                /* parse remaining string */
                ptr = strtok(NULL, DELIM);
                if (ptr != NULL)
                    ++dots;
            } else
                return 0;
        }

        /* valid IP string must contain 3 dots */
        if (dots != 3)
            return 0;
        return 1;
    }

	const int *ConvertConstInt(int var)
	{
		return new int[var];
	}

    std::string GetIpAddressFromUrl(const std::string &host)
    {

        std::string ip;
    //    try {
    //        boost::asio::io_service io_service;
    //        ip::tcp::resolver resolver(io_service);
    //        ip::tcp::resolver::query query(host, "");
    //        ip::tcp::resolver::iterator iter = resolver.resolve(query);
    //        ip::tcp::resolver::iterator end; // End marker.
    //        while (iter != end) {
    //            ip::tcp::endpoint endpoint = *iter++;
    //            ip = std::string(endpoint.address().to_string());
    //            ReplaceWithString(ip, ":80","");
				//int _ip_size = ip.size() + 1;
				//const int* ip_size = ConvertConstInt(_ip_size);
    //            char cstr[ip_size];
    //            strcpy(cstr, ip.c_str());	// or pass &s[0]


    //            if(IsV4(cstr)){
    //                break;
    //            }
    //        }
    //    }
    //    catch (...) {
    //        return {};
    //    }

        return ip;
    }

    bool IsFileExists(const std::string& name)
    {
        struct stat buffer;
        return (stat (name.c_str(), &buffer) == 0);
    }

    bool IsValidDomain(std::string &domain)
    {
        //^([a-z0-9-]{1,63}|[a-z0-9][a-z0-9-]{0,61}[a-z0-9]-)$
        std::regex rx("^(([a-z0-9-]{1,63}|[a-z0-9-][a-z0-9-]{0,61}[a-z0-9-]))$");
        if (std::regex_match(domain, rx)) {
            return true;
        }
        return false;
    }

    std::string CmdOutput(std::string cmd)
    {
		char buffer[256];
		std::string result = "";
		std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), _pclose);
		if (!pipe) {
			result = "";
		}
		while (!feof(pipe.get())) {
			if (fgets(buffer, 128, pipe.get()) != NULL)
				result += buffer;
		}
		return result;
    }

    bool MoveFile(std::string old_path, std::string new_path)
    {
        if(std::rename(old_path.c_str(), new_path.c_str()) < 0) {
            return false;
        }
        return true;
    }

    long GetFileSize(std::string filename)
    {
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
    }

    bool CopyFile(std::string old_path, std::string new_path)
    {
        std::ifstream  src(old_path.c_str(), std::ios::binary);
        std::ofstream  dst(new_path.c_str(),   std::ios::binary);
        dst << src.rdbuf();
    }

    bool RemoveFile(std::string filename)
    {
        bool rstatus = false;
        int status = remove(filename.c_str());
        if(status == 0) {
            rstatus = true;
        }
        return rstatus;
    }

    bool CreateDirectory(std::string path)
    {
        return boost::filesystem::create_directories(path);
    }

    std::string GenerateUuid()
    {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        const std::string tmp = boost::lexical_cast<std::string>(uuid);
        return std::string(tmp);
    }

    bool isAlpha(std::string &str) {
        if(!isalpha(str[0]) || !isspace(str[0])) {
            return true;
        }
        return false;
    }

    bool is_number(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }

    std::string createLicenceKey(std::string default_ip, std::string key, std::string iv)
    {
        std::string licence_iv = ("");
        licence_iv += default_ip;
        licence_iv += key;
        licence_iv += iv;

        std::string licence_md5key = md5(licence_iv.c_str());
        std::string licence_key = "";

        for (int i = 5; (unsigned)i <= licence_md5key.size(); i+=5+1)
            licence_md5key.insert(i, "-");

        licence_md5key = licence_md5key.substr(0, licence_md5key.size()-3);

        return licence_md5key;
    }

    bool commandIsValid(std::string str)
    {
        if(strstr(str.c_str(),"wrapper")) {
            return true;
        }
        if(strstr(str.c_str(),"appinstaller")) {
            return true;
        }
        if(strstr(str.c_str(),"webstatistics")) {
            return true;
        }
        if(strstr(str.c_str(),"mng")) {
            return true;
        }
        else {
            return false;
        }
    }
};


#endif //LICENCE_HELPERS_H
