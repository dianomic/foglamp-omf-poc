/*
 * FogLAMP HTTP Sender wrapper.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */

#include <http_sender.h>
#include <client_http.hpp>
#include <client_https.hpp>

using namespace std;

// Using https://github.com/eidheim/Simple-Web-Server
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

/**
 * Constructor
 */
HttpSender::HttpSender(const string& schema,
		       const string& host_port) : m_host_port(host_port)
{
	if (schema.compare("https") == 0)
	{
		// No remote server certificate verification
		is_https = true;
		m_sender = reinterpret_cast<HttpsClient*>(new HttpsClient(host_port, false));
	}
	else
	{
		is_https = false;
		m_sender = reinterpret_cast<HttpClient*>(new HttpClient(host_port));
	}
}

/**
 * Destructor
 */
HttpSender::~HttpSender()
{
	if (is_https)
	{
		delete reinterpret_cast<HttpsClient*>(m_sender);
	}
	else
	{
		delete reinterpret_cast<HttpClient*>(m_sender);
	}
}

/**
 * Send data
 *
 * @param method    The HTTP method (GET, POST, ...)
 * @param path      The URL path
 * @param headers   The optional headers to send
 * @param payload   The optional data payload (for POST, PUT)
 * @return          The HTTP code on success or 0 on execptions
 */
int HttpSender::sendRequest(const string& method,
			    const string& path,
			    const vector<pair<string, string>>& headers,
			    const string& payload) noexcept
{
	SimpleWeb::CaseInsensitiveMultimap header;

	// Add FogLAMP UserAgent
	header.emplace("User-Agent", HTTP_SENDER_USER_AGENT);

	// Add custom headers
	for (auto it = headers.begin(); it != headers.end(); ++it)
	{
		header.emplace((*it).first, (*it).second);
	}

	string retCode;

	// Call HTTPS or HTTP method
	try
	{
        	if (is_https)
		{       
                	HttpsClient *handler = reinterpret_cast<HttpsClient*>(m_sender);
			auto res = handler->request(method, path, payload, header);
			retCode = res->status_code;
	        }
		else
		{
			HttpClient *handler = reinterpret_cast<HttpClient*>(m_sender);
			auto res = handler->request(method, path, payload, header);
			retCode = res->status_code;
        	}
	} catch (exception& ex) {
		cerr << "Failed to send data: " << ex.what() << endl;
		return 0;
	}

	return atoi(retCode.c_str());
}
