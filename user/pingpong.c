#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define pid_t int
#define buffer_size 100

int main(){
	int pipefd1[2];
	int pipefd2[2];
	char buffer1[buffer_size];
	char buffer2[buffer_size];
	const char *mess = "Hello";
	int length = strlen(mess);
	pid_t pid;
	if (pipe(pipefd1) < 0 || pipe(pipefd2) < 0){
		fprintf(2, "Pipe created failed\n");
		exit(1);
	}
	pid = fork();
	if (pid < 0){
		fprintf(2, "Fork failed\n");
		exit(1);
	}
	if (pid == 0){
		close(pipefd1[1]);
		close(pipefd2[0]);
		write(pipefd2[1], mess, length);
		int n1 = read(pipefd1[0], buffer1, buffer_size);
		buffer1[n1] = '\0';
		if (strcmp(mess, buffer1)){
			printf("%d: receieved ping\n", pid);
		}
	}
	else {
		close(pipefd1[0]);
		close(pipefd2[1]);
		write(pipefd2[0], mess, length);
		int n2 = read(pipefd1[1], buffer2, buffer_size);
		buffer2[n2] = '\0';
		if (strcmp(mess, buffer2)){
			printf("%d: receieved: pong\n", pid);
		}
	}
	exit(0);
}
