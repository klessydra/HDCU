//-----------------------------------------------------------------------------------------------------------------------------------------
// File:        VecBundling.cpp
// Creator:     Rocco Martino
// Date:        2021-06-30
// Description: This program performs Bundling operations on binary hypervectors using both software and HDCU (HyperDimensional Computation Unit). 
//              It utilizes the Klessydra-T13 architecture, featuring 3 hardware threads (harts) with interrupt management.
//              The size of an Hypervector could vary in a very large range (10, 100, 1000, 10000). For this reason we 
//              will memorize the entire hv value in an C array. 
//              In this example we will execute the hvbundle operation on two hypervectors A and B, storing the result in C.
//-----------------------------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ctime>
#include <cstdlib>

#define DEBUG // Abilitare il debug tramite print
#define COUNTER_BITS 4 // Counters bit precision;

extern "C" {			// Klessydra dsp_libraries are written in C and so they're imported as extern:
#include "dsp_functions.h"
#include "functions.h"
}

// Funzione per generare ipervettori casuali
void generateRandomHypervector(uint32_t* vector, int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand();
    }
}

// Funzione per stampare un ipervettore
void printHypervector(const char* label, uint32_t* vector, int size) {
    printf("%s: ", label);
    for (int i = 0; i < size; i++) {
        printf("%08X ", vector[i]);
    }
    printf("\n");
}

// Funzione per eseguire l'operazione di bundling in software
void softwareBundling(uint32_t* result, uint32_t* vectorA, uint32_t* vectorB, int HV_PART, int HV_BIT_SIZE) {
    uint32_t j = HV_PART - 1;
    for (int i = HV_PART * 4 - 1; i >= 0; i--) {
        uint32_t temp = 0;
        uint32_t b = (vectorB[j] >> (HV_BIT_SIZE / HV_PART - 8 - 8 * i)) & 0xFF;
        if (i % 4 == 0 && i != 0) {
            j--;
        }
        for (int bit = 0; bit < HV_BIT_SIZE / HV_PART; bit += COUNTER_BITS) {
            uint32_t bits_of_A = (vectorA[i] >> bit) & 0xF;
            uint32_t bit_of_B = (b >> (bit / COUNTER_BITS)) & 1;
            uint32_t result_bits = bits_of_A + bit_of_B;
            if (result_bits > 15)
                result_bits -= 16;
            temp |= (result_bits << bit);
        }
        result[i] = temp;
    }
}

// Funzione per confrontare i risultati
bool compareResults(uint32_t* vector1, uint32_t* vector2, int size) {
    for (int i = 0; i < size; i++) {
        if (vector1[i] != vector2[i]) {
            return false;
        }
    }
    return true;
}

// Funzione per eseguire l'output dei risultati
void printTestResult(bool result) {
    if (result) {
        printf("\e[32mTEST PASSED\e[39m\n\n");
    } else {
        printf("\e[31mTEST FAILED\e[39m\n\n");
    }
}

int main() {
    // Klessydra-T13: 3 hardware threads (hart), IMT
    __asm__("csrw 0x300, 0x8;"); // Enable interrupts for all threads
    sync_barrier_reset();
    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        printf("\n\e[93m--- HVBUNDLE INSTRUCTION TEST ---\e[39m\n");

        // Seed for random number generation
        srand(TIME);

        // Genera il numero di parti dell'ipervettore
        int HV_PART = rand() % 9 + 1;
        int HV_BYTE_SIZE = HV_PART * sizeof(uint32_t);
        int HV_BIT_SIZE = HV_BYTE_SIZE * 8;

        // Dichiarazione degli operandi
        uint32_t A[HV_PART * 4] = {0};
        uint32_t B[HV_PART] = {0};
        uint32_t C_SW[HV_PART * 4] = {0};
        uint32_t C_HW[HV_PART * 4] = {0};

        // Genera ipervettori casuali
        generateRandomHypervector(A, HV_PART * 4);
        generateRandomHypervector(B, HV_PART);

        #ifdef DEBUG
        // Stampa gli ipervettori generati
        printf("\n\e[94mGenerated Hypervectors\e[39m\n");
        printHypervector("A", A, HV_PART * 4);
        printHypervector("B", B, HV_PART);
        printf("\n");
        #endif

        // Esecuzione software
        softwareBundling(C_SW, A, B, HV_PART, HV_BIT_SIZE);

        #ifdef DEBUG
        // Stampa i risultati dell'esecuzione software
        printf("\e[94mSoftware Bundling Result\e[39m\n");
        printHypervector("C_SW", C_SW, HV_PART * 4);
        printf("\n");
        #endif

        // Esecuzione hardware (HDCU)
        CSR_MVSIZE(HV_BYTE_SIZE);

        void* _spmA = (void*)((int*)spmaddrA);
        void* _spmB = (void*)((int*)spmaddrB);
        void* _spmC = (void*)((int*)spmaddrC);

        // Carica gli operandi nella SPM
        hvmemld(_spmA, &A[0], sizeof(A));
        hvmemld(_spmB, &B[0], sizeof(B));
        printf("\t\t\t\n"); // Workaround per un bug noto in HDCU, non rimuovere

        // Esegui l'operazione di bundling hardware
        hvbundle(_spmC, _spmA, _spmB);

        // Scarica i risultati dalla SPM
        hvmemstr(&C_HW[0], _spmC, sizeof(C_HW));

        #ifdef DEBUG
        // Stampa i risultati dell'esecuzione hardware
        printf("\e[94mHDCU Bundling Result\e[39m\n");
        printHypervector("C_HW", C_HW, HV_PART * 4);
        printf("\n");
        #endif

        // Verifica i risultati e stampa il test
        bool check = compareResults(C_SW, C_HW, HV_PART * 4);
        printTestResult(check);
    }

    sync_barrier();

    return 0;
}

