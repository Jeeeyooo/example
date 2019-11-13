#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mqueue.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NAME "/mq_name"
#define MSG_SIZE 14


void msg_send(char* mq_name, int n);
void msg_receive(char* mq_name, int n);

int main() {
	pid_t ppid = getpid();
	pid_t pid = fork();

	int n = 10;
	char mq_name[10];
	sprintf(mq_name, "%s%d", NAME, 0);		

	// child process
	if(pid == 0) {
		msg_send(mq_name, n);
		printf("child : child process will be exit.\n");
		exit(1);	
	}

	// parent process
	else {
		msg_receive(mq_name, n);
		printf("parent : waiting for child process (%d)\n", pid);
		int state;
		if(pid == waitpid(pid, &state, 0)) {
			printf("parent : child process was done. (state : %d)\n", state);
		}
	}

	return 0;
}

void msg_receive(char* mq_name, int n) {
	struct mq_attr attr;
	mqd_t mqdes;
	char msg[MSG_SIZE];
	int num;
	int isEnd = 0;

	attr.mq_maxmsg = n;
	attr.mq_msgsize = MSG_SIZE;

	mqdes = mq_open(mq_name, O_CREAT | O_RDWR, 0666, &attr);
	if(mqdes < 0) {
		perror("mq_open()");
		exit(6);
	}

	while(mq_receive(mqdes, msg, MSG_SIZE, NULL) < 0 || !isEnd) {
		printf("receive %s\n", msg);
		if(strcmp(msg,"-") == 0) { 
			isEnd = 1;
			break;
		}
	}

	mq_close(mqdes);
	mq_unlink(mq_name);
	printf("receive end\n");
}

void msg_send(char* mq_name, int n) {
	struct mq_attr attr;
	mqd_t mqdes;
	char msg[MSG_SIZE];
	int num;
	
	attr.mq_maxmsg = n;
	attr.mq_msgsize = MSG_SIZE;
	attr.mq_flags = O_NONBLOCK;

	srand(time(NULL));
	for(int i = 0 ; i < n ; i++) {
		num = rand()%100;
		mqdes = mq_open(mq_name, O_CREAT | O_RDWR, 0666, &attr);
		if(mqdes < 0) {
			perror("mq_open()");
			exit(6);
		}
		sprintf(msg, "%d", num);
		if(mq_send(mqdes, msg, MSG_SIZE, 1) == -1)
			perror("mq_send()");
		printf("send %d\n", num);
	}

	sprintf(msg, "-");
	if(mq_send(mqdes, msg, MSG_SIZE, 1) == -1)
		perror("mq_send()");
}
