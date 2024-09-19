
#ifndef SERVER_HPP
#define SERVER_HPP
#include <vector>

class Server{
	public:
		Server();
		~Server();
		Server& operator=(const Server& copy);
		Server(const Server& copy);


		void run();
		/*listener socket*/
		int 	report_ready(std::vector<struct pollfd> &pfds);
		int		get_listener_socket();
		int 	sendall(int s, char *buf, int *len);
		/*Fd management*/
		void 	add_to_pfds(std::vector<struct pollfd> &pfds, int newfd);
		void 	del_from_pfds(std::vector<struct pollfd> &pfds, int i);
		/*Main loop*/
		void 	handle_new_connection(int listener, std::vector<struct pollfd> &pfds);
		void 	handle_client_data(std::vector<struct pollfd> &pfds, int i, int listener);
		void 	broadcast_message(int sender_fd, char *buf, int received, std::vector<struct pollfd> &pfds, int listener);




};

#endif