#!/usr/bin/env python3
import cgi

form = cgi.FieldStorage()

username = form.getvalue("username")

print("Content-Type: text/html\n")
print("<html>")
print("<head>")
print("<title>CGI POST Response</title>")
print("</head>")
print("<body>")
print("<h2>CGI Script Response</h2>")
print("<p>Username: {}</p>".format(username))
print("</body>")
print("</html>")
