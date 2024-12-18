#include "EventPoll.hpp"


// struct pollfd {
//     int fd;         /* File descriptor to monitor */
//     short events;   /* Events to monitor (input, output, errors, etc.) */
//     short revents;  /* Events that occurred (filled by poll()) */
// };

EventPoll::EventPoll() {}

EventPoll::~EventPoll() {}

std::vector<pollfd>& EventPoll::getPollEventFd() {
	return _pollfds;
}

void	EventPoll::addPollFdEventQueue(int fd, int eventType)
{
	pollfd	newEvent;

	newEvent.fd = fd;
	newEvent.events = eventType;
	newEvent.revents = 0;
	_pollfdsToAddQueue.push_back(newEvent);
}

void	EventPoll::ToremovePollEventFd(int fd, int eventType)
{
	t_pollfdToRemove	tmp;

	tmp.fd = fd;
	tmp.eventType = eventType;
	_pollfdsToRemoveQueue.push_back(tmp);
}

static void eraseFromList(std::vector<pollfd>& list, s_pollfdToRemove fdToErase)
{
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (it->fd == fdToErase.fd && it->events == fdToErase.eventType) {
            list.erase(it);
            return; // Exit after erasing the first match
        }
    }
}

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
	}
}
