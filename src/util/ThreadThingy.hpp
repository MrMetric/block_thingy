#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include <concurrentqueue/concurrentqueue.hpp>

namespace block_thingy::util {

template
<
	typename T,
	typename Hash = std::hash<T>
>
class ThreadThingy
{
public:
	ThreadThingy
	(
		std::function<void(T&)> f,
		std::size_t thread_count,
		const Hash& hash = Hash()
	)
	:
		queued(0, hash),
		running(true)
	{
		auto g = [this, f]()
		{
			while(running)
			{
				T thing;
				if(things.try_dequeue(thing))
				{
					f(thing);
				}
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(10ms);
			}
		};
		for(std::size_t i = 0; i < thread_count; ++i)
		{
			threads.emplace_back(g);
		}
	}

	~ThreadThingy()
	{
		stop();
	}

	void enqueue(const T& thing)
	{
		bool emplaced;
		{
			std::lock_guard<std::mutex> g(queued_mutex);
			emplaced = queued.emplace(thing).second;
		}
		if(emplaced)
		{
			things.enqueue(thing);
		}
	}

	void dequeue(const T& thing)
	{
		std::lock_guard<std::mutex> g(queued_mutex);
		const auto i = queued.find(thing);
		assert(i != queued.cend());
		queued.erase(i);
	}

	bool has(const T& thing) const
	{
		std::lock_guard<std::mutex> g(queued_mutex);
		return queued.find(thing) != queued.cend();
	}

	template<typename T2>
	bool has(const std::shared_ptr<const T2>& thing) const
	{
		std::lock_guard<std::mutex> g(queued_mutex);
		return queued.find(std::const_pointer_cast<T2>(thing)) != queued.cend();
	}

	void stop()
	{
		if(running)
		{
			running = false;
			for(auto& thread : threads)
			{
				thread.join();
			}
		}
	}

private:
	moodycamel::ConcurrentQueue<T> things;
	std::unordered_set<T, Hash> queued;
	mutable std::mutex queued_mutex;
	std::atomic<bool> running;
	std::vector<std::thread> threads;
};

}
