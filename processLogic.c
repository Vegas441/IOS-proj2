#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
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

// Pointers to shared memory
int *A;                         // Step index
int *OQue;                      // Oxygen queue counter
int *HQue;                      // Hydrogen queue counter
int *OCreated;                  // Number of O atoms created
int *HCreated;                  // Number of H atoms created
int *OFinished;                 // Number of O atoms bonded
int *HFinished;                 // Number of H atoms bonded

int *moleculeNumber;

FILE *out;
sem_t *OQueSem;
sem_t *HQueSem;
sem_t *barrier;
sem_t *mutex;
sem_t *creating; 

void mysleep(int max){
    if (max == 0) {
        return ;
    } else {
        max = max * 1000;
        usleep(rand() % (max + 1));
    }
}

void setSharedMemory() {
    
    sh_A = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    sh_OQue = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    sh_HQue = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    sh_OCreated = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    sh_HCreated = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    sh_OFinished = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    sh_HFinished = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    sh_moleculeNumber = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);

    A = (int *) shmat(sh_A, NULL, 0);
    OQue = (int *) shmat(sh_OQue, NULL, 0);
    HQue = (int *) shmat(sh_HQue, NULL, 0);
    OCreated = (int *) shmat(sh_OCreated, NULL, 0);
    HCreated = (int *) shmat(sh_HCreated, NULL, 0);
    OFinished = (int *) shmat(sh_OFinished, NULL, 0);
    HFinished = (int *) shmat(sh_HFinished, NULL, 0);
    moleculeNumber = (int *) shmat(sh_moleculeNumber, NULL, 0);

    
    (*A) = 0;
    (*OQue) = 0;
    (*HQue) = 0;
    (*OCreated) = 0;
    (*HCreated) = 0;
    (*OFinished) = 0;
    (*HFinished) = 0;
    (*moleculeNumber) = 0;

    if((out = fopen("proj2.out","w")) == NULL) {
        fprintf(stderr,"error: output file cannot be opened");
        exit(1);
    }
    setbuf(out, NULL); // For correct output order

    MMAP(OQueSem);
    MMAP(HQueSem);
    MMAP(barrier);
    MMAP(mutex);
    MMAP(creating);

    sem_init(OQueSem,1,0);
    sem_init(HQueSem,1,0);
    sem_init(barrier,1,3);
    sem_init(mutex,1,1);
    sem_init(creating,1,0);

}

void destruct() {

    shmctl(sh_A, IPC_RMID, NULL);
    shmctl(sh_OQue, IPC_RMID, NULL);
    shmctl(sh_HQue, IPC_RMID, NULL);
    shmctl(sh_OCreated, IPC_RMID, NULL);
    shmctl(sh_HCreated, IPC_RMID, NULL);
    shmctl(sh_OFinished, IPC_RMID, NULL);
    shmctl(sh_HFinished, IPC_RMID, NULL);
    shmctl(sh_moleculeNumber, IPC_RMID, NULL);
    

    shmdt(A);
    shmdt(OQue);
    shmdt(HQue);
    shmdt(OCreated);
    shmdt(HCreated);
    shmdt(OFinished);
    shmdt(HFinished);
    shmdt(moleculeNumber);

    fclose(out);    

    sem_destroy(OQueSem);
    sem_destroy(HQueSem);
    sem_destroy(mutex);
    sem_destroy(barrier);
    sem_destroy(creating);
}


void oxygenGenerator(params_t params) {
    pid_t O_children[params.NO];
    for(int idO = 1; idO <= params.NO; idO++) {
        pid_t O = fork();   // Oxygen process
        if(O == -1) {
            fprintf(stderr, "error: process failure");
            exit(1);
        }
        else if(O == 0) {
            (*A)++;
            fprintf(out,"%d:\t\tO %d:\t\tstarted\n",*A,idO);
            (*OCreated)++;
            fflush(out);
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
    for(int idH = 1; idH <= params.NH; idH++) {
        pid_t H = fork();   // Hydrogen process
        if(H == -1) {
            fprintf(stderr, "error: process failure");
            exit(1);
        }
        else if(H == 0) {
            (*A)++;
            fprintf(out,"%d:\t\tH %d:\t\tstarted\n",*A,idH);
            (*HCreated)++;
            fflush(out);
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

    sem_wait(mutex);

    if( (params.NH - (*HFinished) < 2) /* && ((*HQue) < 2) */) {
        (*A)++;
        fprintf(out,"%d:\t\tO %d:\t\tnot enough H\n",*A,idO);
        fflush(out);
        sem_post(mutex);
        (*OFinished)++;
        if(params.NO - (*OFinished) < 1) sem_post(HQueSem);
        exit(0);    
    }

    (*A)++;
    fprintf(out,"%d:\t\tO %d:\t\tgoing to queue\n",*A,idO);
    fflush(out);
    (*OQue)++;

    if((*HQue) >= 2) {
        sem_post(HQueSem);sem_post(HQueSem);
        (*HQue) -= 2;
        sem_post(OQueSem);
        (*OQue)--;
    } else sem_post(mutex);

    sem_wait(OQueSem);

    if( (params.NH - (*HFinished) < 2) /* && ((*HQue) < 2) */) {
        (*A)++;
        fprintf(out,"%d:\t\tO %d:\t\tnot enough H\n",*A,idO);
        fflush(out);
        sem_post(OQueSem);
        (*OFinished)++;
        if(params.NO - (*OFinished) < 1) sem_post(HQueSem);
        exit(0);    
    }

    (*A)++;
    fprintf(out,"%d:\t\tO %d:\t\tcreating molecule %d\n",*A,idO,(*moleculeNumber)+1);
    fflush(out);
    bond(params);
    (*A)++;
    fprintf(out,"%d:\t\tO %d:\t\tmolecule %d created\n",*A,idO,*moleculeNumber);
    fflush(out);
    sem_wait(barrier);
    sem_post(mutex);
    (*OFinished)++;
    if(params.NO - (*OFinished) < 1) sem_post(HQueSem);
    exit(0);
}

void hydrogenQue(params_t params, int idH) {

    sem_wait(mutex);

    if( (params.NO - (*OFinished) < 1) /* && ((*OQue) < 1) */) {
        (*A)++;
        fprintf(out,"%d:\t\tH %d:\t\tnot enough O\n",*A,idH);
        fflush(out);
        sem_post(mutex);
        (*HFinished)++;
        if(params.NH - (*HFinished) < 2) sem_post(OQueSem);
        exit(0);    
    }

    (*A)++;
    fprintf(out,"%d:\t\tH %d:\t\tgoing to queue\n",*A,idH);
    fflush(out);
    (*HQue)++;

    if((*HQue) >= 2 && (*OQue) >= 1) {
        sem_post(HQueSem);sem_post(HQueSem);
        (*HQue) -= 2;
        sem_post(OQueSem);
        (*OQue)--;
    }else sem_post(mutex);
    
    sem_wait(HQueSem);

    if( (params.NO - (*OFinished) < 1) /* && ((*OQue) < 1) */) {
        (*A)++;
        fprintf(out,"%d:\t\tH %d:\t\tnot enough O\n",*A,idH);
        fflush(out);
        sem_post(HQueSem);
        (*HFinished)++;
        if(params.NH - (*HFinished) < 2) sem_post(OQueSem);
        exit(0);    
    }

    (*A)++;
    fprintf(out,"%d:\t\tH %d:\t\tcreating molecule %d\n",*A,idH,(*moleculeNumber)+1);
    fflush(out);

    sem_wait(creating);
    (*A)++;
    fprintf(out,"%d:\t\tH %d:\t\tmolecule %d created\n",*A,idH,*moleculeNumber);
    fflush(out);
    sem_wait(barrier);
    (*HFinished)++;
    if(params.NH - (*HFinished) < 2) sem_post(OQueSem);
    exit(0);
}

void bond(params_t params) {
    mysleep(params.TB);
    sem_post(barrier);sem_post(barrier);sem_post(barrier);
    sem_post(creating);sem_post(creating);
    (*moleculeNumber)++;
}