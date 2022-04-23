#include <stdbool.h>
#include <sys/mman.h>


// Makes variable shared 
#define MMAP(ptr) {(ptr) = mmap(NULL, sizeof(*(ptr)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);}
#define UNMAP(ptr){munmap((ptr), sizeof((ptr)));}


extern int *A;
extern int *OQue;
extern int *HQue;
extern int *OFinished;
extern int *HFinished;

extern int *moleculeNumber;

extern int TB_global;   // Global TB value to make it easier to pass to bond

extern FILE *out;

// Que semaphors
extern sem_t *HQueSem;
extern sem_t *OQueSem;
extern sem_t *barrier;
extern sem_t *mutex;
extern sem_t *creating; 

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
 * @brief 
 * 
 */
void destruct();

/**
 * @brief Generates oxygen processes
 * 
 * 
 */
void oxygenGenerator(int NO, int TI);

/**
 * @brief Generates hydrogen processes
 * 
 * 
 */
void hydrogenGenerator(int NH, int TI);

/**
 * @brief 
 * 
 * @param idO ID of oxygen atom
 */
void oxygenQue(int idO);

/**
 * @brief 
 * 
 * @param idH ID of hydrogen atom 
 */
void hydrogenQue(int idH);

/**
 * @brief Creates molecules
 * 
 */
void bond();