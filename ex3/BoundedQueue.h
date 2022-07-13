#ifndef BOUNDED_QUEUE_H
#define BOUNDED_QUEUE_H

#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>

class BoundedQueue : std::queue<std::string>
{
public:
    BoundedQueue(int size);
    ~BoundedQueue();
    BoundedQueue(const BoundedQueue &other);
    void push(std::string s);
    std::string pop();
    int get_size();
    void set_size(int size);

private:
    int _size;
    sem_t _full;
    sem_t _empty;
    std::mutex _mutex;
};

#endif