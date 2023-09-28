#include "subject.hpp"

void Subject::notify(Event e)
{
	for (Observer* o : observers)
	{
		o->onNotify(e);
	}
}

void Subject::addObserver(Observer* observer)
{
	observers.push_back(observer);
}

void Subject::removeObserver(Observer* observer)
{
	assert(std::find(observers.begin(), observers.end(), observer) != observers.end());
	observers.remove(observer);
}

void Subject::clearObservers()
{
	observers.clear();
}