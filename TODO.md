1. use throw instead of std cout or std cerr and have a catch somewhere (lena, djoyke)
2. Search for all read/recv/write/send and check if the returned value is correctly checked (checking only -1 or values is not enough, both should be checked).(anna)


FOR TESTING:
server that is off - make it respond to specific request :
          curl --resolve webserv:8080:0.0.0.0 webserv:8080
