#include "types.h"
#include "stat.h"
#include "user.h"


void testcfs()
{
	int parent = getpid();
	int child;
	int i;
	double x = 0, z;
	
	if((child = fork()) == 0) { // child
		if(setnice(parent, 5) == -1){
			printf(1,"parent setnice failed\n");
			exit();
		};		// if you set parent's priority lower than child, 
								// 2nd ps will only printout parent process,
								// since child finished its job earlier & exit
		for(i = 0; i < 3000; i++){
			for ( z = 0; z < 30000.0; z += 0.1 )
				x =  x + 3.14 * 89.64;
		}
		ps();
		exit();
	} else {	
		if (setnice(child, -5) == -1){
			printf(1,"child setnice failed\n");
			exit();
		};	  //parent
		for(i = 0; i < 3000; i++){
			for ( z = 0; z < 30000.0; z += 0.1 )
				x =  x + 3.14 * 89.64;
		}
		ps();
		wait();
	}
}

int main(int argc, char **argv)
{
		printf(1, "=== TEST START ===\n");
		testcfs();
		printf(1, "=== TEST   END ===\n");
		exit();

}

