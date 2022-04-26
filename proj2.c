#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "processLogic.h"

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
bool argsFormatCheck(params_t params) {
    if(params.NO < 0 || params.NH < 0) 
        return false;
    else if(params.TI < 0 || params.TI > 1000 || params.TB < 0 || params.TB > 1000)
        return false;
    else return true;
} 

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "error: invalid number of arguments\ncorrect format: ./proj2 NO NH TI TB\n");
        exit(1);
    }

    params_t params; char *param_err = NULL;

    params.NO = strtol(argv[1], &param_err, 10);
    params.NH = strtol(argv[2], &param_err, 10);
    params.TI = strtol(argv[3], &param_err, 10);
    params.TB = strtol(argv[4], &param_err, 10);

    if(*param_err != 0) {
        fprintf(stderr, "error: invalid argument format\n");
        exit(1);
    }

    // Check arguments 
    if(!argsFormatCheck(params)){
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
            oxygenGenerator(params);
        }else {
            hydrogenGenerator(params);
        }
    }else{
        // Parent process
        waitpid(mProc,NULL,0);
        destruct();
        exit(0);
    }

}