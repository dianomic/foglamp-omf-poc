/*
 * FogLAMP OSI Soft OMF interface to PI Server.
 *
 * Copyright (c) 2018 OSIsoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */

#include <iostream>
#include <map>
#include <datapoint.h>
#include <reading.h>
#include <omf.h>
#include <simple_https.h>

using namespace std;

int main(void)
{
	/**
	 * Create some readings (Reading class) and push them into a vecor of readings
	 */
	
	vector<Reading> readings;

	// INTEGER REading
	int val = 301;
        DatapointValue value(val);

	// Add it to the vector
	readings.push_back(Reading("home", new Datapoint("power", value)));

	// FLOAT (NUMBER in OMF) Reading
	float fVal = 18.3;
        value = DatapointValue(fVal);

	// Add it to the vector
	readings.push_back(Reading("box", new Datapoint("temp", value)));

	// STRINT + INTERGER Reading
	string tmp = string("calculator");
        value = DatapointValue(tmp);
	Reading office("office", new Datapoint("msg", value));
	DatapointValue number(872);
	office.addDatapoint(new Datapoint("phone", number));

	// Add it to the vector
	readings.push_back(office);

	// Add new readings
	val = 876;
	value = DatapointValue(val);
	readings.push_back(Reading("home", new Datapoint("power", value)));

	fVal = 32.7;
        value = DatapointValue(fVal);
	readings.push_back(Reading("box", new Datapoint("temp", value)));

	// Instantiate an HTTPS handler for "Hostname : port"
	SimpleHttps sender("192.168.1.157:5460");

	// Instantiate the OMF Class with URL path, OMF_TYTPE_ID and producerToken
	OMF omfReadings = OMF(sender, "/ingress/messages", "8421", "omf_translator_8421");

	/**
	 * We can pass the auth token to the new version of PI server connector relay	
	 * OMF omfReadings = OMF(sender, "/ingress/messages", "9221",
	 *			 "uid=UUID&sig=BASE64_SIG");
	 */

	/**
	 * Send the readings vector to PI Server
	 * pass false if we want to send data types for each reading.
	 */

	cerr << "Sent readings data: " << omfReadings.sendToServer(readings) << endl;
	//cerr << "Sent readings data: " << omfReadings.sendToServer(readings, false) << endl;

	return 0;
}
