// Source: https://gameprogrammingpatterns.com/observer.html

#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "observer.hpp"
#include <list>

class Subject
{
private:
	//List has various methods for list manipulation
	std::list<Observer*> observers;

protected:
	void notify(Event event);
public:
	void addObserver(Observer* observer);
	void removeObserver(Observer* observer);
	void clearObservers();
};