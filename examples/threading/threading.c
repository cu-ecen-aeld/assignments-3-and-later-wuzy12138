#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

static inline pthread_mutex_t *thread_mutex_init(pthread_mutex_t *mutex) {
    return mutex;
}

static inline pthread_t *thread_init(pthread_t *thread) {
    if (thread == NULL) {
	    thread = (pthread_t *) malloc(sizeof(pthread_t));
    }
	return thread;
}

struct thread_data *thread_data_init(struct thread_data *param, pthread_mutex_t *mutex, int obtain_wait_ms, int release_wait_ms) {

    param = !param ? (struct thread_data *) malloc(sizeof(struct thread_data)) : param;
    if (param == NULL) {
		return param;
	}

    param->mutex = mutex;
    param->obtain_wait_ms = obtain_wait_ms;
    param->release_wait_ms = release_wait_ms;
	param->thread_complete_success = false;

	return param;
}

unsigned int gettime_ms(struct timeval *timer) {
    if (gettimeofday(timer, NULL) != 0) {
        ERROR_LOG("[%s] get time fail, @%d", __func__, __LINE__);
		return -1;
	}
	return (unsigned int ) ((timer->tv_sec*1000) + (timer->tv_usec/1000));
}



void* threadfunc(void* thread_param)
{

    struct thread_data *param = (struct thread_data *) thread_param;

    if (usleep(param->obtain_wait_ms * 1000) == -1) {
        ERROR_LOG("[%s] usleep fail, @%d", __func__, __LINE__);
		param->thread_complete_success = false;
        return thread_param;;
	}

	if (pthread_mutex_lock(param->mutex) != 0) {
        ERROR_LOG("[%s] mutex lock fail, @%d", __func__, __LINE__);
		param->thread_complete_success = false;
        return thread_param;;
	}

    if (usleep(param->release_wait_ms * 1000) == -1) {
        ERROR_LOG("[%s] usleep fail, @%d", __func__, __LINE__);
		param->thread_complete_success = false;
        return thread_param;;
	}

	if (pthread_mutex_unlock(param->mutex) != 0) {
        ERROR_LOG("[%s] mutex unlock fail, @%d", __func__, __LINE__);
		param->thread_complete_success = false;
        return thread_param;;
	}
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    param->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *param = NULL;

    if ((thread = thread_init(thread)) == NULL) {
        ERROR_LOG("[%s] thread initialize fail, @%d", __func__, __LINE__);
        return false;
	}

    if ((mutex = thread_mutex_init(mutex)) == NULL) {
        ERROR_LOG("[%s] thread initialize fail, @%d", __func__, __LINE__);
        return false;
	}

	if ((param = thread_data_init(param, mutex, wait_to_obtain_ms, wait_to_release_ms)) == NULL) {
        ERROR_LOG("[%s] thread data initialize fail, @%d", __func__, __LINE__);
        return false;
	}

	if (pthread_create(thread, NULL, threadfunc, (void *) param) != 0) {
        ERROR_LOG("[%s] thread create fail, @%d", __func__, __LINE__);
        return false;
	}

    return true;

    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
}

