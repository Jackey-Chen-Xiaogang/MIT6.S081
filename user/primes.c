#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define pid_t int
int l = sizeof(int);

void sieve(int read_fd){
	int primes;
	if (read(read_fd, &primes, l) == 0){
		return;
	}
	printf("prime %d\n", primes);
	int pipefd[2];
	if (pipe(pipefd) < 0){
         	fprintf(2, "Pipe created failed\n");
 	        exit(1);
        }
	pid_t pid = fork();
    if (pid < 0){
                fprintf(2, "Fork failed\n");
                exit(1);
	}
	if (pid == 0){
		close(read_fd);
		close(pipefd[1]);
		sieve(pipefd[0]);
	}else{
		close(pipefd[0]);
		int num;
		while (read(read_fd, &num, l) > 0){
			if (num % primes == 0){
				continue;
			}
			write(pipefd[1], &num, l);
		}
		close(read_fd);
		close(pipefd[1]);
		wait(0);
	}
}

int main(){
	int pipef[2];
	if (pipe(pipef) < 0){
		fprintf(2, "Pipe created failed\n");
		exit(1);
	}
	pid_t pid = fork();
	if (pid < 0){
		fprintf(2, "Fork failed\n");
		exit(1);
	}else if(pid == 0){
		sieve(pipef[0]);
	}
	else if (pid > 0){
		close(pipef[0]);
		for (int i = 2; i <= 35; i++){
			write(pipef[1],&i, l);
		}
	}
	exit(0);
}
