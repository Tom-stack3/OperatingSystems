#include "UnboundedQueue.h"

UnboundedQueue::UnboundedQueue()
{
    // Initialize semaphores
    sem_init(&_full, 0, 0);
}

UnboundedQueue::~UnboundedQueue()
{
    // Destroy semaphores
    sem_destroy(&_full);
}

UnboundedQueue::UnboundedQueue(const UnboundedQueue& other){
    sem_init(&_full, 0, 0);
    std::queue<std::string>::operator=(other);
}

void UnboundedQueue::push(std::string s)
{
    // No need to wait until there is an empty space, since the queue is unbounded
    // Lock the queue
    _mutex.lock();
    // Push the string into the queue
    std::queue<std::string>::push(s);
    // Unlock the queue
    _mutex.unlock();
    // Signal that there is now one more full slot
    sem_post(&_full);
}

std::string UnboundedQueue::pop()
{
    // Wait until there is something in the queue
    sem_wait(&_full);
    // Lock the queue
    _mutex.lock();
    // Pop the string from the queue
    std::string s = std::queue<std::string>::front();
    std::queue<std::string>::pop();
    // Unlock the queue
    _mutex.unlock();
    return s;
}