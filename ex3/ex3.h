#include "BoundedQueue.h"
#include "UnboundedQueue.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#define NUM_OF_TYPES 3
#define DONE "DONE"

struct producer_info
{
    int id;
    int num_of_products;
    int queue_size;
};