#include <mutex>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <semaphore>
#include <cstdlib>

static std::ofstream outfile("log.txt");

//std::counting_semaphore<1> cs(1);
//std::binary_semaphore bs(1);
//std::mutex mu;

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

private:
    void printQueue(){
        for (Node* p = head; p != nullptr; p = p->next)
            std::cout << p->value << '\n';
        std::cout << std::endl;
    }

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

        printQueue();

        // std::string s = "Enqueued " + std::to_string(value) + ". \n";
        // outfile << s;
    }

    // Dequeues value of head of the list, and outputs it
    // Should block if the list is empty
    int dequeue(){
        // fill in and add the two lines below, where returnValue is the output of the dequeue function:
        //if queue is not empty
        if (head){
            //grab node and value
            Node* tmp = head;
            int val = tmp->value;
            
            //set head to next, and tail to head if queue becomes empty
            head = head->next;
            if (!head) tail = head;

            printQueue();
            
            //delete and return
            delete tmp;
            return val;
        }

        //todo: remove this in concurrent version
        return (int)NULL;

        // std::string s = "Dequeued " + std::to_string(returnValue) + ". \n";
        // outfile << s;
    }

    // Dequeues value of head of the list, and outputs it
    // Non-blocking, if the list is empty it returns NULL
    int try_dequeue(){
        // fill in and add the two lines below if successful dequeue, where returnValue is the output of the dequeue function:
        //if queue is not empty
        if (head){
            //grab node and value
            Node* tmp = head;
            int val = tmp->value;
            
            //set head to next, and tail to head if queue becomes empty
            head = head->next;
            if (!head) tail = head;
            
            //delete and return
            delete tmp;
            return val;
        }

        return (int)NULL;

        // std::string s = "Successfully dequeued " + std::to_string(returnValue) + ". \n";
        // outfile << s;
        // Else add the two lines below if unsuccessful dequeue:
        // std::string s = "Failed dequeued. \n";
        // outfile << s;
    }

    // Looks (but does not remove) at value of head of the list, and outputs it
    // Should block if the list is empty
    int peek(){
        // fill in and add the two lines below, where returnValue is the head value (or value that stayed longest in the list):
        if (head){
            return head->value;
        }

        //todo: remove this in concurrent version
        return (int)NULL;

        // std::string s = "Peeked " + std::to_string(returnValue) + ". \n";
        // outfile << s;
    }

    // Looks (but does not remove) at value of head of the list, and outputs it
    // Non-blocking, if the list is empty it returns NULL
    int try_peek(){
        // fill in and add the two lines below if successful peek, where returnValue is the head value (or value that stayed longest in the list):
        if (head){
            return head->value;
        }
        
        return (int)NULL;

        // std::string s = "Successfully peeked " + std::to_string(returnValue) + ". \n";
        // outfile << s;
        // Else add the two lines below if unsuccessful peek:
        // std::string s = "Failed peek. \n";
        // outfile << s;
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
