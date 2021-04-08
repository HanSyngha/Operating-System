#include "types.h"
#include "user.h"
#include "param.h"
#include "fcntl.h"

int main(int argc, char** argv) {
	int i;
	printf(1, "mmap test \n");
	printf(1, "initial freemem is %d\n",freemem());
	int fd = open("README", O_RDWR);
	printf(1, "README opened\n");
	char* text = mmap(0, 4096, PROT_READ, MAP_PRIVATE, fd, 0);							  //File example
	printf(1, "text mmaped %x\n",(int)text);
	char* text2 = mmap(0, 8192, PROT_WRITE|PROT_READ, MAP_POPULATE|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);		  //ANONYMOUS example
	printf(1, "text2 mmaped %x\n",(int)text2);
	char* text3 = mmap((void *)16384, 4096, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_POPULATE|MAP_FIXED, fd, 0); // FIXED example
	printf(1,"FIXED mapping at 0x4000: %x\n",(int)text3);
	char* text4 = mmap((void *)8192, 4096*2, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_POPULATE|MAP_FIXED, fd, 0); // FIXED example(error with overlapped area)
	printf(1,"FIXED mapping at 0x2000 failed : %x\n",(uint)text4);
	munmap(text2);
	char* text5 = mmap(0, 8192, PROT_WRITE|PROT_READ, MAP_POPULATE|MAP_ANONYMOUS, -1, 0);		  //No private example
	printf(1,"No PRIVATE example : %x\n",(uint)text5);
	if (fork()==0){
		printf(1,"child\n");
		printf(1,"print start\n");
		for (i = 0; i < 4096; i++) 
			printf(1, "%c", text[i]);
		printf(1,"print end\n");
		exit();
	}
	else{
		wait();
	}
	int a = munmap(text2);
	int b = munmap(text3);
	printf(1,"%d %d\n",a,b);
	printf(1,"final freemem() is %d\n",freemem());
	exit();
}
