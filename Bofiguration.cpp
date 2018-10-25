#include"include/Bofiguration.hpp"
#include<map>


using string=Bofiguration::string;

static std::map<Bofiguration::string,Bofiguration::string>& _dic()
{
	static std::map<Bofiguration::string,Bofiguration::string> res;
	return res;
}

std::string& Bofiguration::deff(std::string update)
{
	static std::string defaultFile=DEFILE;
	if(!update.empty())
		defaultFile=update;

	return defaultFile;
}

static std::map<Bofiguration::string,std::vector<Bofiguration::string>>& _vecdic()
{
	static std::map<Bofiguration::string,std::vector<Bofiguration::string>> res;
	return res;
}

Bofiguration::string Bofiguration::_fetch(const string& key)
{
	try {
		auto res=_dic().at(key);
		return res;
	} catch (std::out_of_range& e) {
		std::cerr<<"Failed to retrieve config param: "<<key<<std::endl;
		throw e;
	}
}


Bofiguration::string Bofiguration::put(const string& k,const string& v)
{
	string res=_dic()[k];
	_dic()[k]=v;
	return res;
}

std::vector<Bofiguration::string>&  Bofiguration::putvec(const string& key,const string& val)
{
	_vecdic()[key].push_back(val);
	return _vecdic()[key];
}

std::vector<Bofiguration::string>&  Bofiguration::putvec(const string& key)
{
	return _vecdic()[key];
}

std::vector<Bofiguration::string>& Bofiguration::fetchVec(const string& key)
{
	try {
		return _vecdic().at(key);
	} catch (std::out_of_range& e) {
		std::cerr << "Failed to retrieve vector " << key << std::endl;
		throw e;
	}
}
Bofiguration::TokenAutocomplete Bofiguration::tokenize(const std::string &st, unsigned autoindex)
{
	Bofiguration::TokenAutocomplete res;
	res.autocIndex=-1;
	bool setPos=false;
	int init=-1;

	for(unsigned i=0;i<st.length()+1;i++)
	{
		if(i==autoindex)
			setPos=true;

		if(i==st.length()||st[i]==' ')
		{
			if(init!=-1)
			{
				if(setPos)
				{
					res.autocIndex=res.tokens.size();
					setPos=false;
				}
				res.tokens.emplace_back(st.substr(init,i-init));
				init=-1;
			}
		}
		else if(init==-1)
		{
			init=i;
		}
	}

	return res;
}


#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<string.h>
#include<iomanip>
#include<ctime>

bool Bofiguration::fileExists(const std::string& path)
{
	 struct stat buffer;
	 return (stat (path.c_str(), &buffer) == 0)&& S_ISREG(buffer.st_mode) ;

}

void Bofiguration::makedir(const std::string& path)
{
	 struct stat buffer;
	 if (stat(path.c_str(), &buffer) == -1)
		if( mkdir(path.c_str(),DEFFILEMODE|S_IXUSR)==-1)
		{
			bolog("Failed to create "+path+" errno:"+strerror(errno));
		}
}

void Bofiguration::remove(const std::string& path)
{
	struct stat buffer;
	if((stat (path.c_str(), &buffer) == 0)&& S_ISDIR(buffer.st_mode))
	{
		if(rmdir(path.c_str())==-1)
			bolog("Failed to delete dir "+path+" errno:"+strerror(errno));
	}
	else if (S_ISREG(buffer.st_mode))
	{
		if(unlink(path.c_str())==-1)
			bolog("Failed to delete file "+path+" errno:"+strerror(errno));
	}
	else
	{
		bolog("failed to remove: "+path+ "Not dir or file");
	}
}

void Bofiguration::_log(const std::string& msg)
{
	 auto t = std::time(nullptr);
	 auto tm = *std::localtime(&t);
	std::cerr<<std::put_time(&tm, "%d-%m-%Y %H:%M:%S ")<<msg<< std::endl;
}
#endif
