#ifndef _HTTP_SENDER_H
#define _HTTP_SENDER_H
/*
 * Fledge HTTP Sender wrapper.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */

#include <string>
#include <vector>

#define HTTP_SENDER_USER_AGENT     "Fledge http sender"
#define HTTP_SENDER_DEFAULT_METHOD "GET"
#define HTTP_SENDER_DEFAULT_PATH   "/"

class HttpSender
{
	public:
		/**
		 * Constructor:
		 */
		HttpSender();

		// Destructor
		~HttpSender();

		/**
		 * HTTP(S) request: pass method and path, HTTP headers and POST/PUT payload.
		 */
		virtual int sendRequest(const std::string& method,
				const std::string& path,
				const std::vector<std::pair<std::string, std::string> >& headers,
				const std::string& payload) = 0;
		
};

#endif
