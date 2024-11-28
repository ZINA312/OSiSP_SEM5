#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <random>

const int NUM_BLOCKS = 10;
const int NUM_READERS = 5;  
const int NUM_WRITERS = 2;  

struct SharedMemory {
    std::vector<int> data;
    std::shared_mutex rw_mutex; 

    SharedMemory() : data(NUM_BLOCKS, 0) {}
};

void reader(SharedMemory& shared_mem, int id) {
    while (true) {
        {
            std::shared_lock<std::shared_mutex> lock(shared_mem.rw_mutex);
            std::cout << "Reader " << id << " read data: ";
            for (const auto& value : shared_mem.data) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
    }
}

void writer(SharedMemory& shared_mem, int id) {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1, 100);

    while (true) {
        {
            std::unique_lock<std::shared_mutex> lock(shared_mem.rw_mutex);
            int block = distribution(generator) % NUM_BLOCKS;
            shared_mem.data[block] = distribution(generator);
            std::cout << "Writer " << id << " wrote data: " << shared_mem.data[block] << " to block " << block << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
    }
}

int main() {
    SharedMemory shared_mem;

    std::vector<std::thread> readers;
    std::vector<std::thread> writers;

    for (int i = 0; i < NUM_READERS; ++i) {
        readers.emplace_back(reader, std::ref(shared_mem), i);
    }

    for (int i = 0; i < NUM_WRITERS; ++i) {
        writers.emplace_back(writer, std::ref(shared_mem), i);
    }

    for (auto& t : readers) {
        t.join();
    }
    for (auto& t : writers) {
        t.join();
    }

    return 0;
}