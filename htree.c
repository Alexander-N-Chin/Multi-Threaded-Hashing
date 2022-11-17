/* Alexander N. Chin | ANC200008
 * CS3377.005
 * 11/12/2022
 * A11: Multi-threaded Hashing
 */

#include <stdio.h>     
#include <stdlib.h>   
#include <stdint.h>  
#include <inttypes.h>  
#include <errno.h>     // for EINTR
#include <fcntl.h>     
#include <unistd.h>    
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>

#define BSIZE 4096
#define MAXLEN 100000



//forward declaration of functions
void usage(char*);
uint32_t jenkins_one_at_a_time_hash(const uint8_t* , uint64_t );
void* createTree(void* arguments);
double GetTime();

// create structure to pass multiple arguments through threads 
struct arg_struct {
	uint8_t *blocks;
	int current;
	uint32_t blocksPerThread;
	int mthreads;
};

int main(int argc, char** argv) 
{
  int32_t fd;
  uint32_t nblocks;

  // input checking 
  if (argc != 3)
    usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }
  
  // use fstat to get file size
  struct stat buf;
  fstat(fd, &buf);
  size_t filesize = buf.st_size;

  // calculate nblocks 
  nblocks = filesize/BSIZE;
  printf("no. of blocks = %u \n", nblocks);
  int mthreads = atoi(argv[2]);
  
  //map each byte of file to an array
  uint8_t *blocks = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
  uint32_t blocksPerThread = nblocks/mthreads;
  printf("no. of threads per block = %u \n", blocksPerThread);
  
  //start clock
  double start = GetTime();

  //calculate hash
  struct arg_struct arguments;
  arguments.blocks = blocks;
  arguments.current = 0;
  arguments.blocksPerThread = blocksPerThread;
  arguments.mthreads = mthreads;
  pthread_t p0;
  pthread_create(&p0, NULL, createTree, &arguments);
  void* hash;
  pthread_join(p0, &hash);

  //end clock
  double end = GetTime();
  printf("hash value = %u \n", hash);
  //printf("time taken = %f \n", (end - start) * 1.0 / CLOCKS_PER_SEC);
  printf("time taken = %f \n", (end - start));
  close(fd);
  return EXIT_SUCCESS;
}

//calculate hashcode 
//params: array of bytes, length of array
//return hashcode
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, uint64_t length) 
{
  uint64_t i = 0;
  uint32_t hash = 0;

  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

//print usage error message
void usage(char* s) 
{
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}

//create the children threads and recursively call them till all threads are created
//calculate the children thread hashcodes then calculate current thread hashcodes
//concatenate all hashcodes and recalulate hashcodes
//params: array of blocks, current index, blocks per thread, and number of threads (wrapped in a struct)
//return final hashcode
void* createTree(void* arguments) {
  struct arg_struct *args = (struct arg_struct *) arguments;
  if (args->current >= args->mthreads) {
	return 0;
  }

  //create children
  //create sub arguments to pass to children threads
  struct arg_struct subArgsL;
  subArgsL.blocks = args->blocks;
  subArgsL.blocksPerThread = args->blocksPerThread;
  subArgsL.mthreads = args->mthreads;

  struct arg_struct subArgsR;
  subArgsR.blocks = args->blocks;
  subArgsR.blocksPerThread = args->blocksPerThread;
  subArgsR.mthreads = args->mthreads;
 
  //create left child
  pthread_t left;
  subArgsL.current = 2*args->current+1;
  pthread_create(&left, NULL, createTree, &subArgsL);

  //create right child
  pthread_t right;
  subArgsR.current = 2*args->current+2;
  pthread_create(&right, NULL, createTree, &subArgsR);
  
  //save hash values
  void* leftHash;
  pthread_join(left, &leftHash);
  void* rightHash;
  pthread_join(right, &rightHash);
  
  //calculate hash value
  char currHash[MAXLEN];
  uint32_t currHashNum = jenkins_one_at_a_time_hash(args->blocks + args->current*(BSIZE*args->blocksPerThread), BSIZE*args->blocksPerThread);
  
  //concatenate all hash values
  if (rightHash != NULL && leftHash != NULL) {
  	sprintf(currHash, "%u%u%u", currHashNum, leftHash, rightHash);	
  } else if (leftHash != NULL) {
  	sprintf(currHash, "%u%u", currHashNum, leftHash);
  } else if (rightHash != NULL) {
  	sprintf(currHash, "%u%u", currHashNum, rightHash);
  } else {
    return (void*)currHashNum;
  }

  uint32_t combinedHash = jenkins_one_at_a_time_hash((uint8_t*)currHash, strlen(currHash));
  
  return (void*)combinedHash;
}

//this function was from the common.h header file
double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double)t.tv_sec + (double)t.tv_usec/1e6;
}
