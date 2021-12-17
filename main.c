#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"threadpool.h"
//任务函数,打印数字
void taskFunc(void*arg){
	int num = *(int*)arg;
	printf("thread: %ld    number: %d\n",pthread_self(),num);
	sleep(1);
}

int main(){
	
	threadPool*pool = NULL;
	// 创建线程池
	if(threadPoolCreate(&pool,5,50)!=0){
		exit(1);
	}
	for(int i=0;i<100;i++){
		int*num = (int*)malloc(sizeof(int));
		*num = i;
		Task task = {taskFunc,num};
		if(threadPoolTaskAdd(pool,task)!=0){
			break;
		}
	}
	printf("All task appended to the threadPool...\n");
	sleep(15);
	// 销毁线程池
	threadPoolDestroy(pool);
	return 0;
}
