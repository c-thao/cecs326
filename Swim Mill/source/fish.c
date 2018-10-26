#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>

char (*fish)[10];  // fish variable
sem_t *sema; // semaphore variable

// function definitions
int checkPellets(char (*arr)[10], int org);

/* 
Handles shared memory cleanup when an interrupt or a
termination signal is received.
*/
void sig_handler(int signo) {
   if (signo == SIGINT || signo == SIGTERM) {
      if (signo == SIGINT) {
         fprintf(stderr, "interrupt received, fish process [%d] aborting\n", getpid());
      }
      abort();
   }
   else if (signo == SIGABRT) {
      // detach shared memory
      if (shmdt(fish) == -1) {
         perror("shmdt: shmdt failed");
      }
      else {
         printf("fish detached from shared memory\n");
      }
   
      // detach semaphore memory
      if (shmdt(sema) == -1) {
      perror("shmdt: shmdt failed");
      }
      else {
      printf("fish detached from semaphore memory\n");
      }
      exit(0);
   }
}

// main function
int main(int argc, char *argv[]) {
   key_t key = 0612;   // special key
   key_t semaKey = 0642; // special key for semaphore memory
   int shmid, semaid;        // shared memory id & semaphore memory id
   char space[10][10]; // size of array
   int fCol; // fish column
   
   // catches interrupt signal
   signal(SIGINT, sig_handler);
   
   // catches abort signal
   signal(SIGABRT, sig_handler);

   // catches termination signal
   signal(SIGTERM, sig_handler);

   
   // create shared memory and check if successful
   if ((shmid = shmget(key, sizeof(space), IPC_CREAT | 0666)) == -1) {
      perror("shmget: shmget failed");
      exit(1);
   }
   
   // initialize fish
   if ((fish = shmat(shmid, NULL, 0)) == (char*) -1) {
      perror("shmat: shmat failed");
      exit(1);
   }
   
   // create semaphore memory and check if successful
   if ((semaid = shmget(semaKey, sizeof(sem_t), IPC_CREAT | 0666)) == -1) {
      perror("shmget: shmget failed");
      exit(1);
   }
   
   // grab semaphore
   if ((sema = shmat(semaid, NULL, 0)) == (char*) -1) {
      perror("shmat: shmat failed");
      exit(1);
   }
   
   // fish starting point
   fish[9][4] = 'f';
   fCol = 4;
   
   
   while(1) {
      // wait for semaphore to unlock
      sem_wait(&(*sema));
      // check to lock shared memory if head of wait queue
      while((shmctl(shmid, SHM_LOCK, shmctl(shmid, SHM_STAT, 0))) == -1);
      
      // check for closest pellet
      int pCol = checkPellets(fish, fCol);
      // update fish position to chase pellet
      if (fCol < pCol) {
         fish[9][fCol] = ' ';
         fCol = fCol + 1;
         fish[9][fCol] = 'f';
      }
      else if (fCol > pCol){
         fish[9][fCol] = ' ';
         fCol = fCol - 1;
         fish[9][fCol] = 'f';
      }
      display(fish);
      // unlock semaphore
      sem_post(&(*sema));
      // unlock memory
      shmctl(shmid, SHM_UNLOCK, shmctl(shmid, SHM_STAT, 0));
      sleep(1); // delay
   }
   printf("fish is done\n");
   
   // detach shared memory
   if (shmdt(fish) == -1) {
      perror("shmdt: shmdt failed");
   }
   else {
      printf("shmop: shmdt successful\n");
   }
   
   // detach semaphore memory
   if (shmdt(sema) == -1) {
      perror("shmdt: shmdt failed");
   }
   else {
      printf("shmop: shmdt successful\n");
   }
   exit(0);
}

/*
Checks the shared memory to find the closest pellet
and return the column number of the corresponding
pellet.
*/
int checkPellets(char (*arr)[10], int org) {
   int pCol = org;
   int pRow = 0;
   int pFound = 0;
   for(int i = 9; i > 0; i--) {
      for (int j = 0; j < 10; j++) {
         if (arr[i][j] == (char)0xA0) { // space is 0x20 | 0x80 = 0xA0
            if (pFound > 0) { // found another close pellet
               // check for closest
               if ((abs(org - j)) < (abs(org - pCol)) && (pRow > i)) {
                  pCol = j; // update
               }
            }
            else { // found one pellet chase it
               pCol = j;
            }
            pFound++; // update pellet spotted
         }
      }
   }
   if (pFound == 0) {
      pCol = 4; // no pellet go back to middle
   }
   return pCol;
}
