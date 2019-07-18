#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce;
int total_items, max_buf_size, num_workers;
int * buffer = NULL;
int count=0;
pthread_cond_t consume = PTHREAD_COND_INITIALIZER;
pthread_cond_t produce = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock; 

//Worker data structures 
struct worker_info{ 
  pthread_t worker_id;
  int thread_num;
};
struct worker_info *winfo = NULL;
// declare any global data structures, variables, etc that are required
// e.g buffer to store items, pthread variables, etc

void print_produced(int num) {

  printf("Produced %d\n", num);
}

void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}
/* produce items and place in buffer (array or linked list)
 * add more code here to add items to the buffer (these items will be consumed
 * by worker threads)
 * use locks and condvars suitably
 */
void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
      if(item_to_produce >= total_items){
        for(int i=0; i<num_workers; ++i)
        pthread_cond_signal(&consume);
       break;
      }
      pthread_mutex_lock(&lock);
      if(count>=max_buf_size){
        pthread_cond_signal(&consume);
        pthread_cond_wait(&produce, &lock);
      }
      print_produced(item_to_produce);
      count+=1;
      buffer[count-1]=item_to_produce;
      item_to_produce+=1;
      pthread_mutex_unlock(&lock);
    }
  return 0;
}

void *worker_job(void * data){
  int thread_id = *((int*)data);
  while(1){
      pthread_mutex_lock(&lock);
      if(count<=0 && item_to_produce >= total_items){
        pthread_mutex_unlock(&lock);
        break;
      }
      while(count<=0 && item_to_produce < total_items){
        pthread_cond_signal(&produce);
        pthread_cond_wait(&consume, &lock);
      }
      if(count<=0 && item_to_produce >= total_items){
        pthread_mutex_unlock(&lock);
        break;
      }
      print_consumed(buffer[count-1],thread_id);
      buffer[count-1] = -1;
      count-=1;
      pthread_mutex_unlock(&lock);
  }
  return 0;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item

int main(int argc, char *argv[])
{
 
  int master_thread_id = 0;
  pthread_t master_thread;
  item_to_produce = 0;
  
  if (argc < 4) {
    printf("./master-worker #total_items #max_buf_size #num_workers e.g. ./exe 10000 1000 4\n");
    exit(1);
  }
  else {
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  }

  winfo = (struct worker_info*)malloc(sizeof(struct worker_info)*num_workers);
  buffer = (int*)malloc(sizeof(int)*max_buf_size);
  for (int i = 0; i < max_buf_size; ++i)
  {
    buffer[i] = -1;
  }
  // Initlization code for any data structures, variables, etc


  //create master producer thread
  pthread_create(&master_thread, NULL, generate_requests_loop, (void *)&master_thread_id);

  //create worker consumer threads
  for(int i=0; i<num_workers; i=i+1){
    winfo[i].thread_num = i+1;
    pthread_create(&winfo[i].worker_id, NULL, worker_job, (void*)&winfo[i].thread_num);
  }  

  //wait for all threads to complete
  for(int i=0; i<num_workers; ++i){
    pthread_join(winfo[i].worker_id, NULL);
  }
  pthread_join(master_thread, NULL);
  printf("master joined\n");

  //deallocate and free up any memory you allocated
  free(winfo);
  free(buffer);
  pthread_mutex_destroy(&lock);  
  return 0;
}