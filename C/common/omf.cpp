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
#include <string>
#include <typeinfo>
#include <omf.h>

using namespace std;

/**
 * OMFData constructor
 */
OMFData::OMFData(const Reading& reading)
{
	// Convert reading data into the OMF JSON string
	m_value.append("{\"containerid\": \"measurement_");
	m_value.append(reading.getAssetName() + "\", \"values\": [{");

	// Get reading data
	const vector<Datapoint*> data = reading.getReadingData();

	/**
	 * This loop creates:
	 * "dataName": {"type": "dataType"},
	 */
	for (auto it = data.cbegin(); it != data.cend(); ++it)
	{
		// Add datapoint Name
		m_value.append("\"" + (*it)->getName() + "\": " + (*it)->getData().toString());
		m_value.append(", ");
	}

	// Append Z to getAssetDateTime(FMT_STANDARD)
	m_value.append("\"Time\": \"" + reading.getAssetDateTime(Reading::FMT_STANDARD) + "Z" + "\"");

	m_value.append("}]}");
}

/**
 * Return the JSOIN data in m_value
 */
const string OMFData::OMFdataVal() const
{
	return m_value;
}

/**
 * OMF constructor
 */
OMF::OMF(const string& url,
	 const string& id,
	 const string& token) :
	 m_serverURL(url),
	 m_tokenId(id),
	 m_producerToken(token)
{
	string schema("http");
	string host_port;
	size_t host_port_start = m_serverURL.find("https://");
	if (host_port_start != std::string::npos)
	{
		schema = "https";
	}

	string fullSchema = (schema + "://");
	string server = m_serverURL.substr(fullSchema.length());

	size_t path_start = server.find("/");
	if (path_start != std::string::npos)
	{
		host_port = server.substr(0, path_start);
	}
	else
	{	
		host_port = server;
	}

	m_sender = new HttpSender(schema, host_port);
}

// Destructor
OMF::~OMF()
{
	delete m_sender;
}

/**
 * Creates and send data type messages for a Reading data
 */
int OMF::handleTypes(const Reading& row) const
{
	int res;

	// Create header for Type
	vector<pair<string, string>> resType = OMF::createMessageHeader("Type");
	// Create data for Type message	
	string typeData = OMF::createTypeData(row);

	// Build an HTTPS POST with 'resType' headers
	// and 'typeData' JSON payload
	// Then get HTTPS POST ret code and return 0 to client on error
	// TODO: save typeData and send it once
	res = m_sender->sendRequest("POST", "/ingress/messages", resType, typeData);
	if (res != 200 && res != 204)
	{
		return 0;
	}

	// Create header for Container
	vector<pair<string, string>> resContainer = OMF::createMessageHeader("Container");
	// Create data for Container message	
	string typeContainer = OMF::createContainerData(row);

	// Build an HTTPS POST with 'resContainer' headers
	// and 'typeContainer' JSON payload
	// Then get HTTPS POST ret code and return 0 to client on error
	// TODO: save typeContainer and send it once
	res = m_sender->sendRequest("POST", "/ingress/messages", resContainer, typeContainer);
	if (res != 200 && res != 204)
	{
		return 0;
	}

	// Create header for Static data
	vector<pair<string, string>> resStaticData = OMF::createMessageHeader("Data");
	// Create data for Static Data message	
	string typeStaticData = OMF::createStaticData(row);

	// Build an HTTPS POST with 'resStaticData' headers
	// and 'typeStaticData' JSON payload
	// Then get HTTPS POST ret code and return 0 to client on error
	// TODO: save typeStaticData and send it once
	res = m_sender->sendRequest("POST", "/ingress/messages", resStaticData, typeStaticData);
	if (res != 200 && res != 204)
	{
		return 0;
	}

	// Create header for Link data
	vector<pair<string, string>> resLinkData = OMF::createMessageHeader("Data");
	// Create data for Static Data message	
	string typeLinkData = OMF::createLinkData(row);

	// Build an HTTPS POST with 'resLinkData' headers
	// and 'typeLinkData' JSON payload
	// Then get HTTPS POST ret code and return 0 to client on error
	// TODO: save typeLinkData and send it once
	res = m_sender->sendRequest("POST", "/ingress/messages", resLinkData, typeLinkData);

	if (res != 200 && res != 204)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
 * Send all the readings to the PI Server
 *
 * @param readings    A vector of readings data pointers
 * @return            != on success, 0 otherwise
 */
uint32_t OMF::sendToServer(const vector<Reading *> readings)
{
	/*
	 * Iterate over readings:
	 * - Send/cache Types
	 * - transform a reading to OMF format
	 * - add OMND data to new vector
	 */

	ostringstream jsonData;
	jsonData << "[";

	// Fecth Reading* data
	for (vector<Reading *>::const_iterator elem = readings.cbegin();
						    elem != readings.cend();
						    ++elem)
	{
		// The current reading
		if (!OMF::handleTypes(**elem))
		{
			return 0;
		}

		// Add into JSON string the OMF transformed Reading data
		jsonData << OMFData(**elem).OMFdataVal() << (elem < (readings.cend() -1 ) ? ", " : "");
	}

	jsonData << "]";

	/**
	 * Types messages sent, now transorm ech reading to OMF format.
	 *
	 * After formatting the new vector of data can be sent
	 * with one message only
	 */

	// Create header for Readings data
	vector<pair<string, string>> readingData = OMF::createMessageHeader("Data");

	// Build an HTTPS POST with 'redingData headers
	// and 'allReadings' JSON payload
	// Then get HTTPS POST ret code and return 0 to client on error
	int res = m_sender->sendRequest("POST", "/ingress/messages", readingData, jsonData.str());
	if (res != 200 && res != 204)
	{
		return 0;
	}

	// Return number of sen t readings to the caller
	return readings.size();
}

/**
 * Send all the readings to the PI Server
 *
 * @param readings    A vector of readings data
 * @return            != on success, 0 otherwise
 */
uint32_t OMF::sendToServer(const vector<Reading>& readings)
{
	/*
	 * Iterate over readings:
	 * - Send/cache Types
	 * - transform a reading to OMF format
	 * - add OMND data to new vector
	 */
	ostringstream jsonData;
	jsonData << "[";

	// Fecth Reading data
	for (vector<Reading>::const_iterator elem = readings.cbegin();
						    elem != readings.cend();
						    ++elem)
	{
		// Handle the current reading
		if (!OMF::handleTypes(*elem))
		{
			return 0;
		}

		// Add into JSON string the OMF transformed Reading data
		jsonData << OMFData(*elem).OMFdataVal() << (elem < (readings.cend() -1 ) ? ", " : "");
	}

	jsonData << "]";

	// Build headers for Readings data
	vector<pair<string, string>> readingData = OMF::createMessageHeader("Data");

	// Build an HTTPS POST with 'readingData headers and 'allReadings' JSON payload
	// Then get HTTPS POST ret code and return 0 to client on error
	int res = m_sender->sendRequest("POST", "/ingress/messages", readingData, jsonData.str());

	if (res != 200 && res != 204)
	{
		return 0;
	}

	// Return number of sen t readings to the caller
	return readings.size();
}

/**
 * Creates a vector of HTTP header to be sent to Server
 *
 * @param type    The message type ('Type', 'Container', 'Data')
 * @return        A vector of HTTP Header string pairs
 */
const vector<pair<string, string>> OMF::createMessageHeader(const std::string& type) const
{
	vector<pair<string, string>> res;

	res.push_back(pair<string, string>("messagetype", type));
	res.push_back(pair<string, string>("producertoken", m_producerToken));
	res.push_back(pair<string, string>("omfversion", "1.0"));
	res.push_back(pair<string, string>("messageformat", "JSON"));
	res.push_back(pair<string, string>("action", "create"));

	return  res; 
}

/**
 * Creates the Type message for data type definition
 *
 * @param reading    A reading data
 * @return           Type JSON message as string
 */
const std::string OMF::createTypeData(const Reading& reading) const
{
	// Build the Type data message (JSON Array)

	// Add the Static data part

	string tData("[{ \"type\": \"object\", \"properties\": { "
"\"Company\": {\"type\": \"string\"}, \"Location\": {\"type\": \"string\"}, "
"\"Name\": { \"type\": \"string\", \"isindex\": true } }, "
"\"classification\": \"static\", \"id\": \"");

	// Add type_id + '_' + asset_name + '_typename_sensor'
	OMF::setAssetTypeTag(reading.getAssetName(),
			     "typename_sensor",
			     tData);

	tData.append("\" }, { \"type\": \"object\", \"properties\": {");

	// Add the Dynamic data part

	/* We add for ech reading
	 * the DataPoint name & type
	 * type is 'integer' for INT
	 * 'number' for FLOAT
	 * 'string' for STRING
	 */

	const vector<Datapoint*> data = reading.getReadingData();

	/**
	 * This loop creates:
	 * "dataName": {"type": "dataType"},
	 */
	for (auto it = data.cbegin(); it != data.cend(); ++it)
	{
		// Add datapoint Name
		tData.append("\"" + (*it)->getName() + "\"");
		tData.append(": {\"type\": \"");
		// Add datapoint Type
		tData.append(omfTypes[((*it)->getData()).getType()]);
		tData.append("\"}, ");
	}

	// Add time field
	tData.append("\"Time\": {\"type\": \"string\", \"isindex\": true, \"format\": \"date-time\"}}, "
"\"classification\": \"dynamic\", \"id\": \"");

	// Add type_id + '_' + asset_name + '__typename_measurement'
	OMF::setAssetTypeTag(reading.getAssetName(),
			     "typename_measurement",
			     tData);

	 tData.append("\" }]");

	// Return JSON string
	return tData;
}

/**
 * Creates the Container message for data type definition
 *
 * @param reading    A reading data
 * @return           Type JSON message as string
 */
const std::string OMF::createContainerData(const Reading& reading) const
{
	// Build the Container data (JSON Array)
	string cData = "[{\"typeid\": \"";

	// Add type_id + '_' + asset_name + '__typename_measurement'
	OMF::setAssetTypeTag(reading.getAssetName(),
			     "typename_measurement",
			     cData);

	cData.append("\", \"id\": \"measurement_");
	cData.append(reading.getAssetName());
	cData.append("\"}]");

	// Return JSON string
	return cData;
}

/**
 * Creates the Static Data message for data type definition
 *
 * Note: type is 'Data'
 *
 * @param reading    A reading data
 * @return           Type JSON message as string
 */
const std::string OMF::createStaticData(const Reading& reading) const
{
	// Build the Static data (JSON Array)
	string sData = "[{\"typeid\": \"";

	// Add type_id + '_' + asset_name + '_typename_sensor'
	OMF::setAssetTypeTag(reading.getAssetName(),
			     "typename_sensor",
			     sData);

	sData.append("\", \"values\": [{\"Location\": \"Palo Alto\", "
"\"Company\": \"Dianomic\", \"Name\": \"");

	// Add asset_name
	sData.append(reading.getAssetName());
	sData.append("\"}]}]");

	// Return JSON string
	return sData;
}

/**
 * Creates the Link Data message for data type definition
 *
 * Note: type is 'Data'
 *
 * @param reading    A reading data
 * @return           Type JSON message as string
 */
const std::string OMF::createLinkData(const Reading& reading) const
{
	// Build the Link data (JSON Array)

	string lData = "[{\"typeid\": \"__Link\", \"values\": "
"[{\"source\": {\"typeid\": \"";

	// Add type_id + '_' + asset_name + '__typename_sensor'
	OMF::setAssetTypeTag(reading.getAssetName(),
			     "typename_sensor",
			     lData);

	lData.append("\", \"index\": \"_ROOT\"}, \"target\": {\"typeid\": \"");

	// Add type_id + '_' + asset_name + '__typename_sensor'
	OMF::setAssetTypeTag(reading.getAssetName(),
			     "typename_sensor",
			     lData);

	lData.append("\", \"index\": \"");

	// Add asset_name
	lData.append(reading.getAssetName());

	lData.append("\"}}, {\"source\": {\"typeid\": \"");

	// Add type_id + '_' + asset_name + '__typename_sensor'
	OMF::setAssetTypeTag(reading.getAssetName(),
			     "typename_sensor",
			     lData);

	lData.append("\", \"index\": \"");

	// Add asset_name
	lData.append(reading.getAssetName());

	lData.append("\"}, \"target\": {\"containerid\": \"measurement_");

	// Add asset_name
	lData.append(reading.getAssetName());

	lData.append("\"}}]}]");

	// Return JSON string
	return lData;
}

/**
 * Set the tag ID_XYZ_typename_sensor|typename_measurement
 *
 * @param assetName    The assetName
 * @param tagName    The tagName to append
 * @param data         The string to append result tag
 */
void OMF::setAssetTypeTag(const string& assetName,
			  const string& tagName,
			  string& data) const
{
	// Add type_id + '_' + asset_name + '_' + tagName'
	data.append(m_tokenId + "_" + assetName +  "_" + tagName);
}
