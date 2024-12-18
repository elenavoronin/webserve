Server makes sure everything is parsed
POll decides what we're doing not the server

1. Do we need to add an instance of EventPoll in client or Server?
2. use throw instead of std cout or std cerr and have a catch somewhere (lena, djoyke)
3. finish locations (lena)
4. validate data while parsing (lena)
5. link throw to http response, and display correct page (djoyke, lena)
6. handling null characters for requesting images eg assets/nebula.jpg dont handle it like a string
7. should we assert more?

WE USE UTILS... DO NOT REMOVE, love you!




TODO When Anna is back:
1. create post and delete (djoyke, anna)
2. add CGI scripts for post and delete (djoyke)
3. make ugly button not ugly in html (djoyke)
4. eval sheet misery (jan)
5. unit test (all, jan)
6. fix the download thing without us telling it to do so (anna)
7. set the fields as private variables in HTTPRequest (djoyke) so I don't copy the strings all the time