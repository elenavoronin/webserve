# #!/usr/bin/env python3

# import cgi

# # Parse form data
# form = cgi.FieldStorage()
# name = form.getvalue('name', 'Guest')

# # Generate response
# body = f"""
# <html>
# <head>
#     <title>Login Successful</title>
#     <style>
#         body {{ background-color: black; color: white; font-family: Arial, sans-serif; }}
#         button {{ padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }}
#         button:hover {{ background-color: gray; }}
#     </style>
# </head>
# <body>
#     <h1>Welcome, {name}!</h1>
#     <p>You are now logged in.</p>
#     <p>
#         <a href="/index.html" style="text-decoration: none;">
#             <button>Return to Homepage</button>
#         </a>
#     </p>
# </body>
# </html>
# """

# # Calculate Content-Length
# content_length = len(body.encode('utf-8'))

# # Send HTTP headers
# print("HTTP/1.1 200 OK\r")
# print("Content-Type: text/html\r")
# print(f"Content-Length: {content_length}\r")
# print("\r")  # Blank line to separate headers from body

# # Send response body
# print(body)

#!/usr/bin/env python3

import cgi

# Parse form data
form = cgi.FieldStorage()
name = form.getvalue('name', 'Guest')

# Generate response
body = f"""
<html>
<head>
    <title>Login Successful</title>
    <style>
        body {{ background-color: black; color: white; font-family: Arial, sans-serif; }}
        button {{ padding: 10px 20px; font-size: 16px; color: white; background-color: black; border: none; border-radius: 5px; cursor: pointer; }}
        button:hover {{ background-color: gray; }}
    </style>
</head>
<body>
    <h1>Welcome, {name}!</h1>
    <p>You are now logged in. You can proceed to upload a photo of your favorite cat.</p>
    <p>
        <a href="/cgi-bin/upload_cat.py" style="text-decoration: none;">
            <button>Go to Upload Page</button>
        </a>
    </p>
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
