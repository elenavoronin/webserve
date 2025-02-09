#!/usr/bin/env python3
import cgi
import os
import urllib.parse
# Process the form data
form = cgi.FieldStorage()
name = form.getvalue("name")
bio = form.getvalue("bio")
image_file = form["image"]


image_path = "../upload/" + image_file.filename
with open(image_path, "wb") as f:
    f.write(image_file.file.read())

# Calculate Content-Length
# URL encode the values to ensure they are properly formatted for the query string
encoded_name = urllib.parse.quote(name)
encoded_bio = urllib.parse.quote(bio)
encoded_image = urllib.parse.quote(image_file.filename)
# Send HTTP headers
print("HTTP/1.1 302 Found")
print(f"Location: /result.html?name={encoded_name}&bio={encoded_bio}&image={encoded_image}")
print("Content-Type: text/html")
print("\r\n")

# Now, we can redirect the user to result.html

