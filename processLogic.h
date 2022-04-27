#include <stdbool.h>
#include <sys/mman.h>


// Makes variable shared 
#define MMAP(ptr) {(ptr) = mmap(NULL, sizeof(*(ptr)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);}
#define UNMAP(ptr){munmap((ptr), sizeof((ptr)));}

/**
 * @brief Struct for input arguments
 * 
 */
typedef struct params_t {
    int NO;
    int NH;
    int TI;
    int TB;
} params_t;


/*
typedef struct barrier_t {
    sem_t *turnstile;
    sem_t *turnstile2;
    sem_t *bar_mutex;
    int *count;
    int *n;
} barrier_t;
*/

/**
 * @brief Halts process for random amount of time in <0,max> interval
 * 
 */
void mysleep(int max);

/**
 * @brief Sets variables as shared
 * 
 */
void setSharedMemory();

/**
 * @brief Destroys semaphores and frees shared memory
 * 
 */
void destruct();

/**
 * @brief Generates oxygen processes
 * 
 * 
 */
void oxygenGenerator(params_t params);

/**
 * @brief Generates hydrogen processes
 * 
 * 
 */
void hydrogenGenerator(params_t params);

/**
 * @brief Oxygen atom run
 * 
 * @param idO ID of oxygen atom
 */
void oxygenQue(params_t params, int idO);

/**
 * @brief Hydrogen atom run
 * 
 * @param idH ID of hydrogen atom 
 */
void hydrogenQue(params_t params, int idH);

/**
 * @brief Creates molecules
 * 
 */
void bond(params_t params);