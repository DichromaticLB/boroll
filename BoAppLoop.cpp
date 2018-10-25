#include"include/BoAppLoop.hpp"

	BoAppLoop::BoAppLoop(const TagFetch& t,const PicForward& pf,const isPaused& p):tg(t),setImg(pf),_wait(p){
		stopGeneration=true;
		stopConsumer=true;
		filtersChanged=false;
		_forward=false;
		maxPostsNew=Bofiguration::fetch<unsigned>("LimitPostsBufferedEmtpy");
		maxPostsMat=Bofiguration::fetch<unsigned>("LimitPostsBuffered");
		maxPostsSeen=Bofiguration::fetch<unsigned>("LimitPostsBufferedSeen");
		_backtrack=0;
		for(auto&rname:Bofiguration::fetchVec("defaultRules"))
			filters.push_back(rname);
	}

	unsigned BoAppLoop::materialCount()
	{
		materializedAccess.lock();
		unsigned res=materialized.size();
		materializedAccess.unlock();
		return res;
	}

	void BoAppLoop::cleanup()
	{
		std::lock_guard<std::mutex> g(materializedAccess);
		while (!materialized.empty())
		{
			materialized.front().dematerialize();
			materialized.pop();
		}
		Queue n1, n2;
		std::swap(unmaterialized, n1);
		websrc.reset();
		for (auto p : alreadySeen)
			p.dematerialize();
		alreadySeen.clear();
	}

	 void BoAppLoop::generationLoop(BoAppLoop* _this)
	{
		_this->cleanup();

		bool morePages=true;

		XMLPostFetch::map tags;
		tags[Bofiguration::fetch<std::string>("GETTags")]=_this->tg();

		unsigned sleep=Bofiguration::fetch<unsigned>("RefreshDL");

		int u_sleep=0;
		const int u_sleepg=Bofiguration::fetch<int>("sleepGranularity");

		while(!_this->stopGeneration)
		{
			std::this_thread::sleep_for (std::chrono::milliseconds(sleep/u_sleepg));
			u_sleep++;

			if(u_sleep>=u_sleepg)
				u_sleep=0;
			else
				continue;

			if(morePages&&_this->unmaterialized.size()<_this->maxPostsNew)
			{
				morePages=_this->websrc.fetch(tags);
			}
			else if(!morePages&&Bofiguration::fetch<bool>("ResetOnLastPage"))
			{
				_this->websrc.reset();
				morePages=true;
			}

			auto posts=_this->websrc.retrieve();

			for(auto p:posts)
			{
				bool passes=true;

				for(auto fname:_this->getActiveFilters())
				{
					auto& f=XMLPostFilter::getByName(fname);

					if(!(passes=f.satisfiesPredicate(p)))
						break;
				}

				if(passes)
					_this->unmaterialized.push(p);
			}

			if(_this->materialCount()<_this->maxPostsMat&&!_this->unmaterialized.empty())
			{
				auto p=_this->unmaterialized.front();
				if(p.materialize())
				{
					_this->materializedAccess.lock();
					_this->materialized.push(p);
					_this->materializedAccess.unlock();
				}
				_this->unmaterialized.pop();
			}
		}
	}

	void BoAppLoop::consLoop(BoAppLoop* _this)
	{

		_this->materializedAccess.lock();//Wait cleanup
		_this->materializedAccess.unlock();

		unsigned sleep=Bofiguration::fetch<unsigned>("Refresh");
		const int u_sleepg=Bofiguration::fetch<int>("sleepGranularity");
		int u_sleep=u_sleepg;

		while(!_this->stopConsumer)
		{

			std::this_thread::sleep_for (std::chrono::milliseconds(sleep/u_sleepg));
			u_sleep++;

			if(_this->_forward||u_sleep>=u_sleepg)
				u_sleep=0;
			else
				continue;

			if(_this->_wait())
				continue;

			_this->performBacktrack();
			bool success=false;

			if(_this->materialCount()!=0)
			{
				_this->materializedAccess.lock();
				auto post=_this->materialized.front();
				_this->materialized.pop();
				_this->materializedAccess.unlock();

				if(_this->alreadySeen.size()>_this->maxPostsSeen)
				{
					_this->alreadySeen.front().dematerialize();
					_this->alreadySeen.pop_front();
				}
				_this->alreadySeen.push_back(post);
				success=_this->setImg(post.file);
				_this->currentPostMutex.lock();
				_this->inDisplay=post;
				_this->currentPostMutex.unlock();
			}
			else if(!_this->alreadySeen.empty())
			{
				auto post=_this->alreadySeen.back();
				_this->alreadySeen.pop_front();
				_this->alreadySeen.push_back(post);
				success=_this->setImg(post.file);
				_this->currentPostMutex.lock();
				_this->inDisplay=post;
				_this->currentPostMutex.unlock();
			}

			if(!success)
				u_sleep=u_sleepg;

			_this->_forward=false;
		}
	}

	void BoAppLoop::start()
	{
		stopGeneration=false;
		stopConsumer=false;
		generatorLoop=std::thread(generationLoop,this);
		consumerLoop=std::thread(consLoop,this);
	}

	void BoAppLoop::stop()
	{
		stopGeneration=true;
		stopConsumer=true;
		generatorLoop.join();
		consumerLoop.join();
	}

	void BoAppLoop::filterUpdate(){filtersChanged=true;};
	void BoAppLoop::filterUpdateFinish(){filtersChanged=false;};
	bool BoAppLoop::needUpdate(){return filtersChanged;}

	std::list<std::string> BoAppLoop::getActiveFilters()
	{
		filterAccess.lock();
		std::list<std::string> res(filters);
		filterAccess.unlock();
		return res;
	}

	void BoAppLoop::setFilter(const std::string &name)
	{
		filterAccess.lock();
		filters.emplace_back(name);
		filterAccess.unlock();
	}

	void BoAppLoop::disableFilter(const std::string &name)
	{
		filterAccess.lock();
		filters.remove(name);
		filterAccess.unlock();
	}


	BoAppLoop::Post BoAppLoop::currentPost()
	{
		currentPostMutex.lock();
		BoAppLoop::Post res=inDisplay;
		currentPostMutex.unlock();
		return res;
	}

	void BoAppLoop::forward()
	{
		_forward=true;
	}

	void BoAppLoop::requestBacktrack()
	{
		backtrackMutex.lock();
		if(!_backtrack)
			_backtrack++;
		_backtrack++;
		backtrackMutex.unlock();
	}

	void BoAppLoop::performBacktrack()
	{
		backtrackMutex.lock();
		if(_backtrack)
		{
			materializedAccess.lock();
			Queue swap;
			std::swap(materialized,swap);

			while(_backtrack&&alreadySeen.size())
			{
				materialized.push(alreadySeen.back());
				alreadySeen.pop_back();
				_backtrack--;
			}

			while(!swap.empty())
			{
				materialized.push(swap.front());
				swap.pop();
			}

			_backtrack=0;
			materializedAccess.unlock();
		}
		backtrackMutex.unlock();
	}
