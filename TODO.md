Server makes sure everything is parsed
POll decides what we're doing not the server

1. reconnect cgi to poll loop (djoyke)
2. use throw instead of std cout or std cerr and have a catch somewhere (lena, djoyke)
3. finish locations (lena) -> still need to test them
4. validate data while parsing (lena) - need to check with djoyke and anna what else needs validating
5. link throw to http response, and display correct page (djoyke, lena)
6. handling null characters for requesting images eg assets/nebula.jpg dont handle it like a string (lena)
7. should we assert more? (all)

WE USE UTILS... DO NOT REMOVE, love you!


TODO When Anna is back:
1. create post and delete (djoyke, anna)
2. add CGI scripts for post and delete (djoyke)
3. make ugly button not ugly in html (djoyke)
4. eval sheet misery (jan)
5. unit test (all, jan)
6. fix the download thing without us telling it to do so (anna)
7. set the fields as private variables in HTTPRequest (djoyke) so I don't copy the strings all the time
8. parse root, index and so on from config in handle request
9. do we accept HTTP/1.0?
10. do we need broadcast message?
11. Rename methods with CamelCase - done?
12. replace buffer size with READ_SIZE in utils.hpp
13. add charset 8 for Content type
14. handleGet check extension (img. html ..) instead cgi-bin
15. add timeout struct pollfd pfd = { _clientSocket, POLLIN | POLLOUT, 0 };
int ret = poll(&pfd, 1, 5000); // Timeout after 5000ms
if (ret == 0) {
    std::cerr << "Timeout on socket: " << _clientSocket << std::endl;
    close(_clientSocket);
}



to test non blocking:
# In terminal 1
curl -X POST --data-binary "@large_file_1.txt" http://localhost:8080/endpoint
# In terminal 2
curl -X POST --data-binary "@large_file_2.txt" http://localhost:8080/endpoint
# In terminal 3
curl -X POST --data-binary "@large_file_3.txt" http://localhost:8080/endpoint
