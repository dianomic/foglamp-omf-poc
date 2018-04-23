/*
 * FogLAMP storage service.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch, Massimiliano Pinto
 */
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <reading.h>
#include <ingest.h>
#include <iostream>

extern int makeDaemon(void);

using namespace std;

int makeDaemon();
void start();
Reading takeReading();

/**
 * South service main entry point
 */
int main(int argc, char *argv[])
{
	bool	       daemonMode = true;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-d"))
		{
			daemonMode = false;
		}
	}

	if (daemonMode && makeDaemon() == -1)
	{
		// Failed to run in daemon mode
		cout << "Failed to run as deamon - proceeding in interactive mode." << endl;
	}

	start();
	return 0;
}

/**
 * Detach the process from the terminal and run in the background.
 */
int makeDaemon()
{
pid_t pid;

	/* create new process */
	if ((pid = fork()  ) == -1)
	{
		return -1;  
	}
	else if (pid != 0)  
	{
		exit (EXIT_SUCCESS);  
	}

	// If we got here we are a child process

	// create new session and process group 
	if (setsid() == -1)  
	{
		return -1;  
	}

	// Close stdin, stdout and stderr
	close(0);
	close(1);
	close(2);
	// redirect fd's 0,1,2 to /dev/null
	(void)open("/dev/null", O_RDWR);  	// stdin
	(void)dup(0);  			// stdout	GCC bug 66425 produces warning
	(void)dup(0);  			// stderr	GCC bug 66425 produces warning
 	return 0;
}

/**
 * Start the data ingestion loop
 */
void start()
{
	Ingest ingest(10000);

	while (true)
	{
		sleep(1);
		Reading reading = takeReading();
		ingest.ingest(reading);
		//cerr << "Added a reading" << endl;
	}
}


/**
 * Create a Reading data with a ranon value
 */
Reading takeReading()
{
        srand(time(0));
        int rndValue = 1 + 10 * (rand() % 100);
        DatapointValue value(rndValue);
        return Reading("south_sensor", new Datapoint("random", value));
}
