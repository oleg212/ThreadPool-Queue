#pragma once
#include <thread>
#include <vector>
#include <functional>
#include "Queue.h"
#include <typeinfo>
#include <unordered_set>


class ThreadPool {
    Queue<std::pair<std::function<void()>, unsigned long>*> TaskQueue; //Потокобезопасная очередь
    std::condition_variable queue_cv;

    std::unordered_set<unsigned long> completed_task;
    std::mutex completed_task_lock;
    std::condition_variable completed_task_cv;

    std::vector<std::thread> threads;
    std::atomic<bool> quite;
    std::atomic<unsigned long> last_idx = 0;                            //next task id

    std::mutex P_lock;                                                  //Pool lock
    unsigned long num_threads;

    void run() {
        while (!quite) {
            std::unique_lock<std::mutex> lock(P_lock);
            queue_cv.wait(lock, [this]()->bool { return !TaskQueue.isempty() || quite; });
            if (!TaskQueue.isempty()) {
                auto elem = TaskQueue.pop();
                lock.unlock();
                auto task = elem->first;
                task();
                std::lock_guard<std::mutex> lock(completed_task_lock);
                completed_task.insert(elem->second);
                completed_task_cv.notify_all();
            }
        }
        return;
    }
    void operator=(const ThreadPool&) ; //copy, move, assignment prohibited!
    ThreadPool(const  ThreadPool&) ;
    ThreadPool(ThreadPool&&);
    ThreadPool operator=(const ThreadPool&&);
public:



    ThreadPool(unsigned long _num_threads): num_threads(_num_threads) {
        threads.reserve(num_threads);        
    }



    ~ThreadPool() {
        wait_all();
        quite = true;
        for (unsigned long  i = 0; i < num_threads; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            queue_cv.notify_all();
            threads[i].join();
        }
    }

    void init() {
        for (unsigned long i = 0; i < num_threads; ++i) {
            threads.emplace_back(&ThreadPool::run, this);
        }
    }

    template <typename Func, typename ...Args>
    unsigned long add_task(const Func& task_func, Args&&... args) {
        unsigned long task_idx = last_idx++;                            //next task id

        std::lock_guard<std::mutex> q_lock(P_lock);
        auto pair= new std::pair<std::function<void()>, unsigned long>(std::bind(std::forward<Func>(task_func), std::forward<Args>(args)...), task_idx);
        TaskQueue.push(pair);                                           //add task to queue
        queue_cv.notify_one();                                          //wake up one thread if exist
        return task_idx;
    }

    void wait_all() {
        std::unique_lock<std::mutex> lock(P_lock);       
        completed_task_cv.wait(lock, [this]()->bool {                    // wait for notify from run()
            std::lock_guard<std::mutex> task_lock(completed_task_lock);
            return TaskQueue.isempty() && last_idx == completed_task.size();
            });
    }
};