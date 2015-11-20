#ifndef __MLOCK_H_
#define __MLOCK_H_

#include <pthread.h>

class mutex { 
private:
    pthread_mutex_t m_mutex;
public:
    mutex()       { pthread_mutex_init(&m_mutex,NULL);    }
    ~mutex()      { pthread_mutex_destroy(&m_mutex); }
    void lock()   { pthread_mutex_lock(&m_mutex);    }
    void unlock() { pthread_mutex_unlock(&m_mutex);  }
};

class lock {
private:
    mutex &m_mutex;
public:
    lock(mutex &m) : m_mutex(m) { m_mutex.lock(); }
    ~lock() { m_mutex.unlock(); }
};

#endif