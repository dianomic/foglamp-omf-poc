/*
 * Fledge readings ingest.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch, Massimiliano Pinto
 */

#include <simple_https.h>
#include <omf.h>
#include <ingest.h>
#include <reading.h>
#include <chrono>
#include <thread>

using namespace std;

/**
 * Thread to process the ingest queue and send the data
 * to PI Server
 */
static void ingestThread(Ingest *ingest)
{
	uint32_t interval = ingest->getInterval();
	while (ingest->running())
	{
		this_thread::sleep_for(chrono::milliseconds(interval));
		ingest->processQueue();
	}
}

/**
 * Construct an Ingest class to handle the readings queue.
 * A seperate thread is used to send the readings to the
 * storage layer based on time. This thread in created in
 * the constructor and will terminate when the destructor
 * is called.
 */
Ingest::Ingest(uint32_t interval) : m_interval(interval)
{
	// Instantiate an HTTPS handler for "Hostname : port"
	m_sender = new SimpleHttps("192.168.1.157:5460");

	// Instantiate the OMF Class with URL path, OMF_TYTPE_ID and producerToken
	m_omf = new OMF(*m_sender, "/ingress/messages", "1009", "omf_translator_1009");

	m_running = true;
	m_queue = new vector<Reading *>();
	m_thread = new thread(ingestThread, this);
}

/**
 * Destructor for the Ingest class
 *
 * Set's the running flag to false. This will
 * cause the processing thread to drain the queue
 * and then exit.
 * Once this thread has exited the destructor will
 * return.
 */
Ingest::~Ingest()
{
	m_running = false;
	m_thread->join();
	processQueue();
	delete m_queue;
	delete m_thread;
	delete m_omf;
	delete m_sender;
}

/**
 * Check if the ingest process is still running.
 * This becomes false when the service is shutdown
 * and is used to allow the queue to drain and then
 * the procssing routine to terminate.
 */
bool Ingest::running()
{
	return m_running;
}

/**
 * Add a reading to the reading queue
 */
void Ingest::ingest(const Reading& reading)
{
	lock_guard<mutex> guard(m_qMutex);
	m_queue->push_back(new Reading(reading));
	
}

/**
 * Process the queue of readings.
 *
 * Send them to the storage layer as a block. If the append call
 * fails requeue the readings for the next transmission.
 *
 * In order not to lock the queue for an excessie time a new queue
 * is created and the old one moved to a local variable. This minimise
 * the time we hold the queue mutex to the time it takes to swap two
 * variables.
 */
void Ingest::processQueue()
{
vector<Reading *> *savedQ, *newQ;
bool requeue = false;
uint32_t sentReadings;

	newQ = new vector<Reading *>();
	// Block of code to execute holding the mutex
	{
		lock_guard<mutex> guard(m_qMutex);
		savedQ = m_queue;
		m_queue = newQ;
	}

	if ((!savedQ->empty()) &&
		// Send data to PI Server
		!(sentReadings = m_omf->sendToServer(*savedQ)) &&
		requeue == true)
	{
		lock_guard<mutex> guard(m_qMutex);
		m_queue->insert(m_queue->cbegin(), savedQ->begin(), savedQ->end());
	}
	else
	{
		cerr << "Sent " << sentReadings << " readings to PI Server." << endl;

		// Data sent: remove readings from memory
		for (vector<Reading *>::iterator it = savedQ->begin();
						 it != savedQ->end(); ++it)
		{
			Reading *reading = *it;
			delete(reading);
		}
	}
	delete savedQ;
}

/**
 * Return the data sending interval
 */
 
uint32_t Ingest::getInterval()
{
	return m_interval;
}
