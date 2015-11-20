#include "thpool.h"
extern void dbg( const char * str);
void dbg( const char * str, int n);

thpool::thpool(void (*f)(int))
{
	m_func=f;
}

thpool::~thpool()
{
}

void thpool::push_connection(int v) {
	std::lock_guard<std::mutex> lk(m_syncQueue);
	m_queue.push(v);
	m_syncEvents.notify_one();
}

int thpool::pop_connection(void) {
	std::unique_lock<std::mutex> lk(m_syncQueue);
	while (m_queue.empty())
		m_syncEvents.wait(lk);  
	int ret=m_queue.front();
	m_queue.pop();
	return ret;
}

void bootstrap(thpool *p){
	p->main();
}

int thpool::startThread(int n) {
	for (int i=0; i<n; i++) {
		std::thread *t=new std::thread(bootstrap,this);
		m_thList.push_back(t);
		}
}

void thpool::main(void){
	while (1) {
		int fd=pop_connection();
		(*m_func)(fd);
	}
}