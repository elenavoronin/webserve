#!/usr/bin/env python3

import cgi
# import time
import os
# import sys

# check for opening file in correct dir
# file = open("hi","x")

# Parse form data
form = cgi.FieldStorage()
name = form.getvalue('name', 'Guest')

# evoke timeout
# while True:
# 	time.sleep(3)  # Simulate a delay to evoke time-out

# Generate response
body = f"""
<html>
<head>
    <title>Greetings!</title>
    <style>
        body {{ background-color: grey; color: white; font-family: Arial, sans-serif; }}
        button {{ padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }}
        button:hover {{ background-color: purple; }}
    </style>
</head>
<body>
    <h1>Welcome, {name}!</h1>
    <p>Nice to meet you.</p>
    <p>
        <a href="/index.html" style="text-decoration: none;">
            <button>Return to Homepage</button>
        </a>
    </p>
</body>
</html>
"""

# Calculate Content-Length
content_length = len(body.encode('utf-8'))

# Send HTTP headers
print("HTTP/1.1 200 OK\r")
print("Content-Type: text/html\r")
print(f"Content-Length: {content_length}\r")
print("\r")  # Blank line to separate headers from body

# Send response body
print(body)
