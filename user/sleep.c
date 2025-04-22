#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int main(int argc, const char *argv[]){\
	if (argc < 2){
		printf ("Argument needed\n");
		exit(0);
	}
	int time = atoi(argv[1]);
	sleep (time);
	exit(0);
}
