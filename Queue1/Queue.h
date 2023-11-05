#pragma once
#include <mutex>

template <class T>
struct QueueElem {
	T Data;
	QueueElem<T>* next;
	QueueElem(T _data) {
		Data = _data;
		next = nullptr;
	}
};

template <class T>
class Queue {
	QueueElem<T>* start;
	QueueElem<T>* end;
	size_t Q_size;
	std::mutex Q_lock;
public:
	Queue() {
		Q_lock.lock();
		start = nullptr;
		end = nullptr;
		Q_size = 0;
		Q_lock.unlock();
	}
	~Queue() {
		Q_lock.lock();
		if (start == nullptr) { Q_lock.unlock();  return; }
		QueueElem<T>* iterator = start;
		while (iterator->next != nullptr) {
			QueueElem<T>* tmp = iterator;
			iterator = iterator->next;
			delete tmp;
		}
		delete end;
		Q_lock.unlock();
	}
	bool isempty() {
		return end == nullptr;
	}
	void push(T _data) {
		Q_lock.lock();
		++Q_size;
		if (start == nullptr) {
			start = new QueueElem<T>(_data);
			end = start;
		}
		else {
			QueueElem<T>* tmp = start;
			start = new QueueElem<T>(_data);
			start->next = tmp;
		}
		Q_lock.unlock();
	}
	size_t size() {
		return Q_size;
	}
	T pop() {
		Q_lock.lock();
		--Q_size;
		if (end == nullptr) { Q_lock.unlock();  throw std::out_of_range("Queue is empty"); }
		T res = end->Data;
		if (start != end) {
			QueueElem<T>* tmp = start;
			while (tmp->next != end) {
				tmp = tmp->next;
			}
			delete end;
			end = tmp;
			end->next = nullptr;
			
			
		}
		else {
			
			delete end;
			end = nullptr;
			start = nullptr;
			
		}
		Q_lock.unlock();
		return res;

	}
};