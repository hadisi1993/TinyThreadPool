#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include<pthread.h>

typedef struct Task{  // 定义任务
	void(*function)(void*arg);
	void*arg;	
}Task;

typedef struct threadPool{
	
	int threadNum;      	 // 线程的数量
	int maxTaskNum;     	 // 最多任务的数量
	int taskNum;
	Task*taskQueue;	    	 // 任务数组
	int queueFront;     	 // 队列头部
	int queueRear;      	 // 队列尾部
	pthread_t*threads;  	 // 线程数组

	pthread_mutex_t locker;  // 锁
        pthread_cond_t full;     // 信号量: 任务数量
	pthread_cond_t empty;    // 信号量: 可放认为数量
	
	int refuse;              // 是否拒绝接受任务
	int shutdown;            // 是否关闭线程池
}threadPool;

int threadPoolCreate(threadPool**poolAddr,int threadNum,int maxTaskNum);  // 创建线程池
int threadPoolDestroy(threadPool*pool);				     // 销毁线程池
int threadPoolTaskAdd(threadPool*pool,Task task);                    // 添加任务
static void*worker(void*arg);                                               // 任务函数
#endif
