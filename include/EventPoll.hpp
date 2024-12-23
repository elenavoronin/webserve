#pragma once

#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

/**
 * @brief Structure to specify a file descriptor and event type to be removed.
 */
typedef struct s_pollfdToRemove {
	int fd;
	int eventType;
}	t_pollfdToRemove;

/**
 * @brief A class to manage poll events and their associated file descriptors.
 */
class EventPoll {

	private:
		std::vector<struct pollfd> 			_pollfds;						// List of active poll file descriptors
		std::vector<t_pollfdToRemove> 		_pollfdsToRemoveQueue;			// Queue of file descriptors to be removed.
		std::vector<pollfd>					_pollfdsToAddQueue;				// Queue of file descriptors to be added.

	public:
		EventPoll();
		~EventPoll();

		void 					updateEventList();
		std::vector<pollfd>& 	getPollEventFd();
		void 					addPollFdEventQueue(int fd, int eventType);
		void 					ToremovePollEventFd(int fd, int eventType);
};