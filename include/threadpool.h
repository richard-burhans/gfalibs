#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <condition_variable>
#include "memory.h"

extern Log lg;

template<class T>
class ThreadPool {
private:
    int maxThreads;
    std::vector<std::thread> threads;
    std::vector<bool> threadStates;
    std::queue<T> jobs;
    std::mutex queueMutex;
    std::condition_variable mutexCondition;
    bool done = false;

    void threadLoop(int threadN);

public:
    void init(int maxThreads);
    void queueJob(const T& job);
    bool empty();
    bool jobsDone();
    short unsigned int running();
    unsigned int queueSize();
    void join();
    short unsigned int totalThreads();
    void execJob();
    
};

template<class T>
void ThreadPool<T>::threadLoop(int threadN) {
    
    T job;
    
    while (true) {
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
#ifdef DEBUG
            std::cout<<"Thread "<<std::to_string(threadN)<<" waiting"<<std::endl;
#endif
            threadStates[threadN] = true;
            
            mutexCondition.wait(lock, [this] {
                return !jobs.empty() || done;
            });
            if (done) {
                return;
            }
            
            threadStates[threadN] = false;
            
            job = jobs.front();
            jobs.pop();
            
        }
        job();
#ifdef DEBUG
        std::cout<<"Thread "<<std::to_string(threadN)<<" done (thread state: "<<threadStates[threadN]<<")"<<std::endl;
#endif

    }
}

template<class T>
void ThreadPool<T>::init(int maxThreads) {
    
    if(maxThreads == 0) maxThreads = std::thread::hardware_concurrency();
    if(maxThreads == 0 || maxThreads == 1) maxThreads = 2;
    threads.resize(maxThreads-1);
    threadStates.resize(maxThreads-1);
    
    lg.verbose("Generating threadpool with " + std::to_string(maxThreads-1) + " threads");
    
    for(int i=0; i<maxThreads-1; ++i) {
        threads[i] = std::thread(&ThreadPool::threadLoop, this, i);
        threadStates[i] = true;
    }
    this->maxThreads = maxThreads-1;
    done = false;
}

template<class T>
void ThreadPool<T>::queueJob(const T& job) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        jobs.push(job);
    }
    mutexCondition.notify_one();
}

template<class T>
bool ThreadPool<T>::empty() {return jobs.empty();}

template<class T>
unsigned int ThreadPool<T>::queueSize() {return jobs.size();}

template<class T>
bool ThreadPool<T>::jobsDone() {
    
    for(bool isDone : threadStates) {
#ifdef DEBUG
        std::cout<<(isDone == true ? "done" : "not done")<<std::endl;
#endif
        if (!isDone)
            return false;
    }
    
    return true;

}

template<class T>
short unsigned int ThreadPool<T>::running() {
    
    short unsigned int jobN = 0;
    
    for(bool isDone : threadStates) {
        if (!isDone)
            ++jobN;
    }
    
    return jobN;

}

template<class T>
void ThreadPool<T>::join() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        done = true;
    }
    mutexCondition.notify_all();
    for(std::thread& activeThread : threads) {
        activeThread.join();
    }
    threads.clear();
}

template<class T>
short unsigned int ThreadPool<T>::totalThreads() {
    return maxThreads;
}

template<class T>
void ThreadPool<T>::execJob() {
    
    T job;
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (!jobs.empty()) {
            job = jobs.front();
            jobs.pop();
        }else{return;}
    }
    job();

}

template<class T>
void jobWait(ThreadPool<T>& threadPool) {
    
    double mem_usage, mem_total;
    
    while (true) {
        
        if (threadPool.empty() && threadPool.jobsDone()) {
            
            lg.newlines(2);
            break;
            
        }
        
        threadPool.execJob(); // have the master thread contribute
        
        mem_usage = get_mem_usage(3);
        mem_total = get_mem_total(3);
        lg.verbose("Jobs waiting/running: " + std::to_string(threadPool.queueSize()) + "/" + std::to_string(threadPool.running()) + " memory used/total: " + std::to_string(mem_usage) + "/" + std::to_string(mem_total) + " " + memUnit[3], true);
        
    }
    
}

#endif //THREADPOOL
