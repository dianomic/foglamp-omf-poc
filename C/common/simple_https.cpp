/*
 * FogLAMP HTTP Sender implementation using the
 * HTTPS Simple Web Server library
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto, Mark Riddoch
 */

#include <http_sender.h>
#include <client_https.hpp>

using namespace std;

// Using https://github.com/eidheim/Simple-Web-Server
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

/**
 * Constructor
 */
SimpleHttps::SimpleHttps(const string& host_port)
{
	m_sender = new HttpsClient(host_port, false);
	super();
}

/**
 * Destructor
 */
SimpleHttps::~SimpleHttps()
{
	delete m_sender;
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
int SimpleHttps::sendRequest(const string& method,
			    const string& path,
			    const vector<pair<string, string>>& headers,
			    const string& payload)
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

	// Call HTTPS method
	try
	{
		auto res = m_sender->request(method, path, payload, header);
		retCode = res->status_code;
	} catch (exception& ex) {
		cerr << "Failed to send data: " << ex.what() << endl;
		return 0;
	}

	return atoi(retCode.c_str());
}
