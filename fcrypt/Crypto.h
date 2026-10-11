#pragma once
#ifndef FCRPT_CRYPTO_H
#define FCRPT_CRYPTO_H

#include <stdio.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/base64.h>
#include <Helpers.h>

class Crypto {
private:
	Helpers _helpers;
public:
	std::string Encrypt(const std::string& str_in, const std::string& key, const std::string& iv)
	{
		try {
			std::string str_out;
			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryption((byte*)key.c_str(), key.length(), (byte*)iv.c_str());
			CryptoPP::StringSource encryptor(str_in, true,
				new CryptoPP::StreamTransformationFilter(encryption,
					new CryptoPP::Base64Encoder(
						new CryptoPP::StringSink(str_out), false
					)
				)
			);
			return safeBase64Encode(str_out);
			//            return str_out;
		}
		catch (...) {
			return {};
		}
	}

	std::string Decrypt(const std::string& str_in, const std::string& key, const std::string& iv)
	{
		std::string str = safeBase64Decode(str_in);
		try {
			std::string str_out;
			CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryption((byte*)key.c_str(), key.length(), (byte*)iv.c_str());
			CryptoPP::StringSource decryptor(str, true,
				new CryptoPP::Base64Decoder(
					new CryptoPP::StreamTransformationFilter(decryption,
						new CryptoPP::StringSink(str_out)
					)
				)
			);
			return str_out;
		}
		catch (...) {
			return {};
		}
	}

	std::string safeBase64Encode(std::string in)
	{
		_helpers.ReplaceWithString(in, std::string{ "+" }, std::string{ "-" });
		_helpers.ReplaceWithString(in, std::string("/"), std::string("_"));
		in = _helpers.RemoveAllChar(in, std::string("="));
		return in;
	}

	std::string safeBase64Decode(std::string in)
	{
		std::string result;
		_helpers.ReplaceWithString(in, "-", "+");
		_helpers.ReplaceWithString(in, "_", "/");
		int len = in.length();

		std::string comma = "====";

		int mod4 = len % 4;

		if (mod4 > 0) {
			std::string first_two = comma.substr(0, (4 - mod4));
			in += first_two;
		}
		return in;
	}
};

#endif //FCRPT_CRYPTO_H
