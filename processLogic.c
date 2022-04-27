#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include <sys/shm.h>
#include "processLogic.h"

// TODO -> FIXNI MOLEKULOVY INDEX

// Shared variables
int sh_A = 0;
int sh_OQue = 0;
int sh_HQue = 0;
int sh_OCreated = 0;
int sh_HCreated = 0;
int sh_OFinished = 0;
int sh_HFinished = 0;
int sh_moleculeNumber = 0;

// For barrier
int sh_bar_count;
int sh_bar_n;

// Pointers to shared memory
int *A;                         // Step index
int *OQue;                      // Oxygen queue counter
int *HQue;                      // Hydrogen queue counter
int *OCreated;                  // Number of O atoms created
int *HCreated;                  // Number of H atoms created
int *OFinished;                 // Number of O atoms bonded
int *HFinished;                 // Number of H atoms bonded

int *moleculeNumber;

int *bar_count;
int *bar_n;

FILE *out;
sem_t *OQueSem;
sem_t *HQueSem;
sem_t *mutex;
sem_t *write_sem;

// Barrier
sem_t *turnstile;
sem_t *turnstile2;
sem_t *bar_mutex; 


void mysleep(int max){
    if (max == 0) {
        return ;
    } else {
        max = max * 1000;
        usleep(rand() % (max + 1));
    }
}

void setSharedMemory() {

    if ((sh_A = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666))  == -1 ||
        (sh_OQue = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666))  == -1 ||
        (sh_HQue = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666))  == -1 ||
        (sh_OCreated = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 ||
        (sh_HCreated = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 ||
        (sh_OFinished = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 ||
        (sh_HFinished = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 ||
        (sh_moleculeNumber = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 ||
        (sh_bar_count = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 ||
        (sh_bar_n = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) {
            fprintf(stderr,"error: shared memory cannot be created");
            exit(1);
        }

    if ((A = (int *) shmat(sh_A, NULL, 0)) == NULL ||
        (OQue = (int *) shmat(sh_OQue, NULL, 0)) == NULL ||
        (HQue = (int *) shmat(sh_HQue, NULL, 0)) == NULL ||
        (OCreated = (int *) shmat(sh_OCreated, NULL, 0)) == NULL ||
        (HCreated = (int *) shmat(sh_HCreated, NULL, 0)) == NULL ||
        (OFinished = (int *) shmat(sh_OFinished, NULL, 0)) == NULL ||
        (HFinished = (int *) shmat(sh_HFinished, NULL, 0)) == NULL ||
        (moleculeNumber = (int *) shmat(sh_moleculeNumber, NULL, 0)) == NULL ||
        (bar_count = (int *) shmat(sh_bar_count, NULL, 0)) == NULL ||
        (bar_n = (int *) shmat(sh_bar_n, NULL, 0)) == NULL) {
            fprintf(stderr,"error: shared memory cannot be created");
            exit(1);
        }
    
    (*A) = 0;
    (*OQue) = 0;
    (*HQue) = 0;
    (*OCreated) = 0;
    (*HCreated) = 0;
    (*OFinished) = 0;
    (*HFinished) = 0;
    (*moleculeNumber) = 0;

    *(bar_count) = 0;
    *(bar_n) = 3;

    if((out = fopen("proj2.out","w")) == NULL) {
        fprintf(stderr,"error: output file cannot be opened");
        exit(1);
    }
    setbuf(out, NULL); // For correct output order

    MMAP(OQueSem);
    MMAP(HQueSem);
    MMAP(mutex);
    MMAP(write_sem);

    MMAP(turnstile);
    MMAP(turnstile2);
    MMAP(bar_mutex);

    if( sem_init(OQueSem,1,0) == -1 ||
        sem_init(HQueSem,1,0) == -1 ||
        sem_init(mutex,1,1) == -1 ||
        sem_init(write_sem,1,1) == -1 ||
        sem_init(turnstile,1,0) == -1 ||
        sem_init(turnstile2,1,1) == -1 ||
        sem_init(bar_mutex,1,1) == -1){
            fprintf(stderr, "error: semaphore initialization failed");
            exit(1);
        }

}

void destruct() {

    if (
    shmctl(sh_A, IPC_RMID, NULL) == -1 ||
    shmctl(sh_OQue, IPC_RMID, NULL) == -1 ||
    shmctl(sh_HQue, IPC_RMID, NULL) == -1 ||
    shmctl(sh_OCreated, IPC_RMID, NULL) == -1 ||
    shmctl(sh_HCreated, IPC_RMID, NULL) == -1 ||
    shmctl(sh_OFinished, IPC_RMID, NULL) == -1 ||
    shmctl(sh_HFinished, IPC_RMID, NULL) == -1 ||
    shmctl(sh_moleculeNumber, IPC_RMID, NULL) == -1 ||
    shmctl(sh_bar_count, IPC_RMID, NULL) == -1 ||
    shmctl(sh_bar_n, IPC_RMID, NULL) == -1 ||
    

    shmdt(A) == -1 ||
    shmdt(OQue) == -1 ||
    shmdt(HQue) == -1 ||
    shmdt(OCreated) == -1 ||
    shmdt(HCreated) == -1 ||
    shmdt(OFinished) == -1 ||
    shmdt(HFinished) == -1 ||
    shmdt(moleculeNumber) == -1 ||  
    shmdt(bar_count) == -1 ||
    shmdt(bar_n) == -1) {
        fprintf(stderr, "error: shared memory deallocation failed");
        exit(1);
    }

    fclose(out);    

    if (
        sem_destroy(OQueSem) == -1 ||
        sem_destroy(HQueSem) == -1 ||
        sem_destroy(mutex) == -1 ||
        sem_destroy(write_sem) == -1 ||
        sem_destroy(turnstile) == -1 ||
        sem_destroy(turnstile2) == -1 ||
        sem_destroy(bar_mutex) == -1) {
            fprintf(stderr, "error: semaphore destruction failed");
            exit(1);
        }
}

void oxygenGenerator(params_t params) {
    pid_t O_children[params.NO];

    // Prevents deadlock when no oxygens are present
    if(params.NO == 0) sem_post(HQueSem);

    for(int idO = 1; idO <= params.NO; idO++) {
        pid_t O = fork();   // Oxygen process
        if(O == -1) {
            fprintf(stderr, "error: process failure");
            exit(1);
        }
        else if(O == 0) {
            srand(time(NULL)*getpid());
            sem_wait(write_sem);
            (*A)++;
            fprintf(out,"%d: O %d: started\n",*A,idO);
            fflush(out);
            (*OCreated)++;
            sem_post(write_sem);
            mysleep(params.TI);
            // Go into queue
            oxygenQue(params, idO);
            //exit(0);  
        }else O_children[idO-1] = O;
    }

    for(int i = 0; i < params.NO; i++) {
        waitpid(O_children[i],NULL,0);
    }

}

void hydrogenGenerator(params_t params) {
    pid_t H_children[params.NH];

    // Prevents deadlock when no hydrogens are present
    if(params.NH == 0) sem_post(OQueSem);

    for(int idH = 1; idH <= params.NH; idH++) {
        pid_t H = fork();   // Hydrogen process
        if(H == -1) {
            fprintf(stderr, "error: process failure");
            exit(1);
        }
        else if(H == 0) {
            srand(time(NULL)*getpid());
            sem_wait(write_sem);
            (*A)++;
            fprintf(out,"%d: H %d: started\n",*A,idH);
            fflush(out);
            (*HCreated)++;
            sem_post(write_sem);
            mysleep(params.TI);
            // Go into queue
            hydrogenQue(params, idH);
            //exit(0);
        }else H_children[idH-1] = H;
    }

    for(int i = 0; i < params.NH; i++) {
        waitpid(H_children[i],NULL,0);
    }

}

void oxygenQue(params_t params, int idO) {

    //sem_wait(mutex);

    // Prevents deadlock when only 1 of each atoms enter
    if(params.NO == 1 && params.NH == 1) {
        (*OFinished)++;
        sem_post(HQueSem);
    }

    sem_wait(write_sem);
    (*A)++;
    fprintf(out,"%d: O %d: going to queue\n",*A,idO);
    fflush(out);
    sem_post(write_sem);

    sem_wait(mutex);

    (*OQue)++;

    if((*HQue) >= 2) {
        (*moleculeNumber)++;
        sem_post(HQueSem);sem_post(HQueSem);
        (*HQue) -= 2;
        sem_post(OQueSem);
        (*OQue)--;
    } else sem_post(mutex);

    sem_wait(OQueSem);

    if( (params.NH - (*HFinished) < 2) ) {
        sem_wait(write_sem);
        (*A)++;
        fprintf(out,"%d: O %d: not enough H\n",*A,idO);
        fflush(out);
        sem_post(write_sem);
        sem_post(OQueSem);
        sem_post(mutex);
        (*OFinished)++;
        if(params.NO - (*OFinished) < 1) sem_post(HQueSem);
        exit(0);    
    }

    sem_wait(write_sem);
    (*A)++;
    fprintf(out,"%d: O %d: creating molecule %d\n",*A,idO,*moleculeNumber);
    fflush(out);
    sem_post(write_sem);
    bond(params);

    //--- Barrier ---//
    sem_wait(bar_mutex);
        (*bar_count) += 1;
        if((*bar_count) == (*bar_n)) {
            sem_wait(turnstile2);
            sem_post(turnstile);
        }
    sem_post(bar_mutex);

    sem_wait(turnstile);
    sem_post(turnstile);

    sem_wait(bar_mutex);
        (*bar_count) -= 1;
        if((*bar_count) == 0) {
            sem_wait(turnstile);
            sem_post(turnstile2);
        }
    sem_post(bar_mutex);

    sem_wait(turnstile2);
    sem_post(turnstile2);
    //--- Barrier ---//

    sem_wait(write_sem);
    (*A)++;
    fprintf(out,"%d: O %d: molecule %d created\n",*A,idO,*moleculeNumber);
    fflush(out);
    sem_post(write_sem);

    sem_post(mutex);
    (*OFinished)++;
    if(params.NO - (*OFinished) < 1) sem_post(HQueSem);
    exit(0);
}

void hydrogenQue(params_t params, int idH) {

    //sem_wait(mutex);

    // Prevents deadlock when only 1 of each atoms enter
    if(params.NO == 1 && params.NH == 1) {
        (*HFinished)++;
        sem_post(OQueSem);
    }

    sem_wait(write_sem);
    (*A)++;
    fprintf(out,"%d: H %d: going to queue\n",*A,idH);
    fflush(out);
    sem_post(write_sem);

    sem_wait(mutex);

    (*HQue)++;

    if((*HQue) >= 2 && (*OQue) >= 1) {
        (*moleculeNumber)++;
        sem_post(HQueSem);sem_post(HQueSem);
        (*HQue) -= 2;
        sem_post(OQueSem);
        (*OQue)--;
    }else sem_post(mutex);
    
    sem_wait(HQueSem);

    if( (params.NO - (*OFinished) < 1)) {
        sem_wait(write_sem);
        (*A)++;
        fprintf(out,"%d: H %d: not enough O or H\n",*A,idH);
        fflush(out);
        sem_post(write_sem);
        sem_post(mutex);
        sem_post(HQueSem);
        (*HFinished)++;
        if(params.NH - (*HFinished) < 2) sem_post(OQueSem);
        exit(0);    
    }

    sem_wait(write_sem);
    (*A)++;
    fprintf(out,"%d: H %d: creating molecule %d\n",*A,idH,*moleculeNumber);
    fflush(out);
    sem_post(write_sem);

    //--- Barrier ---//
    sem_wait(bar_mutex);
        (*bar_count) += 1;
        if((*bar_count) == (*bar_n)) {
            sem_wait(turnstile2);
            sem_post(turnstile);
        }
    sem_post(bar_mutex);

    sem_wait(turnstile);
    sem_post(turnstile);

    sem_wait(bar_mutex);
        (*bar_count) -= 1;
        if((*bar_count) == 0) {
            sem_wait(turnstile);
            sem_post(turnstile2);
        }
    sem_post(bar_mutex);

    sem_wait(turnstile2);
    sem_post(turnstile2);
    //--- Barrier ---//

    sem_wait(write_sem);
    (*A)++;
    fprintf(out,"%d: H %d: molecule %d created\n",*A,idH,*moleculeNumber);
    fflush(out);
    sem_post(write_sem);

    (*HFinished)++;
    if(params.NH - (*HFinished) < 2) sem_post(OQueSem);
    exit(0);
}

void bond(params_t params) {
    mysleep(params.TB);
    //sem_post(creating);sem_post(creating);
}