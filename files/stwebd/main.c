
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "thread.h"
#include "server.h"
#include "sighandles.h"

#define TCP 	0		// TCP or UDP protocol depending on socket
#define CONNECTIONS 5		// number of waiting connections
#define DEFAULT_CFG ".srvcfg"	// default configuration file

int conn_fd;
int SIGNAL=0;
extern volatile sig_atomic_t threads_waiting;
pthread_cond_t  cond_wait = PTHREAD_COND_INITIALIZER;
int socketfd;

/*=================================================================================
/ error: 
/ Fatal error routine.  Reports the error to stderr and the log file before 
/ terminating the program.
/
/==================================================================================*/
void error( char *msg )
{
	perror( msg );					// print error to stderr
	appendToLog( " (**) FATAL ERROR: %s\n", msg );	// print msg to logfile
	exit( 1 );					// exit with an error
}



/*=================================================================================
/ main: 
/ When the program starts we read the configuration file.  We then open our logfile 
/ and record some information on the configuration.  Next we create a socket to 
/ listen on.  We initialize a thread pool and begin listening for connections.  
/ When a connection is formed we activate a thread to handle the client request.
/
/=================================================================================*/
int main( int argv, char *argc[] )
{
	int addr_len, i, p;
	struct sockaddr_in server_addr, client_addr;
	struct srvcfg* webConfig;
	pthread_mutex_t mutex_wait = PTHREAD_MUTEX_INITIALIZER;
	
	appendToLog( "\n\n============PROGRAM START================\n" );// record program start to log
	webConfig = parseConfig( DEFAULT_CFG );			// load config info
	
	socketfd = socket( AF_INET, SOCK_STREAM, TCP );		// get a socket fd entry
	appendToLog( "got a socket fd: %d\n", socketfd );	// record socket to logfile
	if( socketfd < 0 )					// error getting socket
		error("socket failed");
	bzero( (char*)&server_addr, sizeof( server_addr) );	// zero our memory
	server_addr.sin_family = AF_INET;			// set domain to internet
	server_addr.sin_port = htons( webConfig->sc_port );	// set port ( in network byte order)
	server_addr.sin_addr.s_addr = INADDR_ANY;		// set server IP to this machinge
	p=1;
	if( setsockopt( socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&p, sizeof(int) ) ) // allow the socket to be reused
		error( "setsockopt failed" );	
	if( bind( socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) < 0 ) // assign a name to the socket
		error("bind failed");				// failure to bind
	if( listen( socketfd, CONNECTIONS ) < 0)		// listen for connectionsi *** NEED TO READ MORE ON MAX CONN
		error("listen failure");			// report listening error 
	appendToLog( "listening on socket ... \n" );		// log sucsess of listen
	
	appendToLog( "initializing thread pool\n" );		// start init of thread pool
	initThreadPool( webConfig->sc_numthreads, &mutex_wait, &cond_wait, &handleClientGET ); // call thread pool init 
	
	appendToLog( "stealing interupt handlers...\n" );	// log the attempt to steal signals
	hijackSignalHandlers();					// steal the signal handlers
	appendToLog( "signal theft compleate\n" );		// log signal theft compleate
	
	addr_len = sizeof( struct sockaddr_in );		// store length since it is used a few times
	for(;;)							// loop forever
	{
		appendToLog( "Number waiting threads: %d\n", threads_waiting );
		while( !threads_waiting ){			// busy wait on waiting_thread counter
			; 					// should try to sleep main thread for a bit here
		}
		conn_fd = accept( socketfd, (struct sockaddr*)&client_addr, &addr_len );// listen for client connects
		if( conn_fd < 0 ){				// do we have an exception 
			if( SIGNAL == SIGNAL_EXIT ){			// we have a shutdown signal
				threadFree( thread_data );	// free the memory 
				pthread_mutex_destroy( &mutex_wait ); 	// destroy mutex
				pthread_cond_destroy( &cond_wait );	// destroy wait	
				exit( 0 );				// exit the program
			}else if( SIGNAL == SIGNAL_FORK ){
				SIGNAL = 0;				// reset signal
				if( fork() ){				// create new server
					for( i=0; i<webConfig->sc_numthreads; i++ ){ // loop through threads 
						if( thread_data[i].td_active ) // check if it is active
							pthread_join( thread_data[i].td_thread, NULL );	// wait till it dies
					}
					threadFree( thread_data );// free the thread pool memory
					appendToLog( "(**) SHUT DOWN\n" );	// record the program terminiation
					exit( 0 );	// exit the program
				}
				if( execv( *argc, argc ) < 0 )	// exec a new copy into the fork
					error("execv error");
			}else{					// we have a real error so log it and exit
				error( "accept failed" );
			}
		}else{
			webConfig->sc_conns++;
			appendToLog( "main thread accepting connection on socket desriptor: %d (SIGNALING THREAD TO HANDLE)\n", conn_fd );	
			//appendToLog( "Client: %x : %d\n", ntohl(client_addr.sin_addr.s_addr), ntohl(client_addr.sin_port) );
			pthread_cond_signal( &cond_wait ); 	// singnal a wiating thread
			--threads_waiting;			// decrement the waiting threads count
		}
	}
	return 0;						// return sucsess
}

