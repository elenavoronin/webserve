#pragma once

#include <vector>
#include <poll.h>
#include <unistd.h>

typedef struct s_pollfdToRemove {
	int fd;
	int eventType;
}	t_pollfdToRemove;

class EventPoll {

	private:
		std::vector<struct pollfd> 			_pollfds;
		std::vector<t_pollfdToRemove> 		_pollfdsToRemoveQueue;
		std::vector<pollfd>					_pollfdsToAddQueue;

	public:
		EventPoll();
		~EventPoll();

		void 					updateEventList();
		std::vector<pollfd>& 	getPollEventFd();
		void 					addPollFdEventQueue(int fd, int eventType);
		void 					ToremovePollEventFd(int fd, int eventType);
};