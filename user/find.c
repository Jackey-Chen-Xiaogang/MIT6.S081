#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *name){
    int fd;
    char buf[512];
    char *p;
    struct dirent de;
    struct stat st;
    memmove(buf, path, strlen(path));
    p = buf + strlen(path);
    *p++ = '/';
    if ((fd = open(path, 0)) < 0){
        fprintf(2, "find: Can't open %s\n", path);
        exit(1);
    }

    if (fstat(fd, &st) < 0){
        fprintf(2, "find: can't stat %s\n", path);
        close(fd);
        exit(1);
    }
    
    if (st.type != T_DIR){
        fprintf(2, "find: dir needed\n");
        close(fd);
        exit(1);
    }

    while (read(fd, &de, sizeof(de)) == sizeof(de)){
        if (de.inum == 0){
            continue;
        }
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0){
            printf("ls: cannot stat %s\n", buf);
            continue;
        }
        switch (st.type){
            case T_DIR:
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                    continue;
                }
                find(buf, name);
                break;
            case T_FILE:
                if (strcmp(de.name, name) == 0){
                    printf("%s\n", buf);
                }
                break;
        }
    }
}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Two arguments needed\n");
        exit(1);
    }
    find (argv[1], argv[2]);
    exit(0);
}