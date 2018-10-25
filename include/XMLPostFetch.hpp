/*
 * XMLPostFetch.hpp
 *
 *  Created on: Oct 20, 2018
 *      Author: manuel
 */

#ifndef INCLUDE_XMLPOSTFETCH_HPP_
#define INCLUDE_XMLPOSTFETCH_HPP_

#include<string>
#include<map>
#include<vector>
#include<queue>
#include"pugixml.hpp"
#include"CurlWrap.hpp"

class XMLPostFetch
{
public:
	using map=std::map<std::string,std::string>;

	class XMLPost:public map
	{
	public:
		XMLPost(const pugi::xml_node& n);
		XMLPost()=default;
		std::string getOr(const std::string& key,std::string defaultVal);
		bool existsNoEmpty(const std::string& key) const;
		const std::string& validUrl();
		bool validFormat();
		bool materialize();
		void dematerialize();
		bool isMaterialized();
		operator std::string();
		std::string file;
	private:
		static  std::string& tmpdirmater();
		static  std::vector<std::string>& validURLS();
		pugi::xml_node node;

		long id;
	};

	XMLPostFetch();
	bool fetch(const map& params=map());
	void reset();
	void setPage(long p);
	std::deque<XMLPost> retrieve(unsigned max=0);

private:
	CurlWrap::GET request;
	std::deque<XMLPost> posts;
	long page,retries,maxRetries;
};

class XMLAutocomplete
{
public:
	struct result
	{
		std::string val;
		std::string alternative;
		unsigned int count;
		std::string category;
		operator std::string();
	};
	static std::vector<XMLAutocomplete::result> fetch(std::string pattern);
};

class XMLPostFilter
{
public:

	struct rule
	{
		enum SYM
		{
			EQ,
			NOT_EQ,
			GT,
			LT
		};

		std::string key;
		std::string val;
		unsigned long num;
		bool isNum,required;
		SYM test;
	};
	static XMLPostFilter& create(const std::string& name);
	XMLPostFilter&  chain(const std::string& name,const std::string& val,rule::SYM test,bool important);
	XMLPostFilter&  chain(const std::string& name,unsigned long  val,rule::SYM test,bool important);
	XMLPostFilter&  chain(const rule& r);
	bool satisfiesPredicate(const XMLPostFetch::XMLPost&) const;
	static std::vector<std::string> getAllNames();
	static XMLPostFilter& getByName(const std::string& name);
	XMLPostFilter()=default;
private:

	XMLPostFilter(const std::string& name);
	std::string name;
	std::vector<rule> rules;
};

#endif /* INCLUDE_XMLPOSTFETCH_HPP_ */
