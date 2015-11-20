#ifndef __THPOOL_H_
#define __THPOOL_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <list>
#include <thread>

class thpool {
public:
	thpool(void (*f)(int));
	virtual ~thpool();
	
	void push_connection(int val);
	int pop_connection(void);
	int startThread(int n=1); // start (a) thread
	void main(void);
private:

	std::queue<int>		m_queue;
	std::mutex		m_syncQueue;
	std::condition_variable_any	m_syncEvents;
	std::list<std::thread*>	m_thList;
	void			(*m_func)(int);
};

#endif // __THPOOL_H_