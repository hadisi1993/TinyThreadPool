#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"threadpool.h"

int threadPoolCreate(threadPool**poolAddr,int threadNum,int maxTaskNum){

	int code = 0;  // 状态码
	if(threadNum<0||maxTaskNum<0){
		perror("arg Error! threadNum and maxTaskNum should more than zero\n");
		return -1;
	}

	threadPool*pool = NULL;
	do{
		// 创建线程池
		pool = (threadPool*)malloc(sizeof(threadPool));
		if(pool == NULL){
			perror("malloc threadPool failed...\n");
			code = -2;
			break;
		}
		
		// 创建任务队列	
		
		pool->taskQueue = (Task*)malloc(sizeof(Task)*maxTaskNum);
		if(pool->taskQueue==NULL){
			perror("malloc taskQueue failed...\n");
			code = -3;
			break;
		}
	
		pool->queueFront = 0;
		pool->queueRear = 0;
		pool->maxTaskNum = maxTaskNum;
		pool->taskNum = 0;
	
		// 创建线程数组
		pool->threadNum = threadNum;
		pool->threads = (pthread_t*)malloc(sizeof(pthread_t)*threadNum);
	
		if(pool->threads == NULL){
			perror("malloc threads failed...\n");
			code = -4;
			break;
		}
	
		pool->shutdown = 0;
		pool->refuse = 0;
		memset(pool->threads,0,sizeof(pthread_t)*threadNum);
	
		// 初始化锁和条件变量
		if(pthread_mutex_init(&pool->locker,NULL)!=0|| pthread_cond_init(&pool->full,NULL)!=0||pthread_cond_init(&pool->empty,NULL)!=0){
			perror("mutex or condition init fail...\n");
			code = -5;
			break;
		}
		for(int i=0;i<threadNum;i++){
	
			pthread_create(pool->threads+i,NULL,worker,pool);
			pthread_detach(pool->threads[i]);  // 分离线程
		}
       	}while(0);
	
    if(code!=0){
	    if(pool&&pool->taskQueue) free(pool->taskQueue);
	    if(pool&&pool->threads) free(pool->threads);
	    if(pool) free(pool);
	    pool = NULL;
    }
    *poolAddr = pool;
    return code;       // 创建成功则返回0,否则返回错误的状态码
}
	
int threadPoolDestroy(threadPool*pool){
	
	if(pool==NULL){
		perror("destroy theadpool failed...\n");
		return -1;
	}

	// 拒绝接受任务
	pool->refuse = 1;
	while(pool->taskNum);
	// 关闭线程池
	pool->shutdown = 1;

	// 唤醒所有的线程,使线程自动退出
	for(int i=0;i<pool->threadNum;i++){
		pthread_cond_signal(&pool->full);
	}
	// 释放任务队列和线程数组
	if(pool->taskQueue){
		free(pool->taskQueue);
	}

	if(pool->threads){
		free(pool->threads);
	}

	// 销毁锁和条件变量
	pthread_mutex_destroy(&pool->locker);
	pthread_cond_destroy(&pool->full);
	pthread_cond_destroy(&pool->empty);
	pool = NULL;

	// 释放线程池
	free(pool);
	pool = NULL;
	return 0;
}

int threadPoolTaskAdd(threadPool*pool,Task task){

	if(pool==NULL){
		perror("Add task failed...\n");	
		return -1;
	}
	pthread_mutex_lock(&pool->locker);
	while(pool->taskNum==pool->maxTaskNum&&!pool->shutdown){
		pthread_cond_wait(&pool->empty,&pool->locker);
	}
	if(pool->refuse){  // 线程池停止添加任务
		pthread_mutex_unlock(&pool->locker);
		return -1;
	}
	pool->taskQueue[pool->queueRear] = task;
	pool->queueRear = (pool->queueRear+1)%pool->maxTaskNum;
	pool->taskNum++;

	pthread_cond_signal(&pool->full);
	pthread_mutex_unlock(&pool->locker);
	return 0;
}

static void*worker(void*arg){

	threadPool*pool = (threadPool*)arg;	
	while(1){

		pthread_mutex_lock(&pool->locker);
		while(pool->taskNum==0&&!pool->refuse){
			pthread_cond_wait(&pool->full,&pool->locker);
		}
		// 若线程池已关闭,则退出
		if(pool->shutdown == 1){
		      pthread_mutex_unlock(&pool->locker);
		      printf("thread %ld exit...\n",pthread_self());
		      pthread_exit(NULL);
		}
		// 取出一个任务执行
		Task task = pool->taskQueue[pool->queueFront];
		pool->queueFront = (pool->queueFront+1)%pool->maxTaskNum;
		pool->taskNum--;
		pthread_cond_signal(&pool->empty);
		pthread_mutex_unlock(&pool->locker);
		// 执行任务
		printf("thread %ld start working...\n",pthread_self());
		task.function(task.arg);
		printf("thread %ld end working...\n",pthread_self());
		free(task.arg);
		task.arg = NULL;
	}
	return NULL;
}
