#include "types.h"
#include "user.h"
#include "param.h"
#include "fcntl.h"

int main(int argc, char** argv) {
	printf(1, "mmap test \n");
	int i;
	int size = 8192;
	int fd = open("README", O_RDWR);
	char* text = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);							  //File example
	char* text2 = mmap(0, size, PROT_WRITE|PROT_READ, MAP_POPULATE|MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);		  //ANONYMOUS example
	//char* text = mmap((void *)4096, size, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_POPULATE|MAP_FIXED, fd, 0); // FIXED example
	//char* text2 = mmap((void *)8192, size, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_POPULATE|MAP_FIXED, fd, 0); // FIXED example(error with overlapped area)
	for (i = 0; i < size; i++) 
		printf(1, "%c", text[i]);
	text[8000] = 'Y';
	printf(1,"\n============file mmap end==========\n\n\n\n");

	text2[0] = 's';
	text2[4096] = 'Y';
	for (i = 0; i < size; i++) 
		printf(1, "%c", text2[i]);
	printf(1,"\n============anonymous mmap end==========\n");

	munmap(text);
	munmap(text2);

	exit();
}
