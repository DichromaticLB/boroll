#include"include/XMLPostFetch.hpp"
#include"include/Bofiguration.hpp"
#include<fstream>
#include<algorithm>

XMLPostFetch::XMLPost::XMLPost(const pugi::xml_node& n):node(n),id(0){

	for(auto child:n)
		map::operator [](child.name())=child.child_value();

	id=std::stol(getOr(Bofiguration::fetch<std::string>("postIdAttr"),"0"));
}

std::string XMLPostFetch::XMLPost::getOr(const std::string& key,std::string defaultVal)
{
	if(find(key)==end())
		return defaultVal;
	else
		return at(key);
}

bool XMLPostFetch::XMLPost::existsNoEmpty(const std::string& key) const
{
	return find(key)!=end()&&!at(key).empty();
}

const std::string& XMLPostFetch::XMLPost::validUrl()
{
	for(auto& urltag:validURLS())
		if(existsNoEmpty(urltag))
			return at(urltag);

	return map::operator []("NULL");
}

bool XMLPostFetch::XMLPost::validFormat()
{
	if(existsNoEmpty(Bofiguration::fetch<std::string>("postFormatAttr")))
	{
		auto format=at(Bofiguration::fetch<std::string>("postFormatAttr"));
		auto acceptedFormats=Bofiguration::fetchVec("ValidFormatsVariable");
		if(std::find(acceptedFormats.begin(),acceptedFormats.end(),format)==acceptedFormats.end())
			return false;
	}

	return true;
}

std::vector<std::string>& XMLPostFetch::XMLPost::validURLS()
{
	static std::vector<std::string> v=Bofiguration::fetchVec("ValidURLParams");
	return v;
}

std::string& XMLPostFetch::XMLPost::tmpdirmater()
{
	static std::string v=Bofiguration::fetch<std::string>("tmpDir");
	return v;
}

bool XMLPostFetch::XMLPost::materialize()
{
	Bofiguration::makedir(tmpdirmater());
	file=tmpdirmater()+"/"+std::to_string(id);

	if(!isMaterialized()&&!validUrl().empty()&&validFormat())
	{
		bolog("fetch "+validUrl()+" into: "+file);
		long response;
		auto req=CurlWrap::doGET(validUrl());
		auto res=req.exec(&response);

		if(CurlWrap::GET::valid(response))
		{
			std::ofstream ofile(file,std::ios_base::binary|std::ios_base::out);
			ofile<<res.rdbuf();
			ofile.close();
		}
		else
		{
			bolog("Failed to fetch file "+validUrl()+" into: "+file);
		}

	}

	return isMaterialized();
}

void XMLPostFetch::XMLPost::dematerialize()
{
	if(isMaterialized())
	{
		Bofiguration::remove(file);
		file="";
	}
}

bool XMLPostFetch::XMLPost::isMaterialized()
{
	if(file.empty()||!Bofiguration::fileExists(file))
		return false;

	return true;
}

XMLPostFetch::XMLPost::operator std::string()
{
	return std::to_string(id)+" URL:"+validUrl()+ " file:"+file;
}

XMLPostFetch::XMLPostFetch():
request(CurlWrap::doGET(Bofiguration::fetch<std::string>("getURL"))),
page(1),
retries(0),
maxRetries(std::stol(Bofiguration::fetch<std::string>("emptyPagesUntilStop")))
{
	for(auto key:Bofiguration::fetchVec("getURLParams"))
	{
		auto& pair=Bofiguration::fetchVec(key);
		request.param(pair[0],pair[1]);
	}
}

void XMLPostFetch::setPage(long p){page=p;}

void XMLPostFetch::reset()
{
	page=1;
	retries=0;
	posts.clear();
}

bool XMLPostFetch::fetch(const map& params)
{
	if(retries>=maxRetries)
		return false;

	for(auto pair:params)
		request.param(pair.first,pair.second);

	request.param(Bofiguration::fetch<std::string>("getURLPageParam"),std::to_string(page));

	long response=0;
	auto res=request.exec(&response);
	if(CurlWrap::GET::valid(response))
	{
		auto currentSize=posts.size();

		using namespace pugi;

		xml_document doc;
		doc.load(res);

		auto postContainerId=Bofiguration::fetch<std::string>("postNameContainer");
		auto postTagId=Bofiguration::fetch<std::string>("postname");

		auto container=doc.child(postContainerId.c_str());
		for(xml_node node=container.child(postTagId.c_str()); node; node = node.next_sibling(postTagId.c_str()))
			posts.emplace_back(node);

		if(posts.size()>currentSize)
			retries=0;
		else
			retries++;

		page++;
	}
	else
	{
		bolog("Failed to retrieve page "+std::to_string(page)+" request:\n\t"+request.fullRequest());
	}

	return true;
}

std::deque<XMLPostFetch::XMLPost> XMLPostFetch::retrieve(unsigned max)
{
	if(posts.size()<max||max==0)
		max=posts.size();

	std::deque<XMLPost> res(posts.begin(),posts.begin()+max);
	posts.erase(posts.begin(),posts.begin()+max);

	return res;

}


static bool tryParse(const std::string& num,unsigned long& out)
{
	try{
		out=std::stoul(num);
		return true;

	}catch(const std::exception&e)
	{
		return false;
	}
}

static  std::map<int,std::string> makeMap()
{
	std::map<int,std::string> res;
	auto categs=Bofiguration::fetchVec("autocompleteCategoryTranslate");
	for(unsigned s=0;s<(categs.size()-1);s+=2)
	{
		unsigned long l;
		if(tryParse(categs[s],l))
			res[l]=categs[s+1];

	}
	return res;
}

static std::map<int,std::string>& categoriesMap()
{
	static std::map<int,std::string> mp=makeMap();
	return mp;
}

XMLAutocomplete::result::operator std::string()
{
	std::string rep = val;
	if (!alternative.empty())
		rep += " ->" + alternative;

	if (count)
		rep += " (" + std::to_string(count) + ")";

	if (!category.empty())
		rep += " [" + category + "]";
	return rep;

}

std::vector<XMLAutocomplete::result> XMLAutocomplete::fetch(std::string pattern)
{
	std::vector<XMLAutocomplete::result> res;
	CurlWrap::GET req=CurlWrap::doGET(Bofiguration::fetch<std::string>("getAutocompleteURL"));
	req.param(Bofiguration::fetch<std::string>("getAutocompleteParam"),pattern);

	for(auto key:Bofiguration::fetchVec("getAutocompleteParams"))
	{
		auto& pair=Bofiguration::fetchVec(key);
		req.param(pair[0],pair[1]);
	}
	long code;
	auto ss=req.exec(&code);
	if(CurlWrap::GET::valid(code))
	{
		pugi::xml_document d;
		d.load(ss);
		auto container=Bofiguration::fetch<std::string>("autocompleteContainer");
		auto unit=Bofiguration::fetch<std::string>("autocompleteName");

		auto value   =Bofiguration::fetch<std::string>("autocompleteValue");
		auto alter   =Bofiguration::fetch<std::string>("autocompleteAlter");
		auto count   =Bofiguration::fetch<std::string>("autocompleteCount");
		auto category=Bofiguration::fetch<std::string>("autocompleteCat");


		for(pugi::xml_node n=d.child(container.c_str()).child(unit.c_str());n;n=n.next_sibling(unit.c_str()))
		{
			result r;
			if(n.child(value.c_str()))
				r.val=n.child_value(value.c_str());

			if(n.child(alter.c_str()))
				r.alternative=n.child_value(alter.c_str());

			unsigned long l;

			if(n.child(count.c_str())&&tryParse(n.child_value(count.c_str()),l))
			{
				r.count=l;
			}

			if(n.child(category.c_str())&&tryParse(n.child_value(category.c_str()),l))
			{
				r.category=categoriesMap()[l];
			}

			if(!r.val.empty()||!r.alternative.empty())
				res.push_back(r);

		}
	}

	return res;
}


static std::map<std::string,XMLPostFilter>& findex(){
	static std::map<std::string,XMLPostFilter> index;
	return index;
}

XMLPostFilter& XMLPostFilter::create(const std::string& name)
{
	findex()[name]=XMLPostFilter(name);
	return findex()[name];
}

XMLPostFilter& XMLPostFilter::getByName(const std::string& name)
{
	if(findex().find(name)==findex().end())
		bolog("Trying to find unexistant filter");
	return findex()[name];
}

std::vector<std::string> XMLPostFilter::getAllNames()
{
	std::vector<std::string> res;
	for(auto&s:findex())
		res.push_back(s.first);
	return res;
}


XMLPostFilter::XMLPostFilter(const std::string& n):name(n){}

XMLPostFilter&  XMLPostFilter::chain(const std::string& name,const std::string& val,rule::SYM test,bool important)
{
	rule r;
	r.isNum=false;
	r.required=important;
	r.key=name;
	r.val=val;
	r.test=test;
	rules.push_back(r);
	return *this;
}

XMLPostFilter&  XMLPostFilter::chain(const std::string& name,unsigned long  val,rule::SYM test,bool important)
{
	rule r;
	r.isNum=true;
	r.required=important;
	r.key=name;
	r.num=val;
	r.test=test;
	rules.push_back(r);
	return *this;
}

XMLPostFilter&  XMLPostFilter::chain(const rule& r)
{
	if(r.isNum)
		return chain(r.key,r.num,r.test,r.required);
	else
		return chain(r.key,r.val,r.test,r.required);
}


bool XMLPostFilter::satisfiesPredicate(const XMLPostFetch::XMLPost& post) const
{
	for(auto r:rules)
	{
		if(!post.existsNoEmpty(r.key))
		{
			if(r.required)
				return false;
			else
				continue;
		}
		else
		{
			auto val=post.at(r.key);
			if(r.isNum)
			{
				unsigned long l=0;
				if(!tryParse(val,l))
				{
					if(!r.required)
						continue;
					else
						return false;
				}
				else
					switch(r.test)
					{
					case rule::EQ:
						if(l!=r.num)
							return false;
						break;
					case rule::NOT_EQ:
						if(l==r.num)
							return false;
						break;
					case rule::GT:
						if(l<r.num)
							return false;
						break;
					case rule::LT:
						if(l>r.num)
							return false;
						break;
					}
			}
			else
			{
				switch(r.test)
				{
				case rule::EQ:
					if(val!=r.val)
						return false;
					break;
				case rule::NOT_EQ:
					if(val==r.val)
						return false;
					break;
				default:
					bolog("Trying to do GT/LT comparisons with strings, not supported");
					break;
				}
			}
		}
	}


	return true;
}
