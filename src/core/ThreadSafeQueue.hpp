#pragma once

#include <queue>
#include <mutex>

class DebugMessage
{
public:
	virtual ~DebugMessage() {}
};

template <typename T>
class ThreadSafeQueue
{
public:
	void push(std::unique_ptr<T> item)
	{
		mtx.lock();
		item_queue.push(std::move(item));
		mtx.unlock();
	}

	bool empty()
	{
		std::lock_guard<std::mutex> lock(mtx);
		return item_queue.empty();
	}

	std::unique_ptr<DebugMessage> tryPop()
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (item_queue.empty())
		{
			return nullptr;
		}

		std::unique_ptr<T> popped_value = std::move(item_queue.front());
		item_queue.pop();
		return std::move(popped_value);
	}

private:
	std::queue<std::unique_ptr<T>> item_queue;
	std::mutex mtx;
};