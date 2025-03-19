#include <mutex>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <semaphore>
#include <cstdlib>

static std::ofstream outfile("log.txt");

class Node{
public:
    int value;
    Node* next;
};
/*No need to touch the above code */

// Lock-based Concurrent Queue (of strictly positive integers)
class ConcurrentQueue{
    Node* head;
    Node* tail;

    std::mutex* lock;
    std::counting_semaphore<>* count;
    std::counting_semaphore<>* freeSlots;

public:
    ConcurrentQueue(const int maxSize){
        // fill in
        tail = nullptr;
        head = nullptr;

        lock = new std::mutex();
        count = new std::counting_semaphore<>(0);
        freeSlots = new std::counting_semaphore<>(maxSize);
    }

    // Enqueues input value at the tail of the list
    void enqueue(int value){
        // fill in and add the two lines below:
        //sets up new node
        Node* newNode = new Node();
        newNode->next = nullptr;
        newNode->value = value;

        freeSlots->acquire();
        lock->lock();
        
        //if queue is not empty
        if (tail){
            tail->next = newNode;
            tail = newNode;
        }
        //if queue is empty
        else{
            tail = newNode;
            head = tail;
        }

        std::string s = "Enqueued " + std::to_string(value) + ". \n";
        outfile << s;

        lock->unlock();
        count->release();
    }

    // Dequeues value of head of the list, and outputs it
    // Should block if the list is empty
    int dequeue(){
        //fill in and add the two lines below, where returnValue is the output of the dequeue function:

        //only ever tries to get lock if head is not null
        count->acquire();
        lock->lock();

        //grab node and update head
        Node* tmp = head;
        head = head->next;
        if (!head) tail = head;

        std::string s = "Dequeued " + std::to_string(tmp->value) + ". \n";
        outfile << s;        

        //unlock then delete and return
        lock->unlock();
        freeSlots->release();
        
        int returnValue = tmp->value;
        delete tmp;

        return returnValue;
    }

    // Dequeues value of head of the list, and outputs it
    // Non-blocking, if the list is empty it returns NULL
    int try_dequeue(){
        // fill in and add the two lines below if successful dequeue, where returnValue is the output of the dequeue function:
        // Else add the two lines below if unsuccessful dequeue:

        //attemps to gain locks
        if (!count->try_acquire()){
            std::string s = "Failed dequeued. \n";
            outfile << s;

            return 0;
        }
        if (!lock->try_lock()){
            count->release();

            std::string s = "Failed dequeued. \n";
            outfile << s;

            return 0;
        }

        //grab node and update head
        Node* tmp = head;
        head = head->next;
        if (!head) tail = head;

        std::string s = "Successfully dequeued " + std::to_string(tmp->value) + ". \n";
        outfile << s;

        //unlock then delete and return
        lock->unlock();
        freeSlots->release();

        int returnValue = tmp->value;
        delete tmp;

        return returnValue;
    }

    // Looks (but does not remove) at value of head of the list, and outputs it
    // Should block if the list is empty
    int peek(){
        // fill in and add the two lines below, where returnValue is the head value (or value that stayed longest in the list):
        
        //only ever tries to get lock if head is not null
        count->acquire();
        lock->lock();

        //grab value
        int returnValue = head->value;

        std::string s = "Peeked " + std::to_string(returnValue) + ". \n";
        outfile << s;

        //unlock then return
        lock->unlock();
        count->release();
        
        return returnValue;
    }

    // Looks (but does not remove) at value of head of the list, and outputs it
    // Non-blocking, if the list is empty it returns NULL
    int try_peek(){
        // fill in and add the two lines below if successful peek, where returnValue is the head value (or value that stayed longest in the list):
        // Else add the two lines below if unsuccessful peek:

        //attemps to gain locks
        if (!count->try_acquire()){
            std::string s = "Failed peek. \n";
            outfile << s;

            return 0;
        }
        if (!lock->try_lock()){
            count->release();

            std::string s = "Failed peek. \n";
            outfile << s;

            return 0;
        }

        //grab value
        int returnValue = head->value;

        std::string s = "Successfully peeked " + std::to_string(returnValue) + ". \n";
        outfile << s;

        //unlock then return
        lock->unlock();
        count->release();

        return returnValue;
    }
};

/*From this part onwards, code to test the concurrent queue */
void randomEnqueuer(ConcurrentQueue &l){
    std::srand(std::time(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000)); 
    int queuedValue = rand() % 100+1;
    l.enqueue(queuedValue);
}

void randomDequeuer(ConcurrentQueue &l){
    std::srand(std::time(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000)); 
    l.dequeue();
}

void randomPeek(ConcurrentQueue &l){
    std::srand(std::time(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000)); 
    l.peek();
}

void randomTryDequeue(ConcurrentQueue &l){
    std::srand(std::time(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000)); 
    l.try_dequeue();
}

void randomTryPeek(ConcurrentQueue &l){
    std::srand(std::time(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000)); 
    l.try_peek();
}

int main(int argc, char *argv[]){
    int producerNumber = 14;
    int consumerNumber = 14;
    if (argc >= 2){
        producerNumber = std::strtol(argv[1], nullptr, 0);
        consumerNumber = std::strtol(argv[2], nullptr, 0);        
    }
    std::vector<std::thread> threads;
    ConcurrentQueue list(20);

    for (int i = 0;  i < producerNumber; i++){
        threads.push_back(std::thread(randomEnqueuer, std::ref(list)));
    }
    for (int i = 0;  i < consumerNumber; i++){
        threads.push_back(std::thread(randomDequeuer, std::ref(list)));
    }
    threads.push_back(std::thread(randomPeek, std::ref(list)));

    for (int i = 0; i < threads.size(); ++i){
        threads[i].join();
    }
    threads.push_back(std::thread(randomTryDequeue, std::ref(list)));
    threads.back().join();
    threads.push_back(std::thread(randomTryPeek, std::ref(list)));
    threads.back().join();

    outfile.close();
    return 0;
}
