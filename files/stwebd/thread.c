
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "thread.h"
#include "server.h"

volatile sig_atomic_t threads_waiting = 0;	// init threads waiting to 0

void initThreadPool( size_t pool_size, pthread_mutex_t *m_wait, pthread_cond_t *p_cond_t, void (*td_handler)( struct thread_data* ) )
{
	int i;										// iterator
	 
	thread_data = (struct thread_data*)calloc( pool_size, sizeof( struct thread_data ) );	// get memory for thread pool
	if( !thread_data )									// check if we got it
		error( "calloc filed to allocate thread_data" );			// error if we are out of mem
	for( i=0; i<pool_size; i++ )							// init threads in a loop
	{
		appendToLog( "SPAWNING an evil thread ( mwaaa ha ha ha )\n" ); 		// log thread spawn
		thread_data[i].td_id = i;							// record unique id
		thread_data[i].td_socket = 0;							// set socket to 0
		thread_data[i].td_mutex_wait = m_wait;						// init mutex
		thread_data[i].td_cond_wait = p_cond_t;						// init cond
		thread_data[i].td_handler = td_handler;						// assign a handler
		thread_data[i].td_active = 0;							// make inactive
		thread_data[i].td_post_handler = NULL;						// set the post handler to null
		appendToLog("Creating Thread: %d\n", i );				// log the creation
		if( pthread_create( &thread_data[i].td_thread, NULL, &threadManager, (void*)&thread_data[i] ) ) // create and store pthread
			error( "pthread_create failure" );				// record any errors
		appendToLog( "Thread %d initialized \n", i );				// log the threads creation
	}	
}



void* threadManager( void* thread_args )
{
	struct thread_data *tdata;							// thread data pointer
	tdata = (struct thread_data*)thread_args;					// get the thread data
	for( ;; )									// loop forever
	{
		if( tdata->td_post_handler )						// if we have a post handler function 
			(*tdata->td_post_handler)( tdata );				// call the post handler function	
		tdata->td_active = 0;							// set active bit low
		appendToLog( "Thread %d entering a sleep state (sleep my precious... sleep!! )\n", tdata->td_id );
		++threads_waiting;							// increment the wait counter (could hav race cond here)
		if( pthread_cond_wait(tdata->td_cond_wait, tdata->td_mutex_wait) )	// sleep the thread until signal
			error("pthread_cond_wait failure");
		appendToLog( "Thread %d ACTIVE ( the hoard is restless )\n", tdata->td_id); // log that we have active thread
		tdata->td_active = 1;							// set active bit high
		tdata->td_socket = conn_fd;						// grab the socket
		appendToLog( "Thread %d grabbing socket %d (ready to do your evil biding)\n", tdata->td_id, tdata->td_socket );			
		pthread_mutex_unlock( tdata->td_mutex_wait );				// unlock mutex so another thread may get signaled
	
		(*tdata->td_handler)( tdata );						// call the handler function
		close( tdata->td_socket );						// close the socket
		tdata->td_socket = 0;							// reset our socket
		appendToLog( "Thread %d closed socket \n", tdata->td_id );		// log that socket was closed
		webConfig.sc_connserv++;
	}
}

void threadFree( struct thread_data* tdata )
{
	appendToLog( "FREEING THREADPOOL MEM\n" );
	free( tdata );
}



// thread handler to kill the thread
void threadKiller( struct thread_data *tdata )
{
	tdata->td_active = 0;				// set thread state to inactive
	--webConfig.sc_numthreads;			// decrement thread count
	appendToLog( "Destroying thread: %d (farewell my pet)  \n", tdata->td_id ); 
	pthread_exit( NULL );				// kill the thread
}






