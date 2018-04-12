#ifndef _OMF_H
#define _OMF_H
/*
 * FogLAMP OSI Soft OMF interface to PI Server.
 *
 * Copyright (c) 2018 OSisoft, LLC
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */
#include <string>
#include <vector>
#include <reading.h>
#include <http_sender.h>

#define OMF_TYPE_STRING  "string"
#define OMF_TYPE_INTEGER "integer"
#define OMF_TYPE_FLOAT   "number"

/**
 * The OMF class.
 */
class OMF
{
        public:
		/**
		 * Constructor:
		 * pass server URL, OMF_type_id and producerToken.
		 */
		OMF(HttpSender& sender,
                    const std::string& url,
		    const std::string& typeId,
		    const std::string& producerToken);

		/**
		 * Send data to PI Server passing a vector of readings.
		 *
		 * Data sending is composed by a few phases
		 * handled by private methods:
		 *
		 * Returns the number of processed readings.
		 */

		// Method with vector of readings
		uint32_t sendToServer(const std::vector<Reading>& readings);

		// Method with vector of reading pointers
		uint32_t sendToServer(const std::vector<Reading *> readings);

		// Destructor
		~OMF();

	private:
		/**
		 * Builds the HTTP header to send
		 * messagetype header takes the passed type value:
		 * 'Type', 'Container', 'Data'
		 */
		const std::vector<std::pair<std::string, std::string>> createMessageHeader(const std::string& type) const;

		// Create data for Type message for current row
		const std::string createTypeData(const Reading& reading) const;

		// Create data for Container message for current row
		const std::string createContainerData(const Reading& reading) const;

		// Create data for additional type message, with 'Data' for current row
		const std::string createStaticData(const Reading& reading) const;

		// Create data Link message, with 'Data', for current row
		const std::string createLinkData(const Reading& reading) const;

		/**
		 * Creata data for readings data content, with 'Data', for one row
		 * The new formatted data have to be added to a new JSON doc to send.
		 * we want to avoid sending of one data row
		 */
		const std::string createMessageData(Reading& reading);

		// Set the the tagName in an assetName Type message
		void setAssetTypeTag(const std::string& assetName,
				     const std::string& tagName,
				     std::string& data) const;
		// Send or caches OMF data types
		int handleTypes(const Reading& row) const;

        private:
		const std::string	m_serverURL;
		const std::string	m_tokenId;
		const std::string	m_producerToken;
		// Vector with OMF_TYPES
		const std::vector<std::string> omfTypes = { OMF_TYPE_STRING,
							    OMF_TYPE_INTEGER,
							    OMF_TYPE_FLOAT };
		// HTTP Sender interface
		HttpSender&		m_sender;
};

/**
 * The OMFData class.
 * A reading is formatted with OMF specifications
 */
class OMFData
{
	public:
		OMFData(const Reading& reading);
		const std::string OMFdataVal() const;
	private:
		std::string	m_value;
};

#endif
