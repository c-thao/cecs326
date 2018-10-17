#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>


int shmid, semaid; // shared memory id & semaphore memory id
sem_t *sema1;      // semaphore pointer
char (*shm)[10];   // shared memory pointer
pid_t pPid[20];    // array of pids for every pallet child process
pid_t cPid;        // fish process
timer_t timerid;   // timer
FILE *text;        // pointer to output file

void checkProcess(pid_t cur, int *tPid);
void endChildren(pid_t* pPid, pid_t* cPid);
void display(char (*arr)[10]);
void timer_up(union sigval arg);
void timer_init(timer_t* timerid, struct itimerspec* time);

// signal handler which catches various signals
// and determines what to do
void sig_handler(int signo) {
   if (signo == SIGINT) {
      printf("received SIGINT\n");
      abort();
   }
   else if (signo == SIGABRT) {
      printf("received SIGABRT\n");
      fclose(text);
      
      // kill all children processes
      endChildren(pPid, &cPid);
      
      // detach shared memory
      if (shmdt(shm) == -1) {
         perror("shmdt: shmdt failed");
      }
      /*else {
         printf("shmop: shmdt successful\n");
      }*/
   
      // detach semaphore memory
      if (shmdt(sema1) == -1) {
      perror("shmdt: shmdt failed");
      }
      /*else {
      printf("shmop: shmdt successful\n");
      }*/
   
      // destroy semaphore in semaphore memory
      if (sem_destroy(&(*sema1)) == -1) {
      perror("shmdt: shmdt failed");
      }
      /*else {
         printf("shmop: shmdt successful\n");
      }*/
   
      // delete semaphore memory
      if (shmctl(semaid, IPC_RMID, 0) == -1) {
         perror("\nshmop: shmctl failed shared memory not removed");
      }
      else {
         printf("swim_mill detached from semaphore\n");
      }
   
      // delete shared memory
      if (shmctl(shmid, IPC_RMID, 0) == -1) {
         perror("\nshmop: shmctl failed shared memory not removed");
      }
      else {
         printf("swim_mill detached from shared memory\n");
      }
      exit(0);
   }
}

// kill all children processes
void endChildren(pid_t* pPid, pid_t* cPid) {
   // kill all pellets
   while ((*pPid) != NULL) {
      kill(*pPid, SIGTERM);
      pPid++;
   }
   // kill the fish
   kill(*cPid, SIGTERM);
}


int main(int argc, char *argv[]) {
   key_t key = 0612;       // special key for shared memory
   key_t semaKey = 0642;   // special key for semaphore memory
   char space[10][10];     // shared memory size
   sem_t sema;             // semaphore variable
   int tPid;               // number of pellet processes
   int status;             // temporary status variable
   struct itimerspec time; // timer time
   
   // catches signal
   signal(SIGINT, sig_handler);
   
   // catches abort signal
   signal(SIGABRT, sig_handler);
   
   // open output file to write into
   text = fopen("output.txt", "w");
   if (text == NULL) {
      perror("can't open file");
      exit(1);
   }
   fputs("New Run!\n", text);
   
   
   // create shared memory and check if successful
   if ((shmid = shmget(key, sizeof(space), IPC_CREAT | 0666)) == -1) {
      perror("shmget: shmget failed");
      exit(1);
   }
   /*else {
      (void) fprintf(stderr, "shmget: shmget returned %d\n", shmid);
   }*/
   
   // create semaphore memory and check if successful
   if ((semaid = shmget(semaKey, sizeof(sem_t), IPC_CREAT | 0666)) == -1) {
      perror("shmget: shmget failed");
      exit(1);
   }
   /*else {
      (void) fprintf(stderr, "shmget: shmget returned %d\n", semaid);
   }*/
   
   // attach shared memory
   if ((shm = shmat(shmid, NULL, 0)) == (char*) -1) {
      perror("shmat: shmat failed");
      exit(1);
   }
   /*else {
      printf("shmop: shmat successful\n");
   }*/
   
   // attach semaphore to shared memory
   if ((sema1 = shmat(semaid, NULL, 0)) == (char*) -1) {
      perror("shmat: shmat failed");
      exit(1);
   }
   /*else {
      printf("shmop: shmat successful\n");
   }*/
   
   // initialize shared memory with ' '
   for(int i = 0; i < 10; i++) {
      for (int j = 0; j < 10; j++) {
         shm[i][j] = ' ';
      }
   }
   
   printf("initial river\n");
   display(shm);
   
   // initializes a semaphore for processes to communicate
   if (sem_init(&sema, 1, 1) == -1) {
      perror("semaphore initialization failed");
   }
   /*else {
      printf("\nsemaphore initialization success");
   }*/
   
   // put semaphore into shared semaphore memory
   *sema1 = sema;
   
   // fork to create fish child process for monitoring
   if ((cPid = fork()) == -1)  { // failed
      perror("\nforking has failed");
   }
   else if (cPid == 0) { // child
      //printf("\nfork was successful");
      
      // execute another program
      if((execve("./fish", NULL, NULL)) == -1) {
         perror("\nexecve of ./fish failed");
      }
   }
   
   // initialize timer
   timer_init(&timerid, &time);
   
   // create more pellet processes
   tPid = 0;
   
   // start and arm timer
   if (timer_settime(timerid, 0, &time, NULL) == -1) {
      perror("\ntimer not set");
      exit(1);
   }else {
      printf("timer started and set to 30s\n");
   }

   while (1) {
      // waits till a child process is done
      // if a child process has terminated
      // then clean up the zombie process
      if (tPid > 19) {
         pid_t child = wait();
         // write child status to output file
         fputs(child, text);
         tPid--;
      }
      else { // not too many processes at one time
         if ((pPid[tPid] = fork()) == -1)  { // failed
            perror("\nforking has failed");
         }
         else if (pPid[tPid] == 0) { // child
            tPid++;
            //printf("\nfork was successful");
            // execute another program
            if((execve("./pellet", NULL, NULL)) == -1) {
               perror("\nexecve of ./pellet failed");
            }
         }
      }
      sleep(1); // delay between new process creation
   }
   // close output file
   fclose(text);
   abort();
   
   exit(0);
}

// display shared memory
void display(char (*arr)[10]) {
   printf("\n");
   for(int i = 0; i < 10; i++) {
      printf("|");
      for (int j = 0; j < 10; j++) {
         printf("%c|", arr[i][j]);
      }
      printf("\n");
   }
}


// timer expire notification function
void timer_up(union sigval arg) {
   if(arg.sival_int != 0) {
      printf("timer has expired\n");
      // delete timer
      if (timer_delete(timerid) == -1) {
         perror("\ntimer not deleted");
         exit(1);
      }else {
         printf("timer has been deleted\n");
         abort();
      }
   }
   else {
      perror("\ntimer interrupted");
      exit(1);  
   }
}

// create and initialize a timer
void timer_init(timer_t* timerid, struct itimerspec* time) {
   
   struct sigevent sig;
   
   // create sigevent object for timer signal
   sig.sigev_notify = SIGEV_THREAD;
   sig.sigev_value.sival_ptr = &timerid;
   sig.sigev_notify_function = timer_up;
   sig.sigev_notify_attributes = NULL;
   
   // create time
   if (timer_create(CLOCK_REALTIME, &sig, &(*timerid)) == -1) {
      perror("\ntimer not created");
      exit(1);
   }else {
      printf("timer initialized\n");
   }
   
   // make timer 30s
   time->it_value.tv_sec = 30;
   time->it_value.tv_nsec = 0;
   time->it_interval.tv_sec = 0;
   time->it_interval.tv_nsec = 0;

}
