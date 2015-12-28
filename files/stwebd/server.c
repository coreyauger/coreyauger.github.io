#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "thread.h"
#include "server.h"



/*=================================================================================
/ handleClientGET: 
/	This function handles the client get requests 
/
/=================================================================================*/
void handleClientGET( struct thread_data *tdata )
{
	int len, i, readlen;
	char buf[ PATH_MAX + NAME_MAX ];
	char path[ PATH_MAX + NAME_MAX ];
	char header[ NAME_MAX ];
	char *filedata;
	int pathLen = 0;
	FILE *file;
		
	appendToLog("Thread %d entering a read.\n", tdata->td_id );
	len = read( tdata->td_socket, buf, BUF_SIZE );
	if( len < 0 ){
		appendToLog("THREAD %d reports (No data to read from socket %d) Returing failure to client.\n", tdata->td_id, tdata->td_socket );
		sprintf( header, "HTTP/1.1 %d\n", SERVER_ERR);
		write( tdata->td_socket, header, strlen(header) );
		webConfig.sc_reqerr++;
		return;
	}	
	for( i=0; i<len; i++ )
		if( buf[i] == '\n' )break;
	if( i > PATH_MAX ){
		appendToLog("PATH length is too long... retunring error header" );
		webConfig.sc_reqbad++;
		sprintf( header, "HTTP/1.1 %d\n\n", REQ_TOLONG );
		write( tdata->td_socket, header, strlen(header) );
		return;		
	}
	
	appendToLog( "Thread %d read %d bytes consisting of:\n%s\nPARSING REQUEST ......\n", tdata->td_id, len, buf );
	if( !memcmp( buf, "GET", 3) )
	{
		for( i=4; i<len; i++ ){
			if( buf[i] == ' ' || buf[i] == '\n' )break;	
		}
		pathLen = strlen(webConfig.sc_pagepath);
		strncpy( path, webConfig.sc_pagepath, pathLen );
		strncpy( &path[pathLen] , &buf[4], (i-4) );
		path[pathLen+(i-4)] = '\0';
		
		for(i=0; i<strlen(path); i++ ){
			if( path[i] == '.' && path[i+1] == '.'  ){
				webConfig.sc_reqbad++;
				appendToLog("BAD REQUEST path contains \"..\" \n" );
				sprintf( header, "HTTP/1.1 %d\n\n", BAD_REQ);
				write( tdata->td_socket, header, strlen(header) );		
			}		
		}
		appendToLog("locationg file: %s\n", path );
		file = fopen( path, "rb" );
		if( !file ){
			// 404 no file error
			webConfig.sc_reqnotfound++;
			appendToLog( "File Not Found Error %d : %s\n", NOT_FOUND, path );
			//"HTTP/1.0 404 NOT FOUND\n\n"
			sprintf( header, "HTTP/1.0 404 NOT FOUND\n\n");
			write( tdata->td_socket, header, strlen(header) );	
			return;
		}
		fseek( file, 0, SEEK_END );			// seek to the end of the file
		len = ftell( file );				// how many bytes is the file
		rewind( file );					// rewind the pointer
		filedata = (char*)malloc( sizeof(char) * len );	// create a buffer for the whole file
		if( !filedata ){					// check if we got the meme
			fprintf( stderr, "Error allocating memory \n" ); // error if not
			appendToLog(  "Error allocating memory \n" );
			exit( 1 );				// exit
		}
		readlen = fread( filedata, 1, len, file );		// read the entire file
		if( readlen != len ){				// check if we got it
			fprintf(stderr, "Error readign bytes with fread... continuing\n");
			appendToLog( "Error readign bytes with fread... continuing\n" );	
		}
		sprintf( header, "HTTP/1.1 %d\n\n", OK);
		webConfig.sc_reqok++;
		write( tdata->td_socket, header, strlen(header) );		
		len = write( tdata->td_socket, filedata, len );
		appendToLog( "File Found and writen to stream : %d bytes\n", len );
		if( len < 0 )
			error( "write" );
		free( filedata );
		fclose( file );
	}
}



/*=================================================================================
/ appendToLog: 
/ takes a format string and a variable number of arguments ( just like printf )
/ The function gets the system time and records the information in the logfile,
/ once we are done we close the file ... so as not to lose log data in the event 
/ of a program failure.
/
/=================================================================================*/
void appendToLog( const char* format, ... )
{
	va_list ap;
	FILE *log;					// log file descriptor ptr	
	time_t mytime;					// time structure
	char *ascii_time;				// ascii time fromat ptr
		
	log = fopen("log", "a+");			// open the logfile
	if( !log ){
		perror("error opening logfile");	// had an error
		exit( 1 );	
	}
	mytime = time( NULL );				// get the current time
	if( mytime < 0 )				// check if we got it
	{
		perror( "error getting time" );		// report an error
	}else{						// else append time to log file
		ascii_time = ctime( &mytime );		// convert to time string
		ascii_time[ strlen(ascii_time)-1 ] = '\0';	
		fprintf( log, "(%s): ",  ascii_time ); 	// add time to log file
	}
	va_start( ap, format );				// find our argument list
	vfprintf( log, format, ap ) ;			// print the arguments to log file
	va_end( ap );					// close
	fclose( log );					// close our log file
}





/*=================================================================================
/ getConfig: 
/ function parses a config file and loads the structure with its contents.
/ The struct is then used by the program to setup values for the server.
/
/=================================================================================*/
struct srvcfg* parseConfig( char *filename )
{
	FILE* 	file;					// file ptr
	char 	*line = NULL;				// temp line
	size_t	len = 0;				// length of line
	ssize_t	read;					// length read in
	char 	*token;					// token string
	int 	stringLen;				// string length counter
	int 	lineNum = 0;				// the line we are on
	time_t mytime;					// time structure
	char *ascii_time;				// ascii time fromat ptr
	
	file = fopen( filename, "r" );			// open the file
	if( !file )					// check for errors
		error( "fopen failer" );		// display errors 
	appendToLog( "Parsing config file: %s", filename ); // log 
	while( (read= getline( &line, &len, file)) != -1 ){	// get a line
		lineNum++;				// remember line num
		token = strtok( line, "\t \n" );	// get a token
		while( token ){				// while we still have tokens
			if( token[0] == '#' )break;	// if it is a comment break
			if( !strncmp( token, "PORT", 4 ) ){	// port token
				token = strtok( NULL, "\t \n" );
				webConfig.sc_port = atoi( token );
				appendToLog("port: %d\n", webConfig.sc_port );
				break;
			}else if( !strncmp( token, "CONNECTIONS", 11 ) ){ // connections token
				token = strtok( NULL, "\t \n" );
				webConfig.sc_numthreads = atoi( token );
				appendToLog("threads: %d\n", webConfig.sc_numthreads );
				break;
			}else if( !strncmp( token, "LOGPATH", 7 ) ){ // logpath token
				token = strtok( NULL, "\t \n" );
				stringLen = strlen( token );
				if( stringLen > PATH_MAX + NAME_MAX ){
					fprintf( stderr, "Abnormaly large path in config file line %d\n", lineNum );
					appendToLog( "Abnormaly large path in config file line %d\n", lineNum );
					exit( 1 );	
				}
				if( webConfig.sc_logpath )
					free( webConfig.sc_logpath );
				webConfig.sc_logpath = (char*)malloc( stringLen * sizeof( char) );
				strncpy( webConfig.sc_logpath, token, stringLen );
				appendToLog("logpath: %s\n", webConfig.sc_logpath );
				break;			
			}else if( !strncmp( token, "PAGEPATH", 8 ) ){ // pagepath token
				token = strtok( NULL, "\t \n" );
				stringLen = strlen( token );
				if( stringLen > PATH_MAX + NAME_MAX ){
					fprintf( stderr, "Abnormaly large path in config file line %d\n", lineNum );
					appendToLog( "Abnormaly large path in config file line %d\n", lineNum );
					exit( 1 );	
				}
				if( webConfig.sc_pagepath )
					free( webConfig.sc_pagepath );
				webConfig.sc_pagepath = (char*)malloc( stringLen * sizeof( char) );
				strncpy( webConfig.sc_pagepath, token, stringLen );
				appendToLog("pagepath: %s\n", webConfig.sc_pagepath );
				break;
			}else{						// unknown token
				fprintf( stderr, "Unknown token in config file: %s\n", token );
				appendToLog( "Unknown token in config file: %s\n", token );
				exit( 1 );
			}
		}	
	}
	if( line )					// check if there is memory
		free( line );				// free the memory
	fclose( file );					// close file
	appendToLog( "Finished Config File parse" ); // log 
	ascii_time = ctime( &mytime );		// convert to time string
	ascii_time[ strlen(ascii_time)-1 ] = '\0';	
	webConfig.sc_date = ascii_time;			// record the date
	webConfig.sc_time = time( NULL );		// record the time
	webConfig.sc_reqok = 0;				// set variables to zero
	webConfig.sc_reqbad = 0;
	webConfig.sc_reqerr = 0;
	webConfig.sc_reqnotfound = 0;
	return &webConfig;				// return the config struct
}


