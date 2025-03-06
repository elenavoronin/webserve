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
 * @brief      Updates the internal poll event list by adding new events
 *             and removing old ones.
 *
 *             This function should be called before polling to ensure
 *             that the internal list of events is up to date.
 */
void	EventPoll::updateEventList(void)
{

	while (!_pollfdsToRemoveQueue.empty()) {
        t_pollfdToRemove fdToRemove = _pollfdsToRemoveQueue.back();
        _pollfdsToRemoveQueue.pop_back();

        auto it = std::find_if(_pollfds.begin(), _pollfds.end(),
                               [&](const pollfd& p) {
                                   return p.fd == fdToRemove.fd && p.events == fdToRemove.eventType;
                               });
        if (it != _pollfds.end()) {
            _pollfds.erase(it);
            _pollfds.shrink_to_fit();
        }
    }
    while (!_pollfdsToAddQueue.empty()) {
        pollfd newEvent = _pollfdsToAddQueue.back();
        _pollfdsToAddQueue.pop_back();

        // Ensure no duplicates
        auto it = std::find_if(_pollfds.begin(), _pollfds.end(),
                               [&](const pollfd& p) { return p.fd == newEvent.fd; });
        if (it == _pollfds.end()) {
            _pollfds.push_back(newEvent);
        }
    }
}