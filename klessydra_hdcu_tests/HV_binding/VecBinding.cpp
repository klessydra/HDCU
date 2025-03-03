//-----------------------------------------------------------------------------------------------------------------------------------------
// File:        VecBinding.cpp
// Creator:     Rocco Martino
// Date:        2021-06-30
// Description: This program performs Binding operations on hypervectors using both software and HDCU (HyperDimensional Computation Unit). 
//              It utilizes the Klessydra-T13 architecture, featuring 3 hardware threads (harts) with interrupt management.
//              The size of a Hypervector could vary in a very large range (10, 100, 1000, 10000). For this reason, we 
//              will memorize the entire HV value in a C array. 
//              In this example, we will execute the HVBIND operation (bitwise XOR) on two hypervectors A and B, storing the result in C.
//-----------------------------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ctime>
#include <cstdlib>

//#define DEBUG // Enable detailed debugging print statements

extern "C" { // Klessydra dsp_libraries are written in C and so they're imported as extern:
    #include "dsp_functions.h"
    #include "functions.h"
}

// Funzione per ottenere un valore casuale di HV_PART tra valori validi
int getRandomValidHVPart() {
    int validValues[] = {1, 2, 4, 8, 16, 32}; // Così gli ipervettori vanno da 32 a 8192
    int size = sizeof(validValues) / sizeof(validValues[0]);
    return validValues[rand() % size];
}

// Funzione per generare ipervettori casuali
void generateRandomHypervector(uint32_t* vector, int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand();
    }
}

// Funzione per stampare un ipervettore (solo se DEBUG è definita)
#ifdef DEBUG
void printHypervector(const char* label, uint32_t* vector, int size) {
    printf("=====================================\n");
    printf("%s\n", label);
    printf("=====================================\n");
    for (int i = 0; i < size; i++) {
        printf("| %08X |\n", vector[i]);
    }
    printf("=====================================\n\n");
}

void printSectionHeader(const char* title) {
    printf("\n=====================================\n");
    printf("   %s\n", title);
    printf("=====================================\n\n");
}
#endif

// Funzione per eseguire il binding software
void softwareBinding(uint32_t* result, uint32_t* vectorA, uint32_t* vectorB, int size) {
    for (int i = 0; i < size; i++) {
        result[i] = vectorA[i] ^ vectorB[i];
    }
}

// Funzione per controllare se i risultati di due ipervettori sono uguali
bool compareResults(uint32_t* vector1, uint32_t* vector2, int size) {
    for (int i = 0; i < size; i++) {
        if (vector1[i] != vector2[i]) {
            return false;
        }
    }
    return true;
}

// Funzione per stampare il risultato del test
void printTestResult(bool result) {
    printf("=====================================\n");
    if (result) {
        printf("\e[32m   *** TEST PASSED ***   \e[39m\n");
    } else {
        printf("\e[31m   *** TEST FAILED ***   \e[39m\n");
    }
    printf("=====================================\n\n");
}

int main() { 
    // Klessydra-T13: 3 hardware threads (hart), IMT
    __asm__("csrw 0x300, 0x8;"); // Enable interrupts for all threads  
    sync_barrier_reset();       
    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) { 
        #ifdef DEBUG
        printSectionHeader("--- HVBIND INSTRUCTION TEST ---");
        #endif
	float f = 0.1;
        printf("f = %f\n", f);
        
        srand(TIME); // Seed for random number generation

        int HV_PART = getRandomValidHVPart();
        int HV_BYTE_SIZE = HV_PART * sizeof(uint32_t);

        uint32_t A[HV_PART] = {0};
        uint32_t B[HV_PART] = {0};
        uint32_t C_SW[HV_PART] = {0};
        uint32_t C_HW[HV_PART] = {0};

        generateRandomHypervector(A, HV_PART);
        generateRandomHypervector(B, HV_PART);

        #ifdef DEBUG
        printSectionHeader("Generated Hypervectors");
        printHypervector("Hypervector A", A, HV_PART);
        printHypervector("Hypervector B", B, HV_PART);
        #endif

        softwareBinding(C_SW, A, B, HV_PART);

        #ifdef DEBUG
        printSectionHeader("Software Binding Result");
        printHypervector("C_SW", C_SW, HV_PART);
        #endif

        CSR_MVSIZE(HV_BYTE_SIZE);
        #ifdef DEBUG
        printSectionHeader("CSR Register Written");
        #endif

        hvmemld((void*)spmaddrA, &A[0], sizeof(A));
        hvmemld((void*)spmaddrB, &B[0], sizeof(B));

        #ifdef DEBUG
        printSectionHeader("Executing HDCU Binding");
        #endif

        hvbind((void*)spmaddrC, (void*)spmaddrA, (void*)spmaddrB);

        hvmemstr(&C_HW[0], (void*)spmaddrC, sizeof(C_HW));

        #ifdef DEBUG
        printSectionHeader("HDCU Binding Result");
        printHypervector("C_HW", C_HW, HV_PART);
        #endif

        bool check = compareResults(C_SW, C_HW, HV_PART);
        printTestResult(check);
    }

    sync_barrier(); 

    return 0;
}
