/*
 * FogLAMP OSI Soft OMF interface to PI Server.
 *
 * Copyright (c) 2018 OSIsoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */
#include <mbed.h>
#include <iostream>
#include <datapoint.h>
#include <reading.h>
#include <omf.h>
#include <mbed_https.h>
#include <DS1820.h>

using namespace std;

DS1820 probe(A0);

void sample(OMF& omfReadings)
{
	probe.convertTemperature(true, DS1820::all_devices);         //Start temperature conversion, wait until ready
	float temp = probe.temperature();
        Reading reading("mbed", new Datapoint("Temperature", temp));
	cerr << "Sent: " << temp << " " <<  omfReadings.sendToServer(reading) << endl;
}

int main(void)
{
	DS1820::unassignedProbe(A0);

	MbedHttps sender(PI_SERVER);
	
	// Instantiate OMF Class with OMF_TYPE_ID and producerToken
	OMF omfReadings = OMF(sender, "8010", "omf_mbed_8010");
	while (1)
	{
		sample(omfReadings);
		wait(2);
	}

	return 0;
}

