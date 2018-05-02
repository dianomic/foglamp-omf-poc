#ifndef _MBED_HTTPS_H
#define _MBED_HTTPS_H
/*
 * FogLAMP HTTPS implementation for ARM MbedOS
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */

#include <string>
#include <vector>
#include "mbed.h"
#include "easy-connect.h"
#include "https_request.h"
#include "http_sender.h"


class MbedHttps: public HttpSender
{
	public:
		/**
		 * Constructor:
		 */
		MbedHttps(const std::string& host_port);

		// Destructor
		~MbedHttps();

		/**
		 * HTTP(S) request: pass method and path, HTTP headers and POST/PUT payload.
		 */
		int sendRequest(const std::string& method,
				const std::string& path,
				const std::vector<std::pair<std::string, std::string> >& headers,
				const std::string& payload);
	private:
		void			checkNetwork();
		void			socketSetup();
		TLSSocket		*m_socket;
		std::string		m_host_port;
		NetworkInterface	*m_network;
		std::string		m_host;
		int			m_port;
};

#endif
