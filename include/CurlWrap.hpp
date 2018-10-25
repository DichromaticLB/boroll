#ifndef INCLUDE_CURLWRAP_HPP_
#define INCLUDE_CURLWRAP_HPP_

#include<string>
#include<vector>
#include<sstream>
#include<map>
class CurlWrap
{
public:
	class GET {


	public:
		GET&  param(const std::string& name, const std::string& val,bool escape1=true,bool escape2=true);
		std::stringstream exec(long* response=NULL,const std::string& enc="");
		std::string fullRequest();
		static bool valid(long l);

	private:

		GET(const std::string& url);
		std::string url;
		std::map<std::string,std::string> _params;
		friend class CurlWrap;
	};

	static GET doGET(const std::string& url);


private:

	static const CurlWrap&  instance();
	CurlWrap();
	~CurlWrap();
	void init() const;
	void stop() const;
	operator bool() const;

};


#endif /* INCLUDE_CURLWRAP_HPP_ */
