#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_COUNT 1024l*1024l*1024l*2l

int main(int argc, char ** argv){

    	pthread_spinlock_t * sl1;
	int rv,fd;
	long * count;
	short rv_short;
	int i,x=0;
	void * map_region;
	char file[1024];

        snprintf(file, 1024, "/%s",argv[1]);

        printf("[LOCK] locking on file %s\n", file);

        if ((fd=shm_open(file, O_CREAT|O_RDWR|O_EXCL, S_IREAD | S_IWRITE)) > 0){
                printf("[LOCK] first\n");
        }
        else if ((fd=shm_open(file, O_CREAT|O_RDWR, S_IREAD | S_IWRITE)) > 0){
                printf("[LOCK] second\n");
        } else {
                fprintf(stderr, "ERROR: cannot open file\n");
                exit(-1);
        }

        if ((map_region=mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))<0){
                fprintf(stderr, "ERROR: cannot mmap file\n");
        } else {
                printf("[LOCK] mapped to %p\n", map_region);
        }

	sl1=map_region;
	count=map_region + sizeof(pthread_spinlock_t);

	/* the host initializes */
	*count =0;
	printf("sleeping for 5 seconds...\n");
	sleep(5);
	printf("begin locking (0x%x)\n", sl1);
	pthread_spin_init(sl1, PTHREAD_PROCESS_SHARED);

	/* this would lock the lock */

	while (*count<MAX_COUNT){
		int r=0;

		while (r<100l*1024l*1024l){
			rv=pthread_spin_lock(sl1);
			*count=*count+1;
			rv=pthread_spin_unlock(sl1);
			r++;
		}

		printf("count is %ld\n", *count);
	}

	/* close the file and unmap the region */
	pthread_spin_destroy(sl1);
	munmap(map_region,1024);
	close(fd);
/*	
	rv=pthread_spin_trylock(sl1);
	printf("retval is %d\n", rv);

	rv=pthread_spin_lock(sl1);
	printf("retval is %d\n", rv); 
*/

/*	rv=pthread_spin_lock(sl1);
	printf("retval is %d\n", rv);

	rv=pthread_spin_lock(sl1);
	printf("retval is %d\n", rv);
*/
}
