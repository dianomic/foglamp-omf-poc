#ifndef _HTTP_SENDER_H
#define _HTTP_SENDER_H_H
/*
 * FogLAMP HTTP Sender wrapper.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */

#include <string>
#include <vector>

#define HTTP_SENDER_USER_AGENT     "FogLAMP http sender"
#define HTTP_SENDER_DEFAULT_METHOD "GET"
#define HTTP_SENDER_DEFAULT_PATH   "/"

typedef void HTTP_SENDER;

class HttpSender
{
	public:
		/**
		 * Constructor:
		 * pass schema, host:port, HTTP headers and POST/PUT payload.
		 */
		HttpSender(const std::string& schema, const std::string& host_port);

		// Destructor
		~HttpSender();

		/**
		 * HTTP(S) request: pass method and path, HTTP headers and POST/PUT payload.
		 */
		int sendRequest(const std::string& method = std::string(HTTP_SENDER_DEFAULT_METHOD),
				const std::string& path = std::string(HTTP_SENDER_DEFAULT_PATH),
				const std::vector<std::pair<std::string, std::string>>& headers = {},
				const std::string& payload = std::string()) noexcept;
	private:
		std::string	m_host_port;
		HTTP_SENDER	*m_sender;
		bool		is_https;
		
};

#endif
