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
    <style>
        body {{ background-color: black; color: white; font-family: Arial, sans-serif; }}
        button {{ padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }}
        button:hover {{ background-color: purple; }}
    </style>
</head>
<body>
    <p><h1>Saved Messages</h1></p>
    <p>
        <a href="/index.html" style="text-decoration: none;">
            <button>Return to Homepage</button>
        </a>
    </p>
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
