#include "ex3.h"

// Create n bounded queues for the producers
std::vector<BoundedQueue> producer_queues;
// Create three unbounded queues for the dispatcher
UnboundedQueue dispatcher_queues[3];
char const *TYPES[] = {"SPORTS", "NEWS", "WEATHER"};

void producer(const producer_info info, const int index)
{
    int counter[] = {0, 0, 0};
    for (int i = 0; i < info.num_of_products; ++i)
    {
        std::string type = TYPES[i % NUM_OF_TYPES];
        std::string s = "Producer " + std::to_string(info.id) + " " + type + " " + std::to_string(counter[i % NUM_OF_TYPES]);
        ++counter[i % NUM_OF_TYPES];
        producer_queues[index].push(s);
    }
    producer_queues[index].push(DONE);
}

void dispatcher()
{
    int num_of_producers_left = producer_queues.size();
    while (num_of_producers_left > 0)
    {
        for (auto &producer : producer_queues)
        {
            if (producer.get_size() < 0)
            {
                continue;
            }
            // Get a message from the producer
            std::string s = producer.pop();
            // If s == DONE then we are done with this producer
            if (s == DONE)
            {
                num_of_producers_left--;
                producer.set_size(-1);
                continue;
            }
            // Push the message to the right dispatcher queue
            if (s.find("SPORTS") != std::string::npos)
            {
                dispatcher_queues[0].push(s);
            }
            else if (s.find("NEWS") != std::string::npos)
            {
                dispatcher_queues[1].push(s);
            }
            else if (s.find("WEATHER") != std::string::npos)
            {
                dispatcher_queues[2].push(s);
            }
        }
    }
    // Push DONE to all the dispatcher queues
    dispatcher_queues[0].push(DONE);
    dispatcher_queues[1].push(DONE);
    dispatcher_queues[2].push(DONE);
}

void co_editor(const int index, BoundedQueue &co_editor_queue)
{
    bool done = false;
    while (!done)
    {
        // Get a message from the dispatcher queue
        std::string s = dispatcher_queues[index].pop();
        // If s == DONE then we are done with this dispatcher queue
        if (s == DONE)
        {
            done = true;
            co_editor_queue.push(DONE);
            continue;
        }
        // Sleep for 0.1 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Push the message to the co-editor queue
        co_editor_queue.push(s);
    }
}

void screen_manager(BoundedQueue &co_editor_queue)
{
    int done_counter = 0;
    while (done_counter < 3)
    {
        // Get a message from the co-editor queue
        std::string s = co_editor_queue.pop();
        // If s == DONE then we are done with this co-editor queue
        if (s == DONE)
        {
            ++done_counter;
            continue;
        }
        std::cout << s << std::endl;
    }
    std::cout << "DONE" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <path to config file>" << std::endl;
        return 1;
    }
    // Read the config file and initialize the queues and the threads
    // Assume the config file is valid and in the first format

    int n;
    int co_editors_queue_size;
    // Create a vector of producer info structs
    std::vector<producer_info> producer_infos;

    // Read from the text file
    std::ifstream config_file(argv[1]);
    std::string line;
    int line_num = 0;
    // If the file is not open, return an error
    if (!config_file.is_open())
    {
        std::cout << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    // Use a while loop together with the getline() function to read the file line by line
    while (getline(config_file, line))
    {
        // Increment the line number
        ++line_num;
        // If line is empty or just spaces, continue to the next line
        if (line.empty() || line.find_first_not_of(' ') == std::string::npos)
        {
            continue;
        }
        // If line starts with: "Co-Editor queue size = "
        else if (line.find("Co-Editor queue size = ") != std::string::npos)
        {
            // Get the queue size
            std::string queue_size_str = line.substr(line.find("=") + 1);
            co_editors_queue_size = std::stoi(queue_size_str);
        }
        // If line starts with: "PRODUCER "
        else if (line.find("PRODUCER ") != std::string::npos)
        {
            // Get the producer id
            std::string id_str = line.substr(line.find(" ") + 1);
            int id = std::stoi(id_str);
            // Get the number of products
            getline(config_file, line);
            std::string num_of_products_str = line.substr(line.find(" ") + 1);
            int num_of_products = std::stoi(num_of_products_str);
            // Get the queue size
            getline(config_file, line);
            std::string queue_size_str = line.substr(line.find("=") + 1);
            int queue_size = std::stoi(queue_size_str);
            // Create a producer info struct and add it to the vector
            producer_info producer_info;
            producer_info.id = id;
            producer_info.num_of_products = num_of_products;
            producer_info.queue_size = queue_size;
            producer_infos.push_back(producer_info);
        }
        else
        {
            // Print an error message and exit the program
            std::cout << "Invalid config file at line:'" << line << "'" << line_num << std::endl;
            // Close the file
            config_file.close();
            return 1;
        }
    }
    // Close the file
    config_file.close();

    n = producer_infos.size();
    // Create a bounded queue for the co-editors
    BoundedQueue co_editors_queue(co_editors_queue_size);

    for (int i = 0; i < n; ++i)
    {
        producer_queues.push_back(BoundedQueue(producer_infos[i].queue_size));
    }
    // Create n threads for the producers
    std::vector<std::thread> producer_threads;
    for (int i = 0; i < n; ++i)
    {
        producer_threads.push_back(std::thread(producer, producer_infos[i], i));
    }
    // Create a thread for the dispatcher
    std::thread dispatcher_thread(dispatcher);
    // Create a thread for the three co-editors
    std::vector<std::thread> co_editor_threads;
    for (int i = 0; i < 3; ++i)
    {
        co_editor_threads.push_back(std::thread(co_editor, i, std::ref(co_editors_queue)));
    }
    // Create a thread for the screen manager
    std::thread screen_manager_thread(screen_manager, std::ref(co_editors_queue));

    // Wait for all the producers to finish
    for (std::thread &th : producer_threads)
    {
        if (th.joinable())
        {
            th.join();
        }
    }
    // Wait for the dispatcher to finish
    if (dispatcher_thread.joinable())
    {
        dispatcher_thread.join();
    }
    // Wait for all the co-editors to finish
    for (std::thread &th : co_editor_threads)
    {
        if (th.joinable())
        {
            th.join();
        }
    }
    // Wait for the screen manager to finish
    if (screen_manager_thread.joinable())
    {
        screen_manager_thread.join();
    }

    return 0;
}