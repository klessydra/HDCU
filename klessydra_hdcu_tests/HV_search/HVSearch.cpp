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

extern "C" {			// Klessydra dsp_libraries are written in C and so they're imported as extern:
#include "dsp_functions.h"
#include "functions.h"
}

// Funzione per ottenere un valore di HV_PART tale da ottenere HV di lunghezza potenza di 2 --> 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192
int getRandomValidHVPart() {
    int validValues[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};  // Per ottenere la dimensione in bit bisogna moltiplicare per 32
    int size = sizeof(validValues) / sizeof(validValues[0]); // Numero di elementi dell'array
    return validValues[rand() % size];                       // Restituisce un valore casuale tra quelli validi
}

int main()
{
  // Klessydra-T13: 3 thread hardware (hart), IMT
  __asm__("csrw 0x300, 0x8;"); // Enable interrupts for all threads
  sync_barrier_reset();
  sync_barrier_thread_registration();

  if (Klessydra_get_coreID() == 0)
  {
    printf("\n\e[93mHVSEARCH INSTRUCTION TEST\e[39m\n");

    // Seed for random number generation
    srand(TIME);
    
    // Generate a random number of part in which the hypervector is divided;
    int HV_PART = getRandomValidHVPart();
    int HV_BYTE_SIZE = HV_PART*sizeof(uint32_t);

    // Number of classes in the associative memory
    int CLASS_NUMBER = 4;

    // Definition of HVs in associative memory aka Class Vectors;
    uint32_t A[HV_PART] = {0};
    uint32_t B[HV_PART] = {0};
    uint32_t C[HV_PART] = {0};
    uint32_t D[HV_PART] = {0};
    // uint32_t E[HV_PART] = {0};
    // uint32_t F[HV_PART] = {0};
    // uint32_t G[HV_PART] = {0};
    // uint32_t H[HV_PART] = {0};

    // Definition of the associative memory
    uint32_t associativeMemory[HV_PART * CLASS_NUMBER] = {0};
    
    // Definition of the encoded HV which will be compared with the other hypervectors in the associative memory
    uint32_t encoded_HV[HV_PART] = {0};

    // Temporary XOR result
    uint32_t XOR_result[HV_PART] = {0};

    // Temporary similarity result
    int Similarity_SW = 0;
    int bestSimilarity_SW = 8192;
    int bestSimilarity_HW = 0;
    int bestClassIndex_SW = -1;
    int bestClassIndex_HW = 0;

    // Generate random hypervectors
    for (int i=0;i<HV_PART;i++){
        A[i] = rand();
        B[i] = rand();
        C[i] = rand();
        D[i] = rand();
        // E[i] = rand();
        // F[i] = rand();
        // G[i] = rand();
        // H[i] = rand();

        encoded_HV[i] = rand();
    }

    // // Print random hypervectors in HEX
    // printf("\nRandom generate hypervectors:\n\n");
    // for (int i=0;i<HV_PART;i++){
    //   if (i == 0)
    //     printf("A = %08X", A[i]);
    //   else
    //     printf("    %08X", A[i]); 
    //   if (i == HV_PART-1)
    //     printf("\n");
    // }
    // for (int i=0;i<HV_PART;i++){
    //   if (i == 0)
    //     printf("B = %08X", B[i]);
    //   else
    //     printf("    %08X", B[i]);
    //   if (i == HV_PART-1)
    //     printf("\n");
    // }
    // for(int i=0;i<HV_PART;i++){
    //   if(i == 0)
    //     printf("C = %08X", C[i]);
    //   else
    //     printf("    %08X", C[i]);
    //   if(i == HV_PART-1)
    //     printf("\n");
    // }
    // for(int i=0;i<HV_PART;i++){
    //   if(i == 0)
    //     printf("D = %08X", D[i]);
    //   else
    //     printf("    %08X", D[i]);
    //   if(i == HV_PART-1)
    //     printf("\n");
    // }
    // printf("\n");

    // Definition of the associative memory
    for (int i = 0; i < HV_PART; i++) {
        associativeMemory[i] = A[i];
        associativeMemory[i + 1 * HV_PART] = B[i];
        associativeMemory[i + 2 * HV_PART] = C[i];
        associativeMemory[i + 3 * HV_PART] = D[i];
        // associativeMemory[i + 4 * HV_PART] = E[i];
        // associativeMemory[i + 5 * HV_PART] = F[i];
        // associativeMemory[i + 6 * HV_PART] = G[i];
        // associativeMemory[i + 7 * HV_PART] = H[i];
    }

    // // Print associative memory hypervectors in HEX
    // for (int i=0; i< HV_PART*CLASS_NUMBER; i++){
    //   if (i == 0)
    //     printf("\nAssociative Memory:\n\n");
    //   if (i % HV_PART == 0 && i != 0)
    //     printf("\n");
    //   printf(" %08X", associativeMemory[i]);
    //   if (i % HV_PART == HV_PART-1)
    //     printf("\n");
    //   else
    //     printf("    ");
    // }

    // // Print encoded hypervector in HEX
    // printf("\nEncoded hypervector:\n\n");
    // for (int i=0;i<HV_PART;i++){
    //   if (i == 0)
    //     printf(" %08X", encoded_HV[i]);
    //   else
    //     printf("     %08X", encoded_HV[i]);
    //   if (i == HV_PART-1)
    //     printf("\n");
    // }

    // Stampo informazioni utili per il test: numero di classi, dimensione dell'ipervettore, numero di parti in cui è diviso l'ipervettore
    printf("\nCLASS_NUMBER = %d\n", CLASS_NUMBER);
    printf(  "HV_BIT_SIZE  = %d\n", HV_BYTE_SIZE*8);  
    printf(  "HV_BYTE_SIZE = %d\n", HV_BYTE_SIZE);
    printf(  "HV_PART      = %d\n", HV_PART);


    //------------ SOFTWARE EXECUTION ------------
    printf("\nSOFTWARE EXECUTION --> \n");
    // Calcolo la xor tra l'ipervettore codificato e tutti gli ipervettori nella memoria associativa
    for (int j = 0; j < CLASS_NUMBER; j++) {
      for (int i = 0; i < HV_PART; i++) {
        XOR_result[i] = associativeMemory[j * HV_PART + i] ^ encoded_HV[i];
      }
      for (int i = 0; i < HV_PART; i++) {
        uint64_t x = XOR_result[i];
        int count;
        for (count = 0; x; count++) {
          x &= x - 1;
        }
        XOR_result[i] = count;
        Similarity_SW += XOR_result[i];
      }
      printf("Similarity SW for class %d = %08X\n", j+1, Similarity_SW);  // Stampa la similarità per la classe corrente

      if (Similarity_SW < bestSimilarity_SW) {
        bestSimilarity_SW = Similarity_SW;
        bestClassIndex_SW = j;  // Memorizza l'indice della classe con la migliore similarità
      }
      Similarity_SW = 0;
    }

    printf("\nBest Similarity SW = %08X\n", bestSimilarity_SW);  // Stampa la migliore similarità trovata
    printf("Class index with best similarity = %d\n", bestClassIndex_SW);  // Stampa l'indice della classe con la migliore similarità

    //------------ HDCU EXECUTION ------------
    printf("\nHDCU EXECUTION --> \n");

    // We need to specify the size of the hypervector in the CSR_MVSIZE register
    CSR_MVSIZE(HV_BYTE_SIZE); 

    // We need to specify the number of classes in the CSR_MVTYPE register
    CSR_MPSCLFAC(CLASS_NUMBER);

    void *_spmA = (void *)((int *)spmaddrA);
    void *_spmB = (void *)((int *)spmaddrB);
    void *_spmC = (void *)((int *)spmaddrC);

    // Store of Associative Memory in the SPM A
    hvmemld(_spmA, &associativeMemory, sizeof(associativeMemory));
    printf("\t\t\t");
    // Store the encoded hypervector in the SPM B
    hvmemld(_spmB, &encoded_HV, sizeof(encoded_HV));
    printf("\t\t\t\n");
    // Perform the search operation
    kdotp(_spmC, _spmB, _spmA); 
    
    // Load the result from the SPM to the stack  
    hvmemstr(&bestClassIndex_HW, _spmC, sizeof(int));

    //Print the result
    printf("Class index with best similarity = %d\n", bestClassIndex_HW);  // Stampa l'indice della classe con la migliore similarità

    // Check if the results are the same
    printf("\nTEST RESULT --> ");
    if(bestClassIndex_SW == bestClassIndex_HW)
      printf("\e[92mPASSED\e[39m\n");
    else
      printf("\e[91mFAILED\e[39m\n");
  }

  sync_barrier();

  return 0;
}



