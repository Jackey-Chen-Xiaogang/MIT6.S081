#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX 1024

int readline(char *line){
    char c;
    int i = 0;
    while (read(0, &c, 1) == 1){
        if (c == '\n'){
            line[i] = '\0';
            return i;
        }
        line[i++] = c;
    }
    if (i > 0){
        line[i] = '\0';
        return i;
    }
    return 0;
}

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(2, "Xargs: 2 arguments needed");
        exit(1);
    }
    char *new_argv[MAX];
    char line[MAX];
    int i;
    for (i = 1; i < argc; i++){
        new_argv[i - 1] = argv[i];
    }
    while (readline(line) != 0){
        new_argv[i - 1] = line;
        int pid = fork();
        if (pid < 0){
            fprintf(2, "fork failed\n");
            exit(1);
        }
        if (pid == 0){
            if(exec(new_argv[0], new_argv) < 0){
                fprintf(2, "Exec failed\n");
                exit(1);
            }
        } else {
            wait(0);
        }
    }
    exit(0);
}