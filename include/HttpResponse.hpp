#pragma once

#include <string>
#include <map>

class HttpResponse {
	
	private:
		int _statusCode;
		std::string _statusMessage;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:

		HttpResponse();
		~HttpResponse();

		void setStatus(int code, const std::string& message);
		void setHeader(const std::string& key, const std::string& value);
		std::string getHeader(const std::string& key) const;
		void setBody(const std::string& content);
		std::string buildResponse() const;
};