/*
 * Fledge HTTP Sender implementation using the
 * ARM MbedOS http library
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */
#include <mbed.h>
#include "ntp-client/NTPClient.h"
#include <mbed_https.h>
#include <iostream>
#include <NetworkInterface.h>

#define TRACE_MESSAGES	0

using namespace std;

/* List of trusted root CA certificates
 * currently two: GlobalSign, the CA for developer.mbed.org and Let's Encrypt, the CA for httpbin.org
 *
 * To add more root certificates, just concatenate them.
 */
const char SSL_CA_PEM[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n"
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n"
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n"
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n"
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n"
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n"
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n"
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n"
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n"
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n"
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n"
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n"
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n"
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n"
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n"
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n"
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n"
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n"
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n"
    "-----END CERTIFICATE-----\n";

/**
 * Constructor
 */
MbedHttps::MbedHttps(const string& host_port) : HttpSender(), m_socket(0), m_host_port(host_port)
{
	m_network = easy_connect(true);
	if (!m_network)
	{
		printf("No network\n");
	}
	NTPClient ntp(m_network);
	ntp.set_server(NTP_SERVER_NAME, 123);
	time_t timestamp = ntp.get_timestamp(25000);
	if (timestamp > 0)
	{
		set_time(timestamp);
	}
	else
		printf("Failed to get time %lld\n", timestamp);
printf("1\n");
	m_host = host_port.substr(0, host_port.find(":"));
	m_port = atoi(host_port.substr(host_port.find(":") + 1, string::npos).c_str());
printf("Host %s, Port %d\n", m_host.c_str(), m_port);
	socketSetup();
}

/**
 * Destructor
 */
MbedHttps::~MbedHttps()
{
}

void MbedHttps::socketSetup()
{
	if (m_socket)
		delete m_socket;
	m_socket = new TLSSocket(m_network, m_host.c_str(), m_port, SSL_CA_PEM);
	m_socket->set_debug(true);
	if (m_socket->connect() != 0) {
    		printf("TLS Connect failed %d\n", m_socket->error());
	}
}

/**
 * Check we have a connection to the network
 */
void MbedHttps::checkNetwork()
{
	if (!m_network)
	{
		m_network = easy_connect(true);
		if (!m_network)
		{
			printf("No network\n");
		}
		NTPClient ntp(m_network);
		ntp.set_server(NTP_SERVER_NAME, 123);
		time_t timestamp = ntp.get_timestamp(25000);
		if (timestamp > 0)
		{
			set_time(timestamp);
		}
		else
			printf("Failed to get time %lld\n", timestamp);
	}
	else if (m_network->get_connection_status() == NSAPI_STATUS_DISCONNECTED)
	{
		m_network->connect();
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
int MbedHttps::sendRequest(const string& method,
			    const string& path,
			    const vector<pair<string, string> >& headers,
			    const string& payload)
{
HttpsRequest *req = 0;

	checkNetwork();
	if (!m_network)
	{
		printf("No network\n");
		return 400;
	}

	string url("https://");
	url += m_host_port;
	url += path;
	if (method.compare("GET") == 0)
		req = new HttpsRequest(m_socket, HTTP_GET, url.c_str());
	else if (method.compare("POST") == 0)
		req = new HttpsRequest(m_socket, HTTP_POST, url.c_str());

	// Add Fledge UserAgent
	// req->set_header("User-Agent", HTTP_SENDER_USER_AGENT);
	// Add custom headers
	vector<pair<string, string> >::const_iterator it;
	for (it = headers.begin(); it != headers.end(); ++it)
	{
		req->set_header((*it).first, (*it).second);
	}

	HttpResponse *res = 0;
	if (method.compare("GET") == 0)
	{
		res = req->send();
	}
	else if (method.compare("POST") == 0)
	{
		res = req->send(payload.c_str(), payload.length());
	}

	if (!res)
	{
		printf("Request Failed %d\n", req->get_error());
		int err = req->get_error();
		delete req;
		socketSetup();
		return err;
	}
#if TRACE_MESSAGES
	cerr << url << " " << res->get_status_code() << endl;
	cerr << payload << endl;
#endif

	int code = res->get_status_code();
	delete req; // Also deletes the response

	return code;
}
