#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"
#include "../include/utils.hpp"
#include "../include/Server.hpp"
#include <sstream>

/**
 * @brief       Default constructor for the HttpResponse class.
 * 
 * @details     Initializes the HttpResponse object with a default status code of 200
 *              and a default status message of "OK".
 */
HttpResponse::HttpResponse() : _statusCode(200), _statusMessage("OK") {}

/**
 * @brief       Destructor for the HttpResponse class.
 * 
 * @details     Ensures proper cleanup of resources used by the HttpResponse object.
 *              Currently, no specific cleanup is needed.
 */
HttpResponse::~HttpResponse() {}

/**
 * @brief       Sets the HTTP status code and corresponding status message.
 * 
 * @param       code        The HTTP status code (e.g., 200, 404).
 * @param       message     The HTTP status message (e.g., "OK", "Not Found").
 */
void HttpResponse::setStatus(int code, const std::string& message) {
	_statusCode = code;
	_statusMessage = message;
}

/**
 * @brief       Adds or updates a header in the HTTP response.
 * 
 * @param       key         The header name (e.g., "Content-Type").
 * @param       value       The header value (e.g., "text/html").
 */
void HttpResponse::setHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

/**
 * @brief       Retrieves the value of a specific header.
 * 
 * @param       key         The name of the header to retrieve.
 * 
 * @return      The value of the header if it exists, or an empty string if it does not.
 */
std::string HttpResponse::getHeader(const std::string& key) const {
	auto it = _headers.find(key);
	return (it != _headers.end()) ? it->second : "";
}

/**
 * @brief       Retrieves the current HTTP status message.
 * 
 * @return      A string representing the HTTP status message (e.g., "OK").
 */
std::string HttpResponse::getStatusMessage() const {
	return _statusMessage;
}

/**
 * @brief       Retrieves the current HTTP status code.
 * 
 * @return      An integer representing the HTTP status code (e.g., 200).
 */
int HttpResponse::getStatusCode() const {
	return _statusCode;
}

/**
 * @brief       Sets the body content of the HTTP response.
 * 
 * @param       content     The content to be set as the response body.
 * 
 * @details     Updates the "Content-Length" header to reflect the size of the body.
 */
void HttpResponse::setBody(const std::string& content) {
	_body = content;
	setHeader("Content-Length", std::to_string(_body.size()));
}

/**
 * @brief       Builds the complete HTTP response as a string.
 * 				This method will create a string with the HTTP status line, headers, and body.
 * 				The HTTP status line is built by concatenating the HTTP version, status code,
 * 				and status message.
 *
 * 				The headers are built by iterating over the map of headers and concatenating
 * 				each key-value pair into a string. The key is the header name and the value
 * 				is the header value. The header name and value are separated by a colon and
 * 				a space.
 *
 * 				The body is added to the end of the string and is separated from the headers
 * 				by a double newline
 * 
 * @details     Constructs the HTTP response by combining the status line, headers, 
 *              and body. The headers and body are separated by a blank line.
 * 
 * @return      A string representing the entire HTTP response.
 * 
 * @todo        - Fix redundant status code addition in response.
 *              - Handle edge cases for invalid headers or body content.
 */
std::string HttpResponse::buildResponse() const {
	std::string response;
	// Build the HTTP status line
	response += "HTTP/1.1 " + std::to_string(_statusCode) + " " + _statusMessage + "\r\n";
	// Map status codes to reasons
	response += std::to_string(_statusCode) += "\r\n";
	// Build the headers
	for (const auto& header : _headers) {
		// The header name and value are separated by a colon and a space
		response += header.first + ": " + header.second + "\r\n";
	}
	// End headers section
	response += "\r\n";
	// Add body if present
	if (!_body.empty()) {
		response += _body;
	}
	std::cout << response << std::endl;
	return response;
}

/**
 * @brief       Redirects the client to a specified location.
 * 
 * @param       location    The URL to redirect to.
 * @param       status_code The HTTP status code for the redirect (e.g., 301, 302).
 * @param       message     The HTTP status message for the redirect (e.g., "Moved Permanently").
 * 
 * @details     Sets the "Location" header to the specified URL and clears the response body.
 *              Typically used for HTTP redirections.
 */
void HttpResponse::redirect(const std::string& location, int status_code, const std::string& message) {
	setStatus(status_code, message);
	setHeader("Location", location);
	setBody(""); // Redirections typically have no body
}
