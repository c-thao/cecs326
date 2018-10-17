#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>

char (*pellet)[10];   // pellet variable
sem_t *sema; // semaphore variable

void display(char (*arr)[10]);
// signal handler
void sig_handler(int signo) {
   if (signo == SIGINT || signo == SIGTERM) {
      if (signo == SIGINT) {
         fprintf(stderr, "interrupt received, pellet process [%d] aborting\n", getpid());
      }
      abort();
   }
   else if (signo == SIGABRT) {
      // detach shared memory
      if (shmdt(pellet) == -1) {
         perror("shmdt: shmdt failed");
      }
      else {
         printf("pellet detached from shared memory\n");
      }
   
      // detach semaphore memory
      if (shmdt(sema) == -1) {
      perror("shmdt: shmdt failed");
      }
      else {
      printf("pellet detached from semaphore\n");
      }
      exit(0);
   }
}


int main(int argc, char *argv[]) {
   key_t key = 0612; // special key for shared memory
   key_t semaKey = 0642; // special key for semaphore memory
   int shmid, semaid;        // shared memory id & semaphore memory id
   char space[10][10]; // size of array
   
   // catches signal
   signal(SIGINT, sig_handler);
   
   // catches abort signal
   signal(SIGABRT, sig_handler);

   // catches abort signal
   signal(SIGTERM, sig_handler);
   
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
   
   // grab semaphore
   if ((sema = shmat(semaid, NULL, 0)) == (char*) -1) {
      perror("shmat: shmat failed");
      exit(1);
   }
   /*else {
      printf("shmop: shmat successful pallet is now active\n");
   }*/
   
   // initialize pallet
   if ((pellet = shmat(shmid, NULL, 0)) == (char*) -1) {
      perror("shmat: shmat failed");
      exit(1);
   }
   /*else {
      printf("shmop: shmat successful pallet is now active\n");
   }*/
   
   // initialize pallet
   srand(time(0)); // random seed
   int row = (rand() % 9);
   int col = (rand() % 10);
   // wait for semaphore to unlock
   sem_wait(&(*sema));
   // check to lock shared memory
   while((shmctl(shmid, SHM_LOCK, shmctl(shmid, SHM_STAT, 0))) == -1);
   pellet[row][col] |= 0x80;
   display(pellet);
   // unlock semaphore
   sem_post(&(*sema));
   // unlock memory
   shmctl(shmid, SHM_UNLOCK, shmctl(shmid, SHM_STAT, 0));
   sleep(1); // delay
   
   do {
      // wait for semaphore to unlock
      sem_wait(&(*sema));
      // check to lock shared memory
      while((shmctl(shmid, SHM_LOCK, shmctl(shmid, SHM_STAT, 0))) == -1);
      
      //printf("\npellet moves");
      // clear previous position
      if (row != 0) {
         pellet[row-1][col] = ' ';
      }
      // update position
      pellet[row][col] |= 0x80;
      display(pellet);
      row++;
      if (row != 10) {
        // unlock semaphore
        sem_post(&(*sema));
        // unlock memory
      	shmctl(shmid, SHM_UNLOCK, shmctl(shmid, SHM_STAT, 0));
        sleep(1); // delay
      }
   }while (row < 10);
   printf("pallet pid[%d] at [%d][%d]\n", getpid(), (--row), col);
   // check to see if eaten
   if (pellet[row][col] == (char)0xE6) {
      pellet[row][col] = 'f'; // eaten leave only fish
      printf(" was eaten\n");
      
   }
   else { // clear pallet if not eaten
      pellet[row][col] = ' '; // not eaten leave empty
      printf(" was not eaten\n");
      
   }
   
   printf("pellet is done\n");
   // unlock semaphore
   sem_post(&(*sema)); 
   // unlock memory
   shmctl(shmid, SHM_UNLOCK, shmctl(shmid, SHM_STAT, 0));
   
   // detach shared memory
   if (shmdt(pellet) == -1) {
      perror("shmdt: shmdt failed");
   }
   /*else {
      printf("shmop: shmdt successful\n");
   }*/
   
   // detach semaphore memory
   if (shmdt(sema) == -1) {
      perror("shmdt: shmdt failed");
   }
   /*else {
      printf("shmop: shmdt successful\n");
   }*/
   
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

