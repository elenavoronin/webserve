#!/usr/bin/env python3

import os
import urllib.parse
import sys

# Determine the project root based on the script's location
script_dir = os.path.dirname(os.path.abspath(__file__))  # Directory of this script
project_root = os.path.abspath(os.path.join(script_dir, "../../.."))  # Adjust as needed to reach the project root
safe_dir = os.path.join(project_root, "www/html/assets")  # Path to the assets directory

# Get the query string from the environment
query_string = os.environ.get("QUERY_STRING", "")
query_params = urllib.parse.parse_qs(query_string)
filename = query_params.get("file", [None])[0]

if not filename:
    # Send error headers if the file parameter is missing
    error_html = "<html><body><h1>400 Bad Request</h1><p>File parameter is missing.</p></body></html>"
    print("HTTP/1.1 400 Bad Request\r")
    print("Content-Type: text/html\r")
    print(f"Content-Length: {len(error_html.encode('utf-8'))}\r")
    print("\r")
    print(error_html)
else:
    print("BASENAME: ", os.path.basename(filename))
    print("project_root: ", project_root)
    print("script_dir: ", script_dir)
    print("safe_dir: ", safe_dir)
    # Build the full file path and sanitize the file name
    filepath = os.path.join(safe_dir, os.path.basename(filename))

    # Log the computed file path for debugging
    print(f"DEBUG: Computed file path: {filepath}", file=sys.stderr)

    if not os.path.isfile(filepath):
        # Send error headers if the file is not found
        error_html = "<html><body><h1>404 Not Found</h1><p>The requested file was not found.</p></body></html>"
        print("HTTP/1.1 404 Not Found\r")
        print("Content-Type: text/html\r")
        print(f"Content-Length: {len(error_html.encode('utf-8'))}\r")
        print("\r")
        print(error_html)
    else:
        try:
            # Read the file in binary mode
            with open(filepath, "rb") as file:
                content = file.read()

            # Send success headers
            
            print("HTTP/1.1 200 OK\r")
            print("Content-Type: application/pdf\r")
            print(f"Content-Length: {len(content)}\r")
            print("\r")  # Blank line to indicate the end of headers
            # Send the file content
            sys.stdout.flush() 
            sys.stdout.buffer.write(content)

        except Exception as e:
            # Send error headers in case of a read failure
            error_html = f"<html><body><h1>500 Internal Server Error</h1><p>{str(e)}</p></body></html>"
            print("HTTP/1.1 500 Internal Server Error\r")
            print("Content-Type: text/html\r")
            print(f"Content-Length: {len(error_html.encode('utf-8'))}\r")
            print("\r")
            print(error_html)
