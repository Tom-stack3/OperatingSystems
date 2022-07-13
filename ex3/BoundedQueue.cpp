#include "BoundedQueue.h"

BoundedQueue::BoundedQueue(int size) : _size(size)
{
    // Initialize semaphores
    sem_init(&_full, 0, 0);
    sem_init(&_empty, 0, size);
}

BoundedQueue::~BoundedQueue()
{
    // Destroy semaphores
    sem_destroy(&_full);
    sem_destroy(&_empty);
}

BoundedQueue::BoundedQueue(const BoundedQueue &other)
{
    _size = other._size;
    sem_init(&_full, 0, 0);
    sem_init(&_empty, 0, _size);
    std::queue<std::string>::operator=(other);
}

void BoundedQueue::push(std::string s)
{
    // Wait until there is an empty space in the queue
    sem_wait(&_empty);
    // Lock the queue
    _mutex.lock();
    // Push the string into the queue
    std::queue<std::string>::push(s);
    // Unlock the queue
    _mutex.unlock();
    // Signal that there is now one more full slot
    sem_post(&_full);
}

std::string BoundedQueue::pop()
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
    // Signal that there is now one more empty slot
    sem_post(&_empty);
    return s;
}

int BoundedQueue::get_size()
{
    return _size;
}
void BoundedQueue::set_size(int size)
{
    _size = size;
    // Destroy semaphores
    sem_destroy(&_full);
    sem_destroy(&_empty);
}