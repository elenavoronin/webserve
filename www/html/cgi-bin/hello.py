# !/usr/bin/env python3

import cgi
import os
import time

# The header for an HTTP response (Content-type is required)
print("Content-type: text/html")
print()  # Blank line to indicate the end of the headers

# Start the HTML output
print("<html>")
print("<head>")
print("<title>Simple Python CGI</title>")
print('<style>')
print('body { background-color: black; color: white; font-family: Arial, sans-serif; }')
print('button { padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }')
print('button:hover { background-color: gray; }')
print('</style>')
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
# print("<h2>Environment Variables</h2>")
# print("<pre>")
# for key, value in os.environ.items():
#     print(f"{key}: {value}")
# print("</pre>")


# test if we use poll loop for cgi, should not get stuck
# vcreate cg request then try to open another localhost
# if stuck not good
# while True:
# 	time.sleep(5)


# Add a "Return to Homepage" button
print("<p>")
print('<a href="/index.html" style="text-decoration: none;">') # Replace with the actual URL
print('<button style="padding: 10px 20px; font-size: 16px;">Return to Homepage</button>')
print('</a>')
print("</p>")

# End the HTML output
print("</body>")
print("</html>")
