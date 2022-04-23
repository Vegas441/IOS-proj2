#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "processLogic.h"

int TB_global;

/**
 * @brief Function to check the validity of arguments 
 * 
 * @param NO Number of oxygen atoms
 * @param NH Number of hydrogen atoms
 * @param TI Max time an atom waits before getting into the que 
 * @param TB Max time to create a molecule 
 * @return true 
 * @return false 
 */
bool argsFormatCheck(int NO, int NH, int TI, int TB) {
    if(NO < 0 || NH < 0) 
        return false;
    else if(TI < 0 || TI > 1000 || TB < 0 || TB > 1000)
        return false;
    else return true;
} 

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "error: invalid number of arguments\ncorrect format: ./proj2 NO NH TI TB\n");
        exit(1);
    }

    int NO = atoi(argv[1]);
    int NH = atoi(argv[2]);
    int TI = atoi(argv[3]);
    int TB = atoi(argv[4]);

    TB_global = TB;

    // Check arguments 
    if(!argsFormatCheck(NO,NH,TI,TB)){
        fprintf(stderr, "error: invalid argument format\n");
        exit(1);
    }
    
    // Put variables into shared memory 
    setSharedMemory();
    
    // Main process
    pid_t mProc = fork();
    if(mProc == -1){
        // Error ocurred
        fprintf(stderr, "error: process failure");
        exit(1);
    }else if(mProc == 0) {
        // Child process
        pid_t gen = fork();
        if(gen == -1) {
            // Error ocurred
            fprintf(stderr, "error: process failure");
            exit(1);
        }else if(gen == 0) {
            oxygenGenerator(NO,TI);
        }else {
            hydrogenGenerator(NO,TI);
        }
    }else{
        // Parent process
        waitpid(mProc,NULL,0);
        destruct();
        exit(0);
    }

}