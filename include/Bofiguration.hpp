#ifndef INCLUDE_BOFIGURATION_HPP_
#define INCLUDE_BOFIGURATION_HPP_

#include<string>
#include<sstream>
#include<exception>
#include<iostream>
#include<vector>
#include<ostream>
#define DEFILE ".bconf"

#ifdef _DEBUG_

#define bolog(x) Bofiguration::_log(x)

#else

#define bolog(x)
#endif

namespace Bofiguration
{
	using string=std::string;
	std::string& deff(std::string update="");

	string _fetch(const string& key);
	string put(const string& key,const string& val);


	template<typename T=std::string>
	T fetch(const string& key)
	{
		T res;
		std::stringstream s(_fetch(key));
		s>>res;
		return res;
	}

	std::vector<string>& putvec(const string& key,const string& val);
	std::vector<string>& putvec(const string& key);
	std::vector<string>& fetchVec(const string& key);

	struct TokenAutocomplete
	{
		char autocIndex;
		std::vector<std::string> tokens;
	};

	TokenAutocomplete tokenize(const std::string &st, unsigned autoindex);


	void setup();
	bool fileExists(const std::string& path);
	void makedir(const std::string& path);
	void remove(const std::string& path);

	void _log(const std::string& msg);
};

#endif /* INCLUDE_BOFIGURATION_HPP_ */
