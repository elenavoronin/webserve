Server makes sure everything is parsed
POll decides what we're doing not the server

1. use throw instead of std cout or std cerr and have a catch somewhere (lena, djoyke)
2. handling null characters for requesting images eg assets/nebula.jpg dont handle it like a string (lena)
3. should we assert more? (all)
4. close pipe when something goes wrong reading or writing (djoyke)
5. test config file with 2 servers
6. add CGI scripts for post and delete (djoyke)
7. make ugly button not ugly in html (djoyke)
8. eval sheet misery (jan)
9. unit test (all, jan)
10. fix the download thing without us telling it to do so (anna)
11. do we need broadcast message?
12. add charset 8 for Content type
13. handleGet check extension (img. html ..) instead cgi-bin
14. Redirections (Lena)
15. Directory listing (Lena)
16. Cgi - infinite loops handling and error handling (Djoyke)
17. Test locations (Lena)

WE USE UTILS... DO NOT REMOVE, love you!


Basic Siege command for testing localhost:8080:
siege -c 10 -t 10S http://localhost:8080

to test non blocking:
# In terminal 1
curl -X POST --data-binary "@large_file_1.txt" http://localhost:8080/endpoint
# In terminal 2
curl -X POST --data-binary "@large_file_2.txt" http://localhost:8080/endpoint
# In terminal 3
curl -X POST --data-binary "@large_file_3.txt" http://localhost:8080/endpoint


test POST request:
curl -X POST http://localhost:8080/your-endpoint \
     -H "Content-Type: application/x-www-form-urlencoded" \
     -d "name=John&age=30"

curl -X POST http://localhost:8080/your-endpoint \
     -F "file=@path/to/your/file.txt"

curl -X POST http://localhost:8080/your-endpoint -d "name=John&age=30"


To kill a process, you can use the kill command in Linux or macOS. Follow these steps:

1. Find the Process ID (PID)
Use a command like ps or top to identify the process you want to kill.

Using ps:

bash
Copy code
ps aux | grep <process_name>
Replace <process_name> with the name of the process, such as webserv.

Using top: Press q to quit once you've noted the PID.

Using lsof: If the process is tied to a specific port or file:

bash
Copy code
lsof -i :8080
2. Kill the Process
Once you have the PID, use the kill command:

Graceful Kill (SIGTERM)
bash
Copy code
kill <PID>
This sends the SIGTERM signal, allowing the process to shut down cleanly.

Force Kill (SIGKILL)
If the process doesn’t terminate with SIGTERM, use SIGKILL:

bash
Copy code
kill -9 <PID>
This forces the process to terminate immediately.