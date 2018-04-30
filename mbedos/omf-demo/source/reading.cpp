/*
 * FogLAMP storage service client
 *
 * Copyright (c) 2018 OSIsoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch, Massimiliano Pinto
 */
#include <reading.h>
#include <ctime>
#include <string>
#include <sstream>
#include <iostream>
#ifndef NO_READ_KEYS
#include <uuid/uuid.h>
#endif
#include <stdio.h>
#include <sys/time.h>

using namespace std;
static string dateTypes[] = { DEFAULT_DATE_TIME_FORMAT,
	COMBINED_DATE_STANDARD_FORMAT };

/**
 * Reading constructor
 *
 * A reading is a container for the values related to a single asset.
 * Each actual datavalue that relates to that asset is held within an
 * instance of a Datapoint class.
 */
Reading::Reading(const string& asset, Datapoint *value) : m_asset(asset)
{
#ifndef NO_READ_KEYS
uuid_t	uuid;
char	uuid_str[37];
#endif

	m_values.push_back(value);
#ifndef NO_READ_KEYS
	uuid_generate_time_safe(uuid);
	uuid_unparse_lower(uuid, uuid_str);
	m_uuid = string(uuid_str);
#else
	m_uuid = string("");
#endif
	// Store seconds and microseconds
	gettimeofday(&m_timestamp, NULL);
	m_timestamp.tv_sec = time(0);
}

/**
 * Reading copy constructor
 */
Reading::Reading(const Reading& orig) : m_asset(orig.m_asset),
	m_timestamp(orig.m_timestamp), m_uuid(orig.m_uuid)
{
	for (DatapointConstIterator it = orig.m_values.begin(); it != orig.m_values.end(); it++)
	{
		m_values.push_back(new Datapoint(**it));
	}
}

/**
 * Destructor for Reading class
 */
Reading::~Reading()
{
	for (DatapointIterator it = m_values.begin(); it != m_values.end(); it++)
	{
		delete(*it);
	}
}

/**
 * Add another data point to an asset reading
 */
void Reading::addDatapoint(Datapoint *value)
{
	m_values.push_back(value);
}

/**
 * Return the asset reading as a JSON structure encoded in a
 * C++ string.
 */
string Reading::toJSON()
{
ostringstream convert;

	convert << "{ \"asset_code\" : \"";
	convert << m_asset;
	convert << "\", \"read_key\" : \"";
	convert << m_uuid;
	convert << "\", \"user_ts\" : \"";

	// Add date_time with microseconds + timezone UTC:
	// YYYY-MM-DD HH24:MM:SS.MS+00:00
	convert << Reading::getAssetDateTime(FMT_DEFAULT) << "+00:00";

	// Add values
	convert << "\", \"reading\" : { ";
	for (DatapointIterator it = m_values.begin(); it != m_values.end(); it++)
	{
		if (it != m_values.begin())
		{
			convert << ", ";
		}
		convert << (*it)->toJSONProperty();
	}
	convert << " } }";

	return convert.str();
}

/**
 * Return a formatted m_timestamp DataTime
 * @param dateFormat    Format: FMT_DEFAULT or FMT_STANDARD
 * @return              The formatted datetime string
 */
const string Reading::getAssetDateTime(readingTimeFormat dateFormat) const
{
char date_time[DATE_TIME_BUFFER_LEN];
char micro_s[10];
ostringstream assetTime;

        // Populate tm structure
        const struct tm *timeinfo = std::localtime(&(m_timestamp.tv_sec));

        /**
         * Build date_time with format YYYY-MM-DD HH24:MM:SS.MS+00:00
         * this is same as Python3:
         * datetime.datetime.now(tz=datetime.timezone.utc)
         */

        // Create datetime with seconds
        std::strftime(date_time, sizeof(date_time),
		      dateTypes[dateFormat].c_str(),
                      timeinfo);

        // Add microseconds 
        snprintf(micro_s,
                 sizeof(micro_s),
                 ".%06lu",
                 m_timestamp.tv_usec);
	micro_s[7] = 0;

        // Add date_time + microseconds
        assetTime << date_time << micro_s;

	return assetTime.str();
}

// Return the asset name
const string& Reading::getAssetName() const
{
	return m_asset;
}

// Return the reading Datapoint vector
const vector<Datapoint *>& Reading::getReadingData() const
{
	return m_values;
}
