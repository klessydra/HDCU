//-----------------------------------------------------------------------------------------------------------------------------------------
// File:        VecSimilarity.cpp
// Creator:     Rocco Martino
// Date:        2021-06-30
// Description: This program performs Similarity calculation on hypervectors using both software and HDCU (HyperDimensional Computation Unit). 
//              It utilizes the Klessydra-T13 architecture, featuring 3 hardware threads (harts) with interrupt management.
//              The size of an Hypervector could vary in a very large range (10, 100, 1000, 10000). For this reason we 
//              will memorize the entire hv value in an C array. 
//              This program calculates the Hamming distance between two vectors that correspond to a similarity measure in case of binary hypervectors.
//-----------------------------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ctime>
#include <cstdlib>

#define DEBUG // Abilitare il debug tramite print

extern "C" {			// Klessydra dsp_libraries are written in C and so they're imported as extern:
#include "dsp_functions.h"
#include "functions.h"
}

// Funzione per ottenere un valore di HV_PART tale da ottenere HV di lunghezza potenza di 2 --> 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192
int getRandomValidHVPart() {
    int validValues[] = {1, 2, 4, 8, 16, 32, 64, 128, 256}; // Così gli ipervettori vanno da 32 a 8192
    int size = sizeof(validValues) / sizeof(validValues[0]);
    return validValues[rand() % size];
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

// Funzione per calcolare la similarità in software
int calculateSoftwareSimilarity(uint32_t* vectorA, uint32_t* vectorB, uint32_t* result, int size) {
    int similarity = 0;
    for (int i = 0; i < size; i++) {
        result[i] = vectorA[i] ^ vectorB[i];
        uint64_t x = result[i];
        uint32_t count;
        for (count = 0; x; count++) {
            x &= x - 1; // Conteggio dei bit impostati
        }
        result[i] = count;
        similarity += count;
    }
    return similarity;
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
    // Klessydra-T13: 3 thread hardware (hart), IMT
    __asm__("csrw 0x300, 0x8;"); // Enable interrupts for all threads
    sync_barrier_reset();
    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        printf("\n\e[93m--- HVSIM INSTRUCTION TEST ---\e[39m\n");

        // Seed for random number generation
        srand(TIME);

        // Genera il numero di parti dell'ipervettore
        int HV_PART = getRandomValidHVPart();
        int HV_BYTE_SIZE = HV_PART * sizeof(uint32_t);

        // Dichiarazione degli operandi
        uint32_t A[HV_PART] = {0};
        uint32_t B[HV_PART] = {0};
        uint32_t C_SW[HV_PART] = {0};
        int Similarity_SW = 0;
        int Similarity_HW = 0;

        // Genera ipervettori casuali
        generateRandomHypervector(A, HV_PART);
        generateRandomHypervector(B, HV_PART);

        #ifdef DEBUG
		// Stampa gli ipervettori generati
		printf("\n\e[94mGenerated Hypervectors\e[39m\n");
		printHypervector("A", A, HV_PART);
		printHypervector("B", B, HV_PART);
		printf("\n");
        #endif

        // Esecuzione software
        printf("\n\e[94mSOFTWARE Similarity Result\e[39m\n");
        Similarity_SW = calculateSoftwareSimilarity(A, B, C_SW, HV_PART);
        printf("%08X\n", Similarity_SW);

        // Esecuzione hardware (HDCU)
        printf("\n\e[94mHDCU Similarity Result\e[39m\n");
        CSR_MVSIZE(HV_BYTE_SIZE);

        void* _spmA = (void*)((int*)spmaddrA);
        void* _spmB = (void*)((int*)spmaddrB);
        void* _spmC = (void*)((int*)spmaddrC);

        // Carica gli operandi nella SPM
        hvmemld(_spmA, &A[0], sizeof(A));
        hvmemld(_spmB, &B[0], sizeof(B));

        // Esegui l'operazione di similarità hardware
        hvsim(_spmC, _spmA, _spmB);

        // Scarica i risultati dalla SPM
        hvmemstr(&Similarity_HW, _spmC, sizeof(int));
        printf("%08X\n\n", Similarity_HW);

        // Verifica i risultati e stampa il test
        bool check = (Similarity_SW == Similarity_HW);
        printTestResult(check);
    }

    sync_barrier();

    return 0;
}

