#include "thpool.h"

thpool::thpool()
{
}

thpool::~thpool()
{
}

void thpool::push(int v) {
	lock lk(m_syncQueue);
	m_queue.push(v);
}

int thpool::pop(void) {
	lock lk(m_syncQueue);
	if (m_queue.empty())
		return -1;
	int ret=m_queue.front();
	m_queue.pop();
	return ret;
}