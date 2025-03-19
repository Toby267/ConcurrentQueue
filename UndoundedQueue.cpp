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
    // You might need additional variables here
    //std::binary_semaphore& enqueueSemaphore = *new std::binary_semaphore(1);
    //std::binary_semaphore& dequeueSemaphore = *new std::binary_semaphore(1);
    //std::mutex enqueueMutex, dequeueMutex;
    //int enqueueCnt = 0, dequeueCnt = 0;

    std::mutex enqueueLock, dequeueLock;
    std::mutex cntEqualsOne;
    //it can be released by a thread that didn't acquire it, thats where the bug is. turn it into a mutex

public:
    ConcurrentQueue(){
        // fill in
        tail = nullptr;
        head = nullptr;
    }

    // Enqueues input value at the tail of the list
    void enqueue(int value){
        
        
        // fill in and add the two lines below:
        //sets up new node
        Node* newNode = new Node();
        newNode->next = nullptr;
        newNode->value = value;

        //enqueue lock
        enqueueLock.lock();
        //if queue is of size 1, then both enqueue and dequeue can't happen in parallel
        if (head == tail || head == nullptr)
            cntEqualsOne.lock();
        std::cout << "enqueue" << '\n';
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
        
        cntEqualsOne.unlock();
        enqueueLock.unlock();
        
        
        std::string s = "Enqueued " + std::to_string(value) + ". \n";
        outfile << s;
        std::cout << s;
    }

    // Dequeues value of head of the list, and outputs it
    // Should block if the list is empty
    int dequeue(){
        std::cout << "dequeue start" << '\n';
        
        //fill in and add the two lines below, where returnValue is the output of the dequeue function:

        //only ever tries to get lock if head is not null
        while (!head || !dequeueLock.try_lock())
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        //if queue is of size 1, then both enqueue and dequeue can't happen in parallel
        while ((head == tail || head == nullptr) && !cntEqualsOne.try_lock())
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        std::cout << "dequeue" << '\n';
        //grab node and update head
        Node* tmp = head;
        head = head->next;
        if (!head) tail = head;

        //unlock then delete and return
        cntEqualsOne.unlock();
        dequeueLock.unlock();
        
        int returnValue = tmp->value;
        delete tmp;
        
        std::string s = "Dequeued " + std::to_string(returnValue) + ". \n";
        outfile << s;
        std::cout << s;

        return returnValue;
    }

    // Dequeues value of head of the list, and outputs it
    // Non-blocking, if the list is empty it returns NULL
    int try_dequeue(){
        std::cout << "try dequeue start" << '\n';
        
        // fill in and add the two lines below if successful dequeue, where returnValue is the output of the dequeue function:
        // Else add the two lines below if unsuccessful dequeue:

        //only ever tries to get lock if head is not null
        while (head){
            if (!dequeueLock.try_lock()){
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                std::cout << "here" << "/n";
                continue;
            }
            //if queue is of size 1, then both enqueue and dequeue can't happen in parallel
            if ((head == tail || head == nullptr) && !cntEqualsOne.try_lock()){
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                std::cout << "here2" << "/n";
                continue;
            }
            std::cout << "try dequeue" << '\n';
            //grab node and update head
            Node* tmp = head;
            head = head->next;
            if (!head) tail = head;

            //unlock then delete and return
            cntEqualsOne.unlock();
            dequeueLock.unlock();
            
            int returnValue = tmp->value;
            delete tmp;

            std::string s = "Successfully dequeued " + std::to_string(returnValue) + ". \n";
            outfile << s;
            std::cout << s;

            return returnValue;
        }

        std::string s = "Failed dequeued. \n";
        outfile << s;
        std::cout << s;

        return 0;
    }

    // Looks (but does not remove) at value of head of the list, and outputs it
    // Should block if the list is empty
    int peek(){
        std::cout << "peek start" << '\n';
        // fill in and add the two lines below, where returnValue is the head value (or value that stayed longest in the list):
        
        //only ever tries to get lock if head is not null
        while (!head || !dequeueLock.try_lock())
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        std::cout << "peek" << '\n';
        int returnValue = head->value;
        dequeueLock.unlock();

        std::string s = "Peeked " + std::to_string(returnValue) + ". \n";
        outfile << s;
        std::cout << s;

        return returnValue;
    }

    // Looks (but does not remove) at value of head of the list, and outputs it
    // Non-blocking, if the list is empty it returns NULL
    int try_peek(){
        std::cout << "try peek start" << '\n';
        // fill in and add the two lines below if successful peek, where returnValue is the head value (or value that stayed longest in the list):
        // Else add the two lines below if unsuccessful peek:

        //only ever tries to get lock if head is not null
        while (head){
            if (!dequeueLock.try_lock()){
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
                continue;
            }
            std::cout << "try peek" << '\n';
            int returnValue = head->value;
            dequeueLock.unlock();

            std::string s = "Successfully peeked " + std::to_string(returnValue) + ". \n";
            outfile << s;
            std::cout << s;

            return returnValue;
        }

        std::string s = "Failed peek. \n";
        outfile << s;
        std::cout << s;
        

        return 0;
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
    ConcurrentQueue list;

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
