//
//  utils.h
//  Fractal Mandelbrot
//
//  Created by Renner, Julio on 18/06/20.
//  Copyright © 2020 Renner, Julio. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include <stdio.h>
#include <pthread.h>
#include <queue>

const int MAX_SIZE = 2;

class Work {
    public:
        int xi, yi;
        int xf, yf;

        Work(int xInit,int yInit,int xEnd,int yEnd) {
            xi = xInit; yi = yInit;
            xf = xEnd; yf = yEnd;
        }
};

class WorkResult {
    private:
        std::queue<int> data;
    public:
        int xi, yi;
        int xf, yf;

        WorkResult(int xInit,int yInit,int xEnd,int yEnd) {
            xi = xInit; yi = yInit;
            xf = xEnd; yf = yEnd;
        }
    
        void enqueue(int v) {
            data.push(v);
        }
        
        int dequeue() {
            int v = data.front();
            data.pop();
            return v;
        }
};

class WorkResultQueue {
    private:
        std::queue<WorkResult> workResult;
        int maxSize = MAX_SIZE;
        int finishedThreads = 0;
    
    public:
        int threadsNumber = 0;
        pthread_mutex_t *mutexThreads;
        pthread_mutex_t *mutex;
        pthread_cond_t *condition_not_empty, *condition_not_full;
        
        WorkResultQueue() {
            mutex = (pthread_mutex_t *) malloc(sizeof (pthread_mutex_t));
            mutexThreads = (pthread_mutex_t *) malloc(sizeof (pthread_mutex_t));
            condition_not_full = (pthread_cond_t *) malloc(sizeof (pthread_cond_t));
            condition_not_empty = (pthread_cond_t *) malloc(sizeof (pthread_cond_t));
            pthread_mutex_init(mutex, NULL);
            pthread_mutex_init(mutexThreads, NULL);
            pthread_cond_init(condition_not_full, NULL);
            pthread_cond_init(condition_not_empty, NULL);
        }
    
        bool isFinished() {
            return finishedThreads == threadsNumber;
        }
    
        void threadFinished() {
            pthread_mutex_lock(mutexThreads);
            finishedThreads += 1;
            pthread_mutex_unlock(mutexThreads);
        }
    
        bool isFull() {
            return workResult.size() >= maxSize;
        }
    
        bool isEmpty() {
            return workResult.empty();
        }
    
        void enqueue(WorkResult w)
        {
            pthread_mutex_lock(mutex);
            while(isFull())
                pthread_cond_wait(condition_not_full, mutex);
            
            workResult.push(w);
            
            pthread_mutex_unlock(mutex);
            pthread_cond_signal(condition_not_empty);
        }
    
        WorkResult dequeue() {
            pthread_mutex_lock(mutex);
            while (isEmpty())
                pthread_cond_wait(condition_not_empty, mutex);
            
            WorkResult w = workResult.front();
            workResult.pop();
            
            pthread_mutex_unlock(mutex);
            pthread_cond_signal(condition_not_full);
            
            return w;
        }
};


class WorkQueue {
    private:
        std::queue<Work> works;
        int maxSize = MAX_SIZE;
    
    public:
        bool isFinished = false;
        pthread_mutex_t *mutex;
        pthread_cond_t *condition_not_empty, *condition_not_full;
    
        WorkQueue() {
            mutex = (pthread_mutex_t *) malloc(sizeof (pthread_mutex_t));
            condition_not_full = (pthread_cond_t *) malloc(sizeof (pthread_cond_t));
            condition_not_empty = (pthread_cond_t *) malloc(sizeof (pthread_cond_t));
            pthread_mutex_init(mutex, NULL);
            pthread_cond_init(condition_not_full, NULL);
            pthread_cond_init(condition_not_empty, NULL);
        }
    
        bool isFull() {
            return works.size() >= maxSize;
        }
    
        bool isEmpty() {
            return works.empty();
        }
    
        void enqueue(Work w)
        {
            pthread_mutex_lock(mutex);
            while(isFull())
                pthread_cond_wait(condition_not_full, mutex);
            works.push(w);
            pthread_mutex_unlock(mutex);
            pthread_cond_signal(condition_not_empty);
        }
    
        Work dequeue() {
            pthread_mutex_lock(mutex);
            while (isEmpty())
                pthread_cond_wait(condition_not_empty, mutex);
            Work w = works.front();
            works.pop();
            pthread_mutex_unlock(mutex);
            pthread_cond_signal(condition_not_full);
            
            return w;
        }
};

#endif /* utils_h */
