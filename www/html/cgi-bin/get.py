#!/usr/bin/env python3
import os
import cgitb

# Enable debugging
cgitb.enable()

# Ensure the request method is GET
if os.environ.get("REQUEST_METHOD", "GET") != "GET":
    print("Content-Type: text/plain")
    print()
    print("Error: This script only accepts GET requests.")
    exit(1)

# File where messages are saved
filename = "data/messages.txt"
messages = []
if os.path.exists(filename):
    with open(filename, "r") as f:
        messages = f.readlines()

# Build the HTML response to display the messages
body = """\
<html>
<head>
    <title>Saved Messages</title>
</head>
<body>
    <h1>Saved Messages</h1>
    <ul>
"""
for message in messages:
    body += f"<li>{message.strip()}</li>\n"
body += """\
    </ul>
</body>
</html>
"""

content_length = len(body.encode("utf-8"))
print("HTTP/1.1 200 OK\r")
print("Content-Type: text/html\r")
print(f"Content-Length: {content_length}\r")
print("\r")
print(body)
