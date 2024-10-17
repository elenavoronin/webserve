#!/usr/bin/env python3

import cgi
import os

#this should not be here
# cgi_response = "<html><body><h1>CGI Script Response</h1><p>This is output from your CGI script.</p></body></html>"

#this should not be here
http_response = (
    f"HTTP/1.1 200 OK\r\n"
    f"Content-Type: text/html\r\n"
    f"Content-Length: {1000000}\r\n" # need to parse the response at len of content
    f"Connection: close\r\n"
    f"\r\n" # response builder should give make the new line
    # f"{cgi_response}" I should give the respons body while reading in the parent
)

#this should not be here
print(http_response)

# The header for an HTTP response (Content-type is required)
print("Content-type: text/html")
print()  # Blank line to indicate the end of the headers

# Start the HTML output
print("<html>")
print("<head>")
print("<title>Simple Python CGI</title>")
print("</head>")
print("<body>")

# Get the query string data (for GET requests)
form = cgi.FieldStorage()

# Access specific parameters (if passed via URL, e.g., ?name=John&age=30)
name = form.getvalue('name', 'Guest')  # 'Guest' is the default value if 'name' is not provided
age = form.getvalue('age', 'unknown')  # 'unknown' is the default value if 'age' is not provided

# Print a greeting using the query string data
print(f"<h1>Hello, {name}!</h1>")
print(f"<p>Your age is {age}.</p>")

# Display the environment variables (like REQUEST_METHOD, QUERY_STRING, etc.)
print("<h2>Environment Variables</h2>")
print("<pre>")
for key, value in os.environ.items():
    print(f"{key}: {value}")
print("</pre>")

# End the HTML output
print("</body>")
print("</html>")
