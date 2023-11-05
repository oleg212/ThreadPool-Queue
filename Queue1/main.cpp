

#include <iostream>
#include "Queue.h"
#include <string>
#include <thread>
#include <sstream>
#include "ThreadPool.h"
//#include "Synchapi.h"

Queue<std::string> results_queue;

void thread_func() {

    std::ostringstream ss;
    ss << std::this_thread::get_id();
    std::string idstr = "thread "+ ss.str() + ": returned some result";
    results_queue.push(idstr);
    
}



int main()
{

    ThreadPool pool(4);
    for (int i = 0; i < 32; i++)
        pool.add_task(thread_func);


    pool.init();
    pool.wait_all();
    while (!results_queue.isempty()) {
        std::cout << results_queue.pop()<<"\n";
    }
    for (int i = 0; i < 32; i++)
        pool.add_task(thread_func);

    pool.wait_all();
    while (!results_queue.isempty()) {
        std::cout << results_queue.pop() << "\n";
    }
    return 0;
}


