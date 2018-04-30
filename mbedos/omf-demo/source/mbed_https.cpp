/*
 * FogLAMP HTTP Sender implementation using the
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
#if 0
"-----BEGIN CERTIFICATE-----\n"
"MIIFcDCCA1igAwIBAgIQAIKrl9JhxhPG6FFvcBSj3zANBgkqhkiG9w0BAQsFADAx\n"
"MRgwFgYDVQQDDA9XSU4tNE03T0RLQjBSSDIxFTATBgNVBAoMDE9TSXNvZnQsIExM\n"
"QzAgFw0xNzA2MjExMjI1NDRaGA85OTk5MTIzMTIzNTk1OVowMTEYMBYGA1UEAwwP\n"
"V0lOLTRNN09ES0IwUkgyMRUwEwYDVQQKDAxPU0lzb2Z0LCBMTEMwggIiMA0GCSqG\n"
"SIb3DQEBAQUAA4ICDwAwggIKAoICAQCGWQrsScVDWq+rQnVjH4e4AARQPJAF0m3Y\n"
"mDLQVNeMzyYJ9b0A8miDubtFSAgv9oURwHr6WwVI/Kkz1Z8WYgAm79EubA84HVW3\n"
"6d3NjLEt0mFBe1M7D2gOkd6xJDJNTVPJv/b0pnN3Mm2hF6mzlVygqabqRMMXduoy\n"
"e8PVQgXP5W/UnIOGDPI8CfcBLXhbMQaKxfl0s9coEhme7sThJPKIfzE5TAMCIcP7\n"
"NUR6NhAuEO7KK40nenNFhVKeaGMa9hx0dkjdfwsvdT81q8j83MAaOrKGgAU0cOtK\n"
"f6QED3KUnY+/Fe+ZPUQsqlLlbOw8ucf6di8dtcpBuNrcDc/K8qZvzbrIr7VjghDg\n"
"REfrH3zoCByb2PY3cQX7G/FjLulJO/U4vk0/Y8rGKCrIzkcYlDpZikLFfLvZYUNq\n"
"CUwK7fzTbtWcupCWB5FenmYXAgHTIfEOzRfu3cGqslnHPP16tPO++GRTI4JEeY9r\n"
"adyvJ5wt+uEdyv5zleE6WeOGuLAiGlEFtbA96ix7mMBVWQGRA+ATRITgC1XmHAcj\n"
"QbmR5qckJWAqcnn4T/DxMuvb6Lz/f/mXbYZIBV8tGNYfoU+PYhwbCcrvCiXJRMg4\n"
"hLIovpzWDwc5FYVgE3gVhhg/XeCOfOQrCvRiuka2nUO+hhDwQc4GJTG4W0IdZSxT\n"
"bwjTj7vGIQIDAQABo4GBMH8waAYDVR0jBGEwX4AUs/PlTu1iiC9KBOwH4wlpAD6w\n"
"hEKhNaQzMDExGDAWBgNVBAMMD1dJTi00TTdPREtCMFJIMjEVMBMGA1UECgwMT1NJ\n"
"c29mdCwgTExDghAAgquX0mHGE8boUW9wFKPfMBMGA1UdJQQMMAoGCCsGAQUFBwMB\n"
"MA0GCSqGSIb3DQEBCwUAA4ICAQB49LrV+oYIQxinn1a4f7nUH2HK9Vy3ekM/0KX1\n"
"BCU7YTUjqHHBaIrz8eeZVXxz/CglP3do8Sjx/vOq9gm48S65Z5Csp6+lbYqM7M1i\n"
"qIgE3ikkIjj+Aub1jF+GzvGNCAQwtW4M/jgW8fzWFfNAzma0dsiWK3gS7+s4hxhA\n"
"B8waoU0wOa9XO9I6U3ItpVUEtTHNg8OtlB356fiuIctRoALxfWU+n2B+bBbXntry\n"
"g2Kgts43oP7Hj3JDaiczVYTMxgwYvmo6g2Pz1+zWfhtvqqdUewg0l716P3DvRgfk\n"
"KFESw8zYco23vii0l6zSOn4vnb1kTVGbP4RscfdcwgXVVNvk9hh7x/I0jT1GMd6Q\n"
"92l+xembBoWlFGkt/vggyN/lrVRWLeEoqRzMDX6KIfmYlAaf3w61JUcGBZ0Ge4Rp\n"
"CBVQYWjsD96SfyPi4BAbD1Eav/9iVB7SW6v8ssD+4gNvpXkE43ul0D6AR+45f2z1\n"
"t7PLajIrom+4bExUAn9xyeRqlDyR7JHqN8TFziF/gYFyJksv1TIDrQkpkycNNeP7\n"
"hAjuU9JYj+ZspAgUXxAqCjmEHTuzfzghiKpRCE51RcX1Ey/G1YBRyfNARcr3v292\n"
"/eViiunVnM5DMt2EBrW2n81bLfY/KYVG9RoMO+6h9SA0wZFepBSLTFyr/0QZP8jE\n"
"1iP0dA==\n"
"-----END CERTIFICATE-----\n";
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
    "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
    "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
    "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
    "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
    "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
    "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
    "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
    "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
    "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
    "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
    "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
    "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
    "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
    "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
    "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
    "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
    "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
    "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
    "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
    "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
    "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
    "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
    "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
    "-----END CERTIFICATE-----\n";
#endif
/**
 * Constructor
 */
MbedHttps::MbedHttps(const string& host_port) : HttpSender(), m_host_port(host_port)
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
		printf("Failed to get time %i\n", timestamp);
}

/**
 * Destructor
 */
MbedHttps::~MbedHttps()
{
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

	if (!m_network)
	{
		printf("No network\n");
		return 400;
	}

	string url("https://");
	url += m_host_port;
	url += path;
	if (method.compare("GET") == 0)
		req = new HttpsRequest(m_network, SSL_CA_PEM, HTTP_GET, url.c_str());
	else if (method.compare("POST") == 0)
		req = new HttpsRequest(m_network, SSL_CA_PEM, HTTP_POST, url.c_str());

	// Add FogLAMP UserAgent
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
		return err;
	}
#if TRACE_MESSAGES
	cerr << url << " " << res->get_status_code() << endl;
	cerr << payload << endl;
#endif

	delete req;

	int code = res->get_status_code();
	return code;
}
