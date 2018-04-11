/*
 * FogLAMP HTTP Sender wrapper.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto, Mark Riddoch
 */

#include <http_sender.h>

using namespace std;

/**
 * Constructor
 */
HttpSender::HttpSender(const string& host_port) : m_host_port(host_port)
{
}

/**
 * Destructor
 */
HttpSender::~HttpSender()
{
}
