**********TESTS**********

server that is off - make it respond to specific request :
          curl --resolve webserv:8080:0.0.0.0 webserv:8080 

Just for us (traces): 
curl --trace-ascii trace.log -X POST -H "Content-Type: multipart/form-data" -F "file=@www/html/test_images/cat2.jpeg" http://localhost:8080/cat_upload.html

Check if website works:
curl localhost:8080

Check 404:
curl -v http://localhost:8080/nonexistent

Siege:
siege -c100 -t20S http://localhost:8080/
siege -b http://localhost:8080 - this one keeps ssending requests, you can stop it with ctrl+c and it will give you stats

Wrong formating:
curl -X POST -H "Content-Type: plain/text" --data "We don't accept plain/text, only multiform, it should give 406 error" localhost:8080

Posting through curl:
curl -v -X POST -H "Content-Type: multipart/form-data" -F "file=@www/html/test_images/cat2.jpeg" http://localhost:8080/cat_upload.html

Delete (make sure the image is already uploaded):
curl -X DELETE localhost:8080/upload/cat.jpeg

Upload text (for example Makefile)
curl -v -X POST -H "Content-Type: multipart/form-data" -F "file=@Makefile" http://localhost:8080/cat_upload.html
Show it in the browser:
http://localhost:8080/upload/Makefile

The CGI should be run in the correct directory for relative path file access
add to cgi script: file = open("hi","x")

CGI POST
curl -v \
     -X POST \
     -H "Content-Type: application/x-www-form-urlencoded" \
     -d "name=John+Doe&email=john%40example.com&message=Hello+World" \
     http://localhost:8080/cgi-bin/post.py


kill a process:
lsof -i :<PORT>
kill -9 <PID>

curl --resolve webserv:8080:127.0.0.1 \
     -v \
     -X POST \
     -H "Content-Type: application/x-www-form-urlencoded" \
     -d "name=John+Doe&email=john%40example.com&message=Hello+World" \
     http://webserv:8080/cgi-bin/post.py
