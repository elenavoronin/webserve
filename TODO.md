Server makes sure everything is parsed
POll decides what we're doing not the server

1. use throw instead of std cout or std cerr and have a catch somewhere (lena, djoyke)
2. make the main page (anna)
3. eval sheet misery (JI)
4. Check errno (anna)
5. Search for all read/recv/write/send on a socket and check that, if an error is returned, the client is removed.(anna)
6. Search for all read/recv/write/send and check if the returned value is correctly checked (checking only -1 or values is not enough, both should be checked).(anna)


FOR TESTING:
server that is off - make it respond to specific request :
          curl --resolve webserv:8080:0.0.0.0 webserv:8080



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



BOOOOOOO!!!!
AddressSanitizer:DEADLYSIGNAL
=================================================================
==79619==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000 (pc 0x0000004ddb8d bp 0x7ffcefb5d570 sp 0x7ffcefb5d510 T0)
==79619==The signal is caused by a READ memory access.
==79619==Hint: address points to the zero page.
    #0 0x4ddb8d in Client::closeConnection(EventPoll&) /home/dreijans/Documents/webserv/src/Client.cpp:300:35
    #1 0x4ec697 in Server::handlePollEvent(EventPoll&, int, Server&) /home/dreijans/Documents/webserv/src/Server.cpp:144:11
    #2 0x50c46b in Config::pollLoop() /home/dreijans/Documents/webserv/src/Config.cpp:308:39
    #3 0x50bd85 in Config::addPollFds() /home/dreijans/Documents/webserv/src/Config.cpp:262:5
    #4 0x50c873 in Config::checkConfig(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) /home/dreijans/Documents/webserv/src/Config.cpp:342:9
    #5 0x4d27da in main /home/dreijans/Documents/webserv/src/start.cpp:14:16
    #6 0x7fd4bf7a0d8f in __libc_start_call_main csu/../sysdeps/nptl/libc_start_call_main.h:58:16
    #7 0x7fd4bf7a0e3f in __libc_start_main csu/../csu/libc-start.c:392:3
    #8 0x423d84 in _start (/home/dreijans/Documents/webserv/webserv+0x423d84)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV /home/dreijans/Documents/webserv/src/Client.cpp:300:35 in Client::closeConnection(EventPoll&)
==79619==ABORTING
