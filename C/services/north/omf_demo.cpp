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

	SimpleHttps sender("192.168.1.157:5460");

	// Instantiate OMF Class with URL, OMF_TYTPE_ID and producerToken
	OMF omfReadings = OMF(sender, "https://192.168.1.157:5460/ingress/messages", "8008", "omf_translator_8008");

	/**
	 * Send the readings vector to PI Server
	 */

	cerr << "Sent readings data: " << omfReadings.sendToServer(readings) << endl;

	return 0;
}
