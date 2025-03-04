1. Evaluator can add infinite loop to script, need to catch it, ADD TIMEOUT! if headers not there triggers timeout (solved by time)(Djoyke)

2. Prepare POST for CGI (djoyke)

3. Big files stuck looping

4. Weird delay error page CGI

5. /usr/bin/python3: can't open file '/home/dreijans/webserve/www/html/cgi-bin/hello': [Errno 2] No such file or directory check in waitpid with NOHANG if child has exited and then error page weeeeeh


FOR TESTING:
server that is off - make it respond to specific request :
          curl --resolve webserv:8080:0.0.0.0 webserv:8080 

Just for us (traces): 
curl --trace-ascii trace.log -X POST -H "Content-Type: multipart/form-data" -F "file=@www/html/test_images/cat2.jpeg" http://localhost:8080/cat_upload.html



				**********TESTS**********

Check if website works:
curl localhost:8080

Check 404:
curl -v http://localhost:8080/nonexistent

Siege:
siege -c100 -t20S http://localhost:8080/

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


kill a process:
lsof -i :<PORT>
kill -9 <PID>