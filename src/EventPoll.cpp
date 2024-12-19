#include "EventPoll.hpp"

EventPoll::EventPoll() {}

EventPoll::~EventPoll() {}

/**
 * @brief Retrieves the current list of poll events being monitored.
 *
 * This function provides direct access to the internal vector of `pollfd`
 * structures used by the EventPoll class to manage monitored file descriptors.
 *
 * @return A reference to the vector of `pollfd` structures.
 */
std::vector<pollfd>& EventPoll::getPollEventFd() {
	return _pollfds;
}

/**
 * @brief Adds a file descriptor to the poll queue.
 *
 * This function adds a file descriptor to the poll queue to be monitored for
 * the given event type. The addition is done in the updateEventList()
 * function.
 *
 * @param fd The file descriptor to add to the poll queue.
 * @param eventType The event type (such as POLLIN or POLLOUT) to add.
 */
void	EventPoll::addPollFdEventQueue(int fd, int eventType)
{
	pollfd	newEvent;

	newEvent.fd = fd;
	newEvent.events = eventType;
	newEvent.revents = 0;
	_pollfdsToAddQueue.push_back(newEvent);
}

/**
 * @brief Removes a file descriptor from the poll queue.
 *
 * This function adds a file descriptor to a queue to be removed from the poll
 * list. The removal is done in the updateEventList() function.
 *
 * @param fd The file descriptor to remove.
 * @param eventType The event type (such as POLLIN or POLLOUT) to remove.
 */
void	EventPoll::ToremovePollEventFd(int fd, int eventType)
{
	t_pollfdToRemove	tmp;

	tmp.fd = fd;
	tmp.eventType = eventType;
	_pollfdsToRemoveQueue.push_back(tmp);
}

/**
 * @brief       Removes the first occurrence of a pollfd element from the list
 *              that matches the given fd and eventType.
 *
 * @param[in]   list       The list of pollfd elements to search in.
 * @param[in]   fdToErase  The pollfd element to remove, given as a s_pollfdToRemove struct.
 *
 * @details     This function iterates over the list and erases the first pollfd element
 *              that matches the given file descriptor and event type. If no match is
 *              found, the list remains unchanged.
 */
static void eraseFromList(std::vector<pollfd>& list, s_pollfdToRemove fdToErase)
{
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (it->fd == fdToErase.fd && it->events == fdToErase.eventType) {
            list.erase(it);
            return; // Exit after erasing the first match
        }
    }
}

/**
 * @brief      Updates the internal poll event list by adding new events
 *             and removing old ones.
 *
 *             This function should be called before polling to ensure
 *             that the internal list of events is up to date.
 */
void	EventPoll::updateEventList( void )
{
	// remove every fd from the removal list
	while (_pollfdsToRemoveQueue.size() != 0) {
		t_pollfdToRemove	fdToRemove = _pollfdsToRemoveQueue.back();

		_pollfdsToRemoveQueue.pop_back();
		eraseFromList(_pollfds, fdToRemove);
	}

	// add every fd from the addition list
	while (_pollfdsToAddQueue.size() != 0) {
		_pollfds.push_back(_pollfdsToAddQueue.back());
		_pollfdsToAddQueue.pop_back();
		std::cout << "ADDED POLL EVENT " << _pollfds.back().fd << std::endl;
	}
}