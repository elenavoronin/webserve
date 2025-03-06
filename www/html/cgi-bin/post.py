#!/usr/bin/env python3
import os
import cgi
import cgitb
import datetime
# import sys

# sys.stderr.write("Received POST data: " + repr(os.environ.get("CONTENT_LENGTH")) + "\n")


# Enable debugging
cgitb.enable()

# Create the data folder if it doesn't exist
os.makedirs("data", exist_ok=True)

# Ensure the request method is POST
if os.environ.get("REQUEST_METHOD", "GET") != "POST":
    print("Content-Type: text/plain")
    print()
    print("Error: This script only accepts POST requests.")
    exit(1)

# Parse form data
form = cgi.FieldStorage()
name = form.getvalue("name", "Anonymous")
email = form.getvalue("email", "No email provided")
message = form.getvalue("message", "No message")

# Save the submitted data to a file (make sure the directory has write permission)
timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
entry = f"{timestamp} - Name: {name}, Email: {email}, Message: {message}\n"

# Append the new entry to the messages file
with open("data/messages.txt", "a") as f:
    f.write(entry)

# Build the confirmation HTML response
body = f"""\
<html>
<head>
    <title>Message Saved</title>
    <style>
        body {{ background-color: black; color: white; font-family: Arial, sans-serif; }}
        button {{ padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }}
        button:hover {{ background-color: purple; }}
    </style>
</head>
<body>
    <h1>Thank you, {name}!</h1>
    <p>Your message has been saved.</p>
    <p>
        <a href="/cgi-bin/get.py">View all messages</a>
    </p>
    <p>
        <a href="/index.html" style="text-decoration: none;">
            <button>Return to Homepage</button>
        </a>
    </p>
</body>
</html>
"""

content_length = len(body.encode("utf-8"))

print("HTTP/1.1 200 OK\r")
print("Content-Type: text/html\r")
print(f"Content-Length: {content_length}\r")
print("\r")

print(body)
