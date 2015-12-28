
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stropts.h>
#include "sighandles.h"
#include "thread.h"
#include "server.h"

extern pthread_cond_t  cond_wait;
extern volatile sig_atomic_t threads_waiting;
extern int socketfd;
extern int SIGNAL;

void hijackSignalHandlers()
{
	sigfillset( &sig_mask );		// mask all signals
	
	sigint.sa_handler = &sigintHandler;	// signal handler function
	sigint.sa_mask = sig_mask;		// mask all signals
	sigint.sa_flags = SA_RESTART;		// set flags
	appendToLog( "Stole signal SIGINT, handler at: %p \n", &sigintHandler );
	
	sighup.sa_handler = &sighupHandler;
	sighup.sa_mask = sig_mask;
	sighup.sa_flags = SA_RESTART;
	appendToLog( "Stole signal SIGHUP, handler at: %p \n", &sighupHandler );
	
	sigusr1.sa_handler = &sigusr1Handler;
	sigusr1.sa_mask = sig_mask;
	sigusr1.sa_flags = SA_RESTART;
	appendToLog( "Stole signal SIGUSR1, handler at: %p \n", &sigusr1Handler );
	
	sigusr2.sa_handler = &sigusr2Handler;
	sigusr2.sa_mask = sig_mask;
	sigusr2.sa_flags = SA_RESTART;
	appendToLog( "Stole signal SIGUSR2, handler at: %p \n", &sigusr2Handler );
	
	sigaction( SIGINT, &sigint, NULL );	// steal signal interupts
	sigaction( SIGHUP, &sighup, NULL );
	sigaction( SIGUSR1, &sigusr1, NULL );
	sigaction( SIGUSR2, &sigusr2, NULL );	
}


void sigintHandler( int sid )
{
	// dump the status info to a file
	time_t now;
	 
	now = time( NULL );
	appendToLog("Recieved SIGINT signal (dump status info)\n");
	printf("SERVER STATUS DUMP\n");
	printf("Port: %d\n", webConfig.sc_port );
	printf("Path: %s\n", webConfig.sc_pagepath );
	printf("Log File: %s\n", webConfig.sc_logpath );
	printf("Number of Threads: %d\n", webConfig.sc_numthreads );
	printf("Server Started On: %s\n", webConfig.sc_date );
	printf("Connections: %d\n", webConfig.sc_conns );
	printf("Connections Serviced: %d\n", webConfig.sc_connserv );
	printf("Uptime: %d seconds\n", (int)now - (int)webConfig.sc_time ); 
	printf("Number of OK requests: %d\n", webConfig.sc_reqok );
	printf("Number of BAD requests: %d\n", webConfig.sc_reqbad );
	printf("Number of 404 requests: %d\n", webConfig.sc_reqnotfound );
	printf("Number of SERVER_ERROR requests: %d\n", webConfig.sc_reqerr );
}


void sighupHandler( int sid )
{
	int i;
	appendToLog("Recieved SIGHUP signal (hard restart)\n");
	close( socketfd );				// close the listning socket
	for( i=0; i<webConfig.sc_numthreads; i++ ){	// loop through the threads
		appendToLog("killing thread %d\n", thread_data[i].td_id );
		pthread_kill( thread_data[i].td_thread , SIGNAL_KILL );	// kill the thread
		thread_data[i].td_active = 0;		// no longer active
	}
	webConfig.sc_numthreads = 0;
	SIGNAL= SIGNAL_FORK;				// signal main to fork

}

void sigusr1Handler( int sid )
{
	// dissallow new connections finish old then terminate server.
	int i;
	appendToLog("Recieved SIGUSR1 signal (finish with connections and shut down)\n");
	close( socketfd );				// accept no more connections
	appendToLog("Accpting no more connections\n");
	for( i=0; i<webConfig.sc_numthreads ; i++ ){	// give the threads an exit handler 
		thread_data[i].td_handler = &threadKiller;
		thread_data[i].td_post_handler = &threadKiller;
	}
	pthread_cond_broadcast( &cond_wait ); 		// signal all waiting threads to go and die
	appendToLog("Signaling waiting threads to die.\n");
	for( i=0; i<webConfig.sc_numthreads ; i++ ){	// for each thread
		if( thread_data[i].td_active )		// if it is still active
			pthread_join( thread_data[i].td_thread, NULL );	// wait till it dies
	}	
	SIGNAL = SIGNAL_EXIT;				// tell main to exit now
}

void sigusr2Handler( int sid )
{
	// soft restart
	int i;
	appendToLog("Recieved SIGUSR2 signal (soft restart)\n");
	close( socketfd );				// accept no more connections
	appendToLog("Accpting no more connections from current cofiguration\n");
	for( i=0; i<webConfig.sc_numthreads; i++ ){	// give the threads an exit handler 
		thread_data[i].td_handler = &threadKiller;
		thread_data[i].td_post_handler = &threadKiller;
	}
	pthread_cond_broadcast( &cond_wait ); 		// signal all waiting threads to go and die	
	appendToLog("Signaling waiting threads to die.\n");
	SIGNAL= SIGNAL_FORK;				// signal main to fork
}





