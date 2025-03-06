#pragma once

#include <string>
#include <map>
#include "HttpRequest.hpp"
#include "utils.hpp"
#include "Server.hpp"
#include <sstream>

/**
 * @brief The HttpResponse class handles the construction and storage of HTTP responses.
 *
 * This class allows setting status codes, headers, and the body of an HTTP response,
 * as well as building a complete response string to be sent to the client.
 */
class HttpResponse {

	private:
		std::string									_fullResponse;				// The complete HTTP response string
		int 										_statusCode;				// HTTP status code (e.g., 200, 404)
		std::string 								_statusMessage;				// Corresponding status message (e.g., "OK", "Not Found")
		std::map<std::string, std::string> 			_headers;					// HTTP headers (key-value pairs)
		std::string 								_body;
		std::string 								_headersOnly;				// HTTP headers without body

	public:
		HttpResponse();
		~HttpResponse();

		void 				setStatus(int code, const std::string& message);
		void 				setHeader(const std::string& key, const std::string& value);
		void 				setBody(const std::string& content);
		void				setFullResponse(const std::string& response);
		std::string 		getHeader(const std::string& key) const;
		std::string 		getStatusMessage() const;
		std::string 		&getFullResponse();
		void 				buildResponse();
		void 				redirect(const std::string& location, int status_code, const std::string& message);
		std::string 		getBody() { return this->_body; };
};
