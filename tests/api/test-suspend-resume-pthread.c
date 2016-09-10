/*
 *  duk_suspend() and duk_resume().
 *
 *  This test uses pthreads for stressing the suspend/resume mechanism a bit.
 */

#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*---
{
    "pthread": true
}
---*/

/*===
create mutex
pthread_mutex_init: 0
spawn threads
wait for threads to finish
join threads
final globalCounter: 100000
destroy mutex and exit
pthread_mutex_destroy: 0
all done
===*/

#define NUM_THREADS 100

static pthread_mutex_t duktape_lock;
static pthread_t threads[NUM_THREADS];
static volatile int finish_count = 0;

static void my_lock(void) {
	int rc;
	rc = pthread_mutex_lock(&duktape_lock);
	if (rc != 0) {
		printf("pthread_mutex_lock: %d\n", rc); fflush(stdout);
	}
}

static void my_unlock(void) {
	int rc;
	rc = pthread_mutex_unlock(&duktape_lock);
	if (rc != 0) {
		printf("pthread_mutex_lock: %d\n", rc); fflush(stdout);
	}
}

/* In most cases don't sleep, in some cases sleep a short time (10 ms maximum)
 * and in rare cases sleep for a long time (1 second maximum).
 */
static void random_sleep_maybe(void) {
	int r = rand() % 1000;
	if (r < 950) {
		return;
	}
	if (r < 999) {
		r = rand() % 10000;
	} else {
		r = rand() % 1000000;
	}
	(void) usleep(r);
}

static void *thread_start(void *arg) {
	duk_context *ctx = (duk_context *) arg;
	int i;

	/* Increment a global counter, suspending between each increment.
	 * Use random sleeps to vary the contention behavior.
	 */
	for (i = 0; i < 1000; i++) {
		random_sleep_maybe();
		my_lock();
		duk_eval_string_noresult(ctx, "globalCounter++;");
		random_sleep_maybe();
		my_unlock();
	}

	my_lock();
	finish_count++;
	my_unlock();

	return NULL;
}

void test(duk_context *ctx) {
	int rc;
	int i;

	srand((unsigned int) time(NULL));

	duk_eval_string_noresult(ctx, "globalCounter = 0;");

	memset((void *) &duktape_lock, 0, sizeof(duktape_lock));
	memset((void *) threads, 0, sizeof(threads));

	printf("create mutex\n"); fflush(stdout);

	rc = pthread_mutex_init(&duktape_lock, NULL);
	printf("pthread_mutex_init: %d\n", rc); fflush(stdout);
	if (rc != 0) {
		return;
	}

	printf("spawn threads\n"); fflush(stdout);
	my_lock();
	for (i = 0; i < NUM_THREADS; i++) {
		/* Each thread gets a Duktape thread which is passed as the
		 * thread 'arg'.  We keep the mutex while we're creating
		 * the threads so that the threads don't do anything until
		 * all threads have been created.  In this test the threads
		 * share the same global environment but that doesn't have
		 * to be the case.
		 *
		 * The Duktape threads are left on the original value stack
		 * so that they remain reachable.  A proper scheduler would
		 * keep them e.g. in a stash object.
		 */
		(void) duk_push_thread(ctx);
		rc = pthread_create(threads + i, NULL, thread_start, (void *) duk_get_context(ctx, -1));
		if (rc != 0) {
			printf("pthread_create: %d\n", rc); fflush(stdout);
			break;
		}
	}
	my_unlock();

	printf("wait for threads to finish\n"); fflush(stdout);
	for (;;) {
		if (finish_count == NUM_THREADS) {
			break;
		}
		(void) usleep(1000000);
	}

	printf("join threads\n"); fflush(stdout);
	for (i = 0; i < NUM_THREADS; i++) {
		void *retval;
		rc = pthread_join(threads[i], &retval);
		if (rc != 0) {
			printf("pthread_join: %d\n", rc); fflush(stdout);
			break;
		}
	}

	duk_eval_string_noresult(ctx, "print('final globalCounter:', globalCounter);");

	printf("destroy mutex and exit\n"); fflush(stdout);

	rc = pthread_mutex_destroy(&duktape_lock);
	printf("pthread_mutex_destroy: %d\n", rc); fflush(stdout);

	printf("all done\n"); fflush(stdout);
}
