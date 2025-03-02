1. Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0
http://example.com/ (http://example.com/)). (Lena)

2. Setup default error page (try to change the error 404). When we change default : "	error_page		404 www/html/504.html    " it still gives error 404 - change errorResponse everywhere (Lena)

4. Something weird with locations, requesting /no when autoindex is off should displaye www/html/root2/no/index.html, but it's an empty page, also not for location /bin the notindex.html. ergo looks like something with index files in location blocks doesnt work correctly (Lena)

7. Kind of more on top of the second todo, curl -X PUT localhost:8080, this doesn't display the error page, but just a string (Leno?)

8. chdir before execve and change hard coded root stuff (getRoot does not return root) (Djoyke)

9. localhost:8080///// should give error page (low prio)

10. Evaluator can add infinite loop to script, need to catch it, ADD TIMEOUT! if headers not there triggers timeout (solved by time)(Djoyke)

11. Prepare POST for CGI (djoyke)

12. Handle script errors CGI (djoyke) 



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

