#ifndef INCLUDE_BOAPPLOOP_HPP_
#define INCLUDE_BOAPPLOOP_HPP_
#include"Bofiguration.hpp"
#include"XMLPostFetch.hpp"
#include<thread>
#include<queue>
#include<deque>
#include<functional>
#include<mutex>
#include<vector>
#include<list>

class BoAppLoop
{

public:
	using Post=XMLPostFetch::XMLPost;
	using Queue=std::queue<Post>;
	using Deque=std::deque<Post>;
	using TagFetch=std::function<std::string()>;
	using PicForward=std::function<bool(const std::string& src)>;
	using isPaused=std::function<bool()>;

	BoAppLoop(const TagFetch& t,const PicForward& p,const isPaused& w);
	unsigned materialCount();
	void cleanup();
	static void generationLoop(BoAppLoop* _this);
	static void consLoop(BoAppLoop* _this);
	void start();
	void stop();
	void filterUpdate();
	void filterUpdateFinish();
	bool needUpdate();
	std::list<std::string> getActiveFilters();
	void setFilter(const std::string &name);
	void disableFilter(const std::string &name);

	Post currentPost();
	void forward();
	void requestBacktrack();
	void performBacktrack();

private:
	TagFetch tg;
	PicForward setImg;
	isPaused _wait;

	Queue unmaterialized;
	Queue materialized;
	Deque alreadySeen;
	XMLPostFetch websrc;
	Post inDisplay;
	std::mutex materializedAccess,filterAccess,currentPostMutex,backtrackMutex;
	std::list<std::string> filters;
	unsigned maxPostsNew,maxPostsMat,maxPostsSeen,_backtrack;
	bool stopGeneration,stopConsumer,filtersChanged,_forward;
	std::thread generatorLoop;
	std::thread consumerLoop;
};



#endif /* INCLUDE_BOAPPLOOP_HPP_ */
