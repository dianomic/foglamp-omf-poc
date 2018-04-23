#ifndef _INGEST_H
#define _INGEST_H
/*
 * FogLAMP reading ingest.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch, Massimiliano Pinto
 */

#include <simple_https.h>
#include <reading.h>
#include <omf.h>
#include <vector>
#include <thread>
#include <mutex>

/**
 * This ingest class is used to ingest asset readings to PI Server.
 * It maintains a queue of readings to be sent to storage,
 * these are sent using a background thread that regularly
 * wakes up and sends the queued readings.
 */
class Ingest {

public:
	Ingest(uint32_t interval);
	~Ingest();

	void		ingest(const Reading& reading);
	bool		running();
	void		processQueue();
	uint32_t	getInterval();

private:
	bool			m_running;
	std::vector<Reading *>	*m_queue;
	std::mutex		m_qMutex;
	std::thread		*m_thread;
	// Ingest thread interval
	uint32_t		m_interval;
	// OMF interface to Pi Server
	OMF*			m_omf;
	// HTTP sender interface
	SimpleHttps*		m_sender;
};
#endif
