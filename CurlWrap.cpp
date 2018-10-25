#include"include/CurlWrap.hpp"
#include"include/Bofiguration.hpp"
#include<curl/curl.h>
#include<cstdio>
#include<iostream>
#include<algorithm>
#include<numeric>

static CURL* _curlHandle=NULL;

const CurlWrap& CurlWrap::instance()
{
	static const CurlWrap instance;
	instance.init();

	if(!instance)
	{
		std::cerr<<"CURL failed to setup";
		exit(1);
	}

	return instance;
}

void CurlWrap::init() const
{
	if(_curlHandle==0)
		_curlHandle=curl_easy_init();
}

void CurlWrap::stop() const
{
	if(_curlHandle!=NULL)
		curl_easy_cleanup(_curlHandle);
	_curlHandle=NULL;

}

CurlWrap::CurlWrap()
{
	init();
}

CurlWrap::~CurlWrap()
{
	stop();
}

CurlWrap::operator bool() const
{
	return _curlHandle!=NULL;
}

using GET=CurlWrap::GET;

GET& CurlWrap::GET::param(const std::string& name,const std::string& val,bool _escape1,bool _escape2)
{
	auto instance =CurlWrap::instance();

	const char* nameEscaped=name.c_str();
	const char* valueEscaped=val.c_str();

	if(_escape1)
		nameEscaped=curl_easy_escape(_curlHandle,name.c_str(),name.size());

	if(_escape2)
		valueEscaped=curl_easy_escape(_curlHandle,val.c_str(),val.size());

	if(_escape1&&!nameEscaped)
	{
		std::cerr<<"Failed to escape name "<<name<<std::endl;
	}

	if(_escape2&&!valueEscaped)
	{
		std::cerr<<"Failed to escape val "<<val<<std::endl;
	}

	if(!nameEscaped||!valueEscaped)
		exit(1);

	_params[nameEscaped]=(std::string(nameEscaped)+"="+std::string(valueEscaped));

	if(_escape1)
		curl_free((void*)nameEscaped);

	if(_escape2)
		curl_free((void*)valueEscaped);

	return *this;
}

GET::GET(const std::string &_url):url(_url){}


 bool GET::valid(long response)
{
	return (response>=200&&response<300);
}

GET CurlWrap::doGET(const std::string& url)
{
	return GET(url);
}

std::stringstream GET::exec(long* response,const std::string& enc)
{

	auto instance =CurlWrap::instance();
	std::stringstream res;
	auto copyHandler=curl_easy_duphandle(_curlHandle);
	curl_easy_setopt(copyHandler,CURLOPT_HTTPGET,1);



	curl_easy_setopt(copyHandler,CURLOPT_URL,(fullRequest()).c_str());
	if(!enc.empty())
		curl_easy_setopt(copyHandler,CURLOPT_ACCEPT_ENCODING,enc.c_str());

	FILE* tmp=tmpfile();
	curl_easy_setopt(copyHandler,CURLOPT_WRITEDATA,tmp);
	CURLcode result=curl_easy_perform(copyHandler);

	long code;
	curl_easy_getinfo(copyHandler, CURLINFO_RESPONSE_CODE, &code);

	if(response!=NULL)
		*response=code;

	if(result!=CURLE_OK)
	{
		bolog("Curl GET "+fullRequest() + "Error: "+curl_easy_strerror(result)+" HTTP Code: "+std::to_string(code));
	}
	else
	{
		rewind(tmp);
		char data[256];
		int bytesRead=0;
		while((bytesRead=fread(data,sizeof(char),256,tmp)))
			res.write(data,bytesRead);
	}

	fclose(tmp);

	if(copyHandler!=NULL)
	{
		curl_easy_cleanup(copyHandler);
		copyHandler=NULL;
	}


	return res;
}

std::string GET::fullRequest() //It's just like my Japascript cartoons
{
	std::string paramString;

	if(_params.size())
		{
			bool first=true;
			for(auto param:_params)
			{
				if(first)
				{
					first=false;
					paramString="?"+param.second;
				}
				else
				{
					paramString+="&"+param.second;
				}
			}
		}

	return url+paramString;
}
