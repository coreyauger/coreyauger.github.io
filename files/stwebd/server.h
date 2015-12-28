#ifndef _SERVER_H

#define BUF_SIZE	1024
#include <stdarg.h>
#include <time.h>

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif
#ifndef NAME_MAX
	#define NAME_MAX 256
#endif

void handleClientGET( );		// handle the GET request
void appendToLog( const char* format, ... );	// append to logfile method
int readFile( FILE *file, char *buf );		// read an entire file 
extern void error( char *msg );

struct srvcfg{				// server configuration structure
	int 	sc_port;	
	char*	sc_pagepath;
	int	sc_numthreads;
	char*	sc_configpath;
	char*	sc_logpath;
	int	sc_conns;
	int	sc_connserv;
	char*	sc_date;
	time_t	sc_time;
	int	sc_reqok;
	int	sc_reqbad;
	int	sc_reqerr;
	int	sc_reqnotfound;
};

struct srvcfg webConfig;		// config struct


struct srvcfg* parseConfig( char *filename );	// parse a config file

enum ResponseCodes { OK=200, BAD_REQ=400, NOT_FOUND=404, REQ_TOLONG=414, SERVER_ERR=500 };


#endif

