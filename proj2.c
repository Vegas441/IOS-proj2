#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "processLogic.h"

/**
 * @brief Function to determine if argument is a string of digits
 * Function taken from stackoverflow: 
 * https://stackoverflow.com/questions/29248585/c-checking-command-line-argument-is-integer-or-not
 * 
 * @param number Argument
 * @return true 
 * @return false 
 */
bool isNumber(char number[])
{
    int i = 0;

    // Checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}

/**
 * @brief Function to check the correct values of arguments 
 * 
 * @param NO Number of oxygen atoms
 * @param NH Number of hydrogen atoms
 * @param TI Max time an atom waits before getting into the que 
 * @param TB Max time to create a molecule 
 * @return true 
 * @return false 
 */
bool argsValueCheck(params_t params) {
    if(params.NO < 1 || params.NH < 1) 
        return false;
    else if(params.TI < 0 || params.TI > 1000 || params.TB < 0 || params.TB > 1000)
        return false;
    else return true;
} 

//////////////
//   MAIN
/////////////
int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "error: invalid number of arguments\ncorrect format: ./proj2 NO NH TI TB\n");
        exit(1);
    }

    params_t params; char *param_err = NULL;

    // Check if arguments are numbers
    if(!isNumber(argv[1]) || !isNumber(argv[2]) || !isNumber(argv[3]) || !isNumber(argv[4])) {
        fprintf(stderr, "error: invalid argument format\n");
        exit(1);
    }

    params.NO = strtol(argv[1], &param_err, 10);
    params.NH = strtol(argv[2], &param_err, 10);
    params.TI = strtol(argv[3], &param_err, 10);
    params.TB = strtol(argv[4], &param_err, 10);

    if(*param_err != 0) {
        fprintf(stderr, "error: invalid argument format\n");
        exit(1);
    }

    // Check correct values of arguments
    if(!argsValueCheck(params)){
        fprintf(stderr, "error: invalid argument format\n");
        exit(1);
    }
    
    // Put variables into shared memory 
    setSharedMemory();
    
    // Main process
    pid_t mProc = fork();
    if(mProc == -1){
        // Error ocurred
        fprintf(stderr, "error: main process failure\n");
        exit(1);
    }else if(mProc == 0) {
        // Child process
        oxygenGenerator(params);
        waitpid(mProc,NULL,0);
        exit(0);
    }else{
        // Parent process
        hydrogenGenerator(params);
        waitpid(mProc,NULL,0);
        destruct();
        exit(0);
    }

}