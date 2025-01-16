#!/usr/bin/env python3

import cgi
import os
import re

# Directory to store uploaded files
UPLOAD_DIR = "/home/dreijans/uploads/"
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Sanitize file names to prevent security issues
def sanitize_filename(filename):
    return re.sub(r'[^\w\-.]', '_', filename)

# Get the request method
request_method = os.environ.get('REQUEST_METHOD', 'GET')

if request_method == 'POST':
    # Handle file upload
    form = cgi.FieldStorage()

    if 'photo' not in form or not form['photo'].filename:
        # Respond with an error if no file was uploaded
        error_html = """
        <html>
        <head><title>Upload Failed</title></head>
        <body>
            <h1>Upload Failed</h1>
            <p>No file was uploaded. Please try again.</p>
            <a href="/cgi-bin/upload_cat.py"><button>Return to Upload Page</button></a>
        </body>
        </html>
        """
        print("HTTP/1.1 400 Bad Request\r")
        print("Content-Type: text/html\r")
        print(f"Content-Length: {len(error_html.encode('utf-8'))}\r")
        print("\r")  # End of headers
        print(error_html)
    else:
        # Get the uploaded file
        fileitem = form['photo']
        filename = sanitize_filename(fileitem.filename)
        filepath = os.path.join(UPLOAD_DIR, filename)

        # Save the file
        with open(filepath, 'wb') as f:
            f.write(fileitem.file.read())

        # Respond with success message
        success_html = f"""
        <html>
        <head><title>Upload Successful</title></head>
        <body>
            <h1>Upload Successful!</h1>
            <p>Your file <strong>{filename}</strong> has been uploaded successfully.</p>
            <img src="/uploads/{filename}" alt="Uploaded Photo" style="max-width: 300px; margin-top: 20px;">
            <br><br>
            <a href="/cgi-bin/upload_cat.py"><button>Upload Another Photo</button></a>
        </body>
        </html>
        """
        print("HTTP/1.1 200 OK\r")
        print("Content-Type: text/html\r")
        print(f"Content-Length: {len(success_html.encode('utf-8'))}\r")
        print("\r")  # End of headers
        print(success_html)
else:
    # Serve the upload form for GET requests
    form_html = """
    <html>
    <head>
        <title>Upload Cat Photo</title>
        <style>
            body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; padding: 20px; }
            h1 { color: #444; }
            form { background: white; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
            label { display: block; margin-bottom: 10px; }
            input[type="file"] { margin-bottom: 20px; }
            button { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 5px; cursor: pointer; }
            button:hover { background-color: #45a049; }
        </style>
    </head>
    <body>
        <h1>Upload Your Cat Photo</h1>
        <form action="/cgi-bin/upload_cat.py" method="POST" enctype="multipart/form-data">
            <label for="photo">Choose a photo:</label>
            <input type="file" id="photo" name="photo" accept="image/*" required>
            <button type="submit">Upload Photo</button>
        </form>
    </body>
    </html>
    """
    print("HTTP/1.1 200 OK\r")
    print("Content-Type: text/html\r")
    print(f"Content-Length: {len(form_html.encode('utf-8'))}\r")
    print("\r")  # End of headers
    print(form_html)
