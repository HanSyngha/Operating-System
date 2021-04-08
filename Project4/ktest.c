#include "types.h"
#include "user.h"
#include "param.h"
#include "fcntl.h"

int main(int argc, char**argv){
	printf(1,"ktest\n");
	int num1;
	int num2;
	for (int i=0;i<55;i++){
		printf(1,"%d\n",i);
		if (i == 49){
			swapstat(&num1,&num2);
			printf(1,"read write %d %d\n",num1,num2);
		}
		if (fork()==0){
			sleep(1000000);
		}
	}
	while(1);
	printf(1,"ktest end\n");
	exit();
}
