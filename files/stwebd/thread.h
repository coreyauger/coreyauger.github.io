
#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>


extern int conn_fd;	// todo: could be done a little better


/*=================================================================================
/ thread_data: 
/ This is all the data associated with a thread. 
/=================================================================================*/
struct thread_data{
	pthread_t td_thread;				// the pthread
	int td_id;					// a unique thread id
	int td_socket;					// the socket that the thread acts on
	struct sockaddr_in td_client_addr;		// the client address
	pthread_mutex_t *td_mutex_wait;			// mutex for the wait condition
	pthread_cond_t  *td_cond_wait;			// wait condition
	void (*td_handler)( struct thread_data* );	// function ptr to a handler
	volatile sig_atomic_t td_active;		// active/wait flag
	void (*td_post_handler)( struct thread_data* );	// function ptr to a post handler
};
struct thread_data *thread_data;

// initialize the thread pool
void initThreadPool( size_t pool_size, pthread_mutex_t *m_wait, pthread_cond_t *p_cond_t, void (*td_handler)( struct thread_data* )  );
void* threadManager( void* thread_args ); 	// thread wait/active routine
void threadFree( struct thread_data* tdata );	// free the thread pool mem
void threadKiller( );				// function handler to destroy a thread
struct thread_data* getThreadDatat( );		// return a pointer to our thread data struct

extern void error( char *msg );			// fatal error function imported from main.c

#endif

