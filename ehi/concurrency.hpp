#ifndef _CONCURRENCY_H
#define _CONCURRENCY_H

#include <pthread.h>

template<class T>
class concurrent_object {
private:
	T content;
	pthread_mutex_t mutex;

public:
	concurrent_object(T in) : content(in), mutex() {
		pthread_mutex_init(&mutex, nullptr);
	}
	~concurrent_object() {
		pthread_mutex_destroy(&mutex);
	}

	T get() {
		pthread_mutex_lock(&mutex);
		T out = content;
		pthread_mutex_unlock(&mutex);
		return out;
	}
	void set(T new_value) {
		pthread_mutex_lock(&mutex);
		content = new_value;
		pthread_mutex_unlock(&mutex);
	}
};

#endif /* _CONCURRENCY_H */
