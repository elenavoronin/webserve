#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"
#include "../include/utils.hpp"
#include "../include/Server.hpp"
#include <sstream>


	HttpResponse::HttpResponse() : _statusCode(200), _statusMessage("OK") {}
	HttpResponse::~HttpResponse() {}

	void HttpResponse::setStatus(int code, const std::string& message) {
		_statusCode = code;
		_statusMessage = message;
	}

	void HttpResponse::setHeader(const std::string& key, const std::string& value) {
		_headers[key] = value;
	}

	std::string HttpResponse::getHeader(const std::string& key) const {
		auto it = _headers.find(key);
		return (it != _headers.end()) ? it->second : "";
	}

	void HttpResponse::setBody(const std::string& content) {
		_body = content;
		setHeader("Content-Length", std::to_string(_body.size()));
	}

	/**
	 * Build the entire HTTP response as a single string.
	 *
	 * This method will create a string with the HTTP status line, headers, and body.
	 * The HTTP status line is built by concatenating the HTTP version, status code,
	 * and status message.
	 *
	 * The headers are built by iterating over the map of headers and concatenating
	 * each key-value pair into a string. The key is the header name and the value
	 * is the header value. The header name and value are separated by a colon and
	 * a space.
	 *
	 * The body is added to the end of the string and is separated from the headers
	 * by a double newline.
	 *
	 * @return A string containing the full HTTP response.
	 * @todo fix this
	 */

	std::string HttpResponse::buildResponse() const {
		std::ostringstream response;
		// Build the HTTP status line
		response << "HTTP/1.1 " << std::to_string(_statusCode) << " " << _statusMessage << "\r\n";
		// Map status codes to reasons
		response << getStatusMessage(_statusCode) << "\r\n";
		// Build the headers
		for (const auto& header : _headers) {
			// The header name and value are separated by a colon and a space
			response << header.first << ": " << header.second << "\r\n";
		}
		// End headers section
    	response << "\r\n";
		// Add body if present
   	 	if (!_body.empty()) {
        	response << _body;
   	 	}
		std::cout << response.str() << std::endl;
		return response.str();
	}

	void HttpResponse::redirect(const std::string& location, int status_code, const std::string& message) {
		setStatus(status_code, message);
		setHeader("Location", location);
		setBody(""); // Redirections typically have no body
}
