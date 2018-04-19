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

	// Vector of Readings	
	vector<Reading> readings;
	// Vector of Reading POINTERS
	vector<Reading* > ptrReadings;

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

	// STRING + INTERGER Reading
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

	/**
	 * Send Reading data
	 *
	 * -1- Send vector of Readings
	 * -2- Send ONE Reading
	 * -3- Semd TWO Readings
	 * -4- Send vector of Reading POINTERS
	 * -5- Send ONE Reading pointer
	 */

	/**
	 * -1- send vector of Readings
	 */
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

	/**
	 * -2- send ONE Reading
	 */
	tmp = string("printer");
        value = DatapointValue(tmp);
	// ONE reading
	Reading lab("lab", new Datapoint("device", value));

	OMF oneReading = OMF(sender, "/ingress/messages", "6422", "omf_translator_6422");
	cerr << "Sent one reading data: " << oneReading.sendToServer(lab, false) << endl;

	/**
	 * -3- send vector of Reading POINTERS
	 */
	// TWO readings
	val = 346;
	value = DatapointValue(val);
	// reading 1
	Reading sensor1("sensor_1", new Datapoint("measure_1", value));
	OMF anotherOneReading = OMF(sender, "/ingress/messages", "1122", "omf_translator_1122");

	cerr << "Sent one reading data: " << anotherOneReading.sendToServer(sensor1) << endl;
	val = 812;
	value = DatapointValue(val);
	// reading 2
	Reading sensor2("sensor_1", new Datapoint("measure_1", value));
	cerr << "Sent one reading data: " << anotherOneReading.sendToServer(sensor2) << endl;

	/**
	 * -4- send vector of Reading POINTERS
	 */
	val = 876;
	value = DatapointValue(val);
	// A Reading pointer
	Reading* ptrData_1 = new Reading("pointers", new Datapoint("the_int", value));
	value = DatapointValue(665.432);
        ptrData_1->addDatapoint(new Datapoint("the_float", value));
	// Add the Reading pointer to the vector
	ptrReadings.push_back(ptrData_1);

	val = 331;
	value = DatapointValue(val);
	// Another Reading pointer
	Reading* ptrData_2 = new Reading("angle", new Datapoint("the_value", value));
	// Add the Reading pointer to the vector
	ptrReadings.push_back(ptrData_2);

	// Instantiate the OMF Class with URL path, OMF_TYTPE_ID and producerToken
	OMF ptrDataReading = OMF(sender, "/ingress/messages", "3422", "omf_translator_3422");
	cerr << "Sent data of ptr readings: " << ptrDataReading.sendToServer(ptrReadings) << endl;

	/**
	 * -5- Send ONE Reading POINTER
	 */
	val = 17;
	value = DatapointValue(val);
	Reading* onePtrData = new Reading("onepointer", new Datapoint("int_level", value));

	// Instantiate the OMF Class with URL path, OMF_TYTPE_ID and producerToken
	OMF onePtrDataReading = OMF(sender, "/ingress/messages", "6007", "omf_translator_6007");
	cerr << "Sent data of one ptr reading: " << onePtrDataReading.sendToServer(onePtrData) << endl;

	// Cleanup
	delete ptrData_1;
	delete ptrData_2;
	delete onePtrData;

	return 0;
}
