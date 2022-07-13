#ifndef UNBOUNDED_QUEUE_H
#define UNBOUNDED_QUEUE_H

#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>

class UnboundedQueue : std::queue<std::string>
{
public:
    UnboundedQueue();
    ~UnboundedQueue();
    UnboundedQueue(const UnboundedQueue& other);
    void push(std::string s);
    std::string pop();

private:
    sem_t _full;
    std::mutex _mutex;
};

#endif