//
//  main.cpp
//  Fractal Mandelbrot
//
//  Created by Renner, Julio on 18/06/20.
//  Copyright © 2020 Renner, Julio. All rights reserved.
//
  
#include <iostream>
#include <fstream>
#include <complex>
#include <cmath>
#include <queue>

#include <pthread.h>
#include "utils.h"

using namespace std;

const float width = 600;
const float height = 600;

const int grain_width = 50;
const int grain_height = 50;

int iterations;
int workerThreadsNumber;

WorkQueue workQueue;
WorkResultQueue workResultQueue;

int finalImage [int(width)][int(height)];

ofstream my_Image;

void createImage(string imageName) {
    my_Image = ofstream(imageName + ".ppm");
    if (my_Image.is_open ()) {
        my_Image << "P3\n" << width << " " << height << " 255\n";
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++)  {
                int val = finalImage[i][j];
                my_Image << val << ' ' << 0 << ' ' << val << "\n";
            }
        }
        my_Image.close();
    } else {
        cout << "Could not open the file";
        exit(1);
    }
}

int mandelbrot (int x, int y)  {
    complex<float> point((float)x/width-1.5, (float)y/height-0.5);
    complex<float> z(0, 0);
    int nb_iter = 0;
    while (abs (z) < 2 && nb_iter <= iterations) {
        z = z * z + point;
        nb_iter++;
    }
    if (nb_iter < iterations)
        return (255*nb_iter)/iterations;
    else
       return 1;
}

void createWork() {
    const int horizontal_chunks = width/grain_width;
    const int vertical_chunks = height/grain_height;

    for(int j = 0; j < vertical_chunks; j++) {
        for(int i = 0; i < horizontal_chunks; i++) {
            int xi = i * grain_width;
            int xf = ((i + 1) * grain_width) - 1;
            
            int yi = j * grain_height;
            int yf = ((j + 1) * grain_height) - 1;
            
            workQueue.enqueue(Work(xi, yi, xf, yf));
        }
    }
    workQueue.isFinished = true;
}

void *worker(void *data) {
    while(!workQueue.isFinished or !workQueue.isEmpty()) {
        Work task = workQueue.dequeue();
        
        WorkResult result = WorkResult(task.xi, task.yi, task.xf, task.yf);
        
        for (int i = task.xi; i <= task.xf; i++) {
            for (int j = task.yi; j <= task.yf; j++)  {
                result.enqueue(mandelbrot(i, j));
            };
        }
        
        workResultQueue.enqueue(result);
    }
    
    workResultQueue.threadFinished();
    
    return NULL;
}

void *consumer(void *data) {
    string *imageName = (string *) data;
    while(!workResultQueue.isFinished() or !workResultQueue.isEmpty()) {
        WorkResult task = workResultQueue.dequeue();
        
        for (int i = task.xi; i <= task.xf; i++) {
            for (int j = task.yi; j <= task.yf; j++)  {
                finalImage[i][j] = task.dequeue();
            }
        }
    }
    
    createImage(*imageName);
    
    return NULL;
}


int main (int argc, char *argv[])  {
    string imageName;
    if (argc == 1) {
        workerThreadsNumber = 4;
        iterations = 20;
        imageName = "mandelbrot";
    } else {
        workerThreadsNumber = atof(argv[1]);
        iterations = atof(argv[2]);
        imageName = argv[3];
    }
    
    workResultQueue.threadsNumber = workerThreadsNumber;
    
    pthread_t workerThreads[workerThreadsNumber];
    pthread_t consumerThread;
    
    pthread_create(&consumerThread, NULL, consumer, &imageName);
    
    for (int i = 0; i < workerThreadsNumber; i++) {
      pthread_create(&workerThreads[i], NULL, worker, NULL);
    }
    
    createWork();
    
    for (int i = 0; i < workerThreadsNumber; i++) {
      pthread_join(workerThreads[i], NULL);
    }
    
    pthread_join(consumerThread, NULL);
    
    return 0;
}
