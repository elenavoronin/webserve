#!/usr/bin/env python3

import cgi
import os

# Generate the HTML body
body = """<html>
<head>
<title>Simple Python CGI</title>
<style>
body { background-color: black; color: white; font-family: Arial, sans-serif; }
button { padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }
button:hover { background-color: gray; }
</style>
</head>
<body>
"""

# Get the query string data (for GET requests)
form = cgi.FieldStorage()
name = form.getvalue('name', 'Guest')
age = form.getvalue('age', 'unknown')

# Add dynamic content
body += f"<h1>Hello, {name}!</h1>"
body += f"<p>Your age is {age}.</p>"
body += """
<p>
<a href="/index.html" style="text-decoration: none;">
<button style="padding: 10px 20px; font-size: 16px;">Return to Homepage</button>
</a>
</p>
</body>
</html>
"""

# Calculate Content-Length
content_length = len(body.encode('utf-8'))

# Send the HTTP headers
print(f"Content-Type: text/html")
print(f"Content-Length: {content_length}")
print()  # Blank line to indicate the end of the headers

# Send the HTML body
print(body)
