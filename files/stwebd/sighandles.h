

#ifndef _SIGHANDELS_H
#define _SIGHANDLES_H

#define SIGNAL_EXIT 1					// some of my own signals (EXIT)
#define SIGNAL_FORK 2					// FORK

#define SIGNAL_KILL 9					// Kill signal

#include <signal.h>					// signal definitions

struct sigaction sigint, sighup, sigusr1, sigusr2;	// sigaction def
sigset_t sig_mask;					// signal mask

void hijackSignalHandlers();				// function to hijack all the signals we want

void sigintHandler( int sid );				// handler functions
void sighupHandler( int sid );
void sigusr1Handler( int sid );
void sigusr2Handler( int sid );
extern void error( char *msg );				// error function imported from main

#endif

