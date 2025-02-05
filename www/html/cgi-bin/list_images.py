#!/usr/bin/python3

import os

# Define the upload directory path (ensure this path is correct for your server setup)
UPLOAD_DIR = "/home/akrepkov/Desktop/webserv_git/git/www/html/images"

# Get a list of image files
try:
    images = [
        f"/images/{filename}" for filename in os.listdir(UPLOAD_DIR)
        if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.webp'))
    ]
except FileNotFoundError:
    images = []

# Construct the HTML body
body = """<!DOCTYPE html>
<html>
<head>
<title>Image Gallery</title>
<style>
body { background-color: black; color: white; font-family: Arial, sans-serif; text-align: center; }
button { padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }
button:hover { background-color: gray; }
img { margin: 10px; width: 200px; border-radius: 10px; }
</style>
</head>
<body>
<h1>Uploaded Images</h1>
"""

# Add images dynamically
for img in images:
    body += f'<img src="{img}" alt="Uploaded Image">\n'

body += """<br><button onclick="window.location.reload();">Refresh</button>
</body>
</html>"""

# Calculate Content-Length
content_length = len(body.encode('utf-8'))

# Send the correct HTTP headers
print("HTTP/1.1 200 OK\r")
print("Content-Type: text/html\r")
print(f"Content-Length: {content_length}\r")
print("\r")  # End of headers

# Send HTML body
print(body)


