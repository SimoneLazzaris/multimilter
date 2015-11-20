#ifndef __THPOOL_H_
#define __THPOOL_H_

#include <queue>
#include "mlock.h"

class thpool {
public:
	thpool();
	virtual ~thpool();
	
	void push(int val);
	int pop(void);
private:
	std::queue<int>	m_queue;
	mutex		m_syncQueue;
	pthread_cond_t 	m_syncEvents;
};

#endif // __THPOOL_H_