//-----------------------------------------------------------------------------------------------------------------------------------------
// File:        HVPermutation.cpp
// Creator:     Rocco Martino
// Date:        2021-06-01
// Description: This file tests the hvperm instruction used to shift the hypervector by a given number of positions specified by the second
//              operand. The permutation is circular, so the LSBs are shifted to the MSBs. 
//-----------------------------------------------------------------------------------------------------------------------------------------


#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ctime>
#include <cstdlib>

extern "C" {            // Klessydra dsp_libraries are written in C and so they're imported as extern:
#include "dsp_functions.h"
#include "functions.h"
}

// Funzione per ottenere un valore di HV_PART tale da ottenere HV di lunghezza potenza di 2 --> 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192
int getRandomValidHVPart() {
    int validValues[] = {1, 2, 4}; // Cosi gli ipervettori vanno da 32 a 8192
    int size = sizeof(validValues) / sizeof(validValues[0]);
    return validValues[rand() % size];
}

// Funzione per eseguire lo shift ciclico a destra
void right_rotate_array(uint32_t* array, int size, int shift) {
    if (size == 0) return;
    
    // Il numero effettivo di bit da shiftare
    int effective_shift = shift % (size * 32);
    
    if (effective_shift == 0) return;

    // Numero di bit residui da shiftare
    int bit_shift = effective_shift;

    // Array temporaneo per contenere il risultato
    uint32_t temp[size];
    
    // Copia dell'array originale nel temporaneo
    for (int i = 0; i < size; i++) {
        temp[i] = array[i];
    }

    // Shift dei bit per ogni elemento dell'array
    for (int i = 0; i < size; i++) {
        // Combina i bit dell'elemento corrente e del successivo
        uint32_t lower_bits = temp[i] >> bit_shift;
        uint32_t upper_bits = temp[(i + size - 1) % size] << (32 - bit_shift);
        array[i] = lower_bits | upper_bits;
    }
}

int main()
{
  // Klessydra-T13: 3 thread hardware (hart), IMT
  __asm__("csrw 0x300, 0x8;"); // Enable interrupts for all threads
  sync_barrier_reset();
  sync_barrier_thread_registration();

  if (Klessydra_get_coreID() == 0)
  {
    printf("\n\e[93mHVPERM INSTRUCTION TEST\e[39m\n");

    int shift_amount = 4; // Quantità di shift (ad esempio 4 bit)
    printf("\nShift amount: %d\n", shift_amount);
    
    // Seed for random number generation
    srand(TIME);
    
    // Generate a random number of part in which the hypervector is divided in range 1-10
    int HV_PART = getRandomValidHVPart();
    int HV_BYTE_SIZE = HV_PART*sizeof(uint32_t);

    // Definition of operand;
    uint32_t A[HV_PART] = {0};    // Hypervector
    uint32_t C_SW[HV_PART] = {0}; // Software result
    uint32_t C_HW[HV_PART] = {0}; // HDCU result

    // Generate random hypervectors
    for (int i=0;i<HV_PART;i++){
        A[i] = rand();
    }
    // Print random hypervectors in HEX
    printf("\nRandom generate hypervectors:\n");
    for (int i=0;i<HV_PART;i++){
      if (i == 0)
        printf("A = %08X", A[i]);
      else
        printf("       %08X", A[i]);
      if (i == HV_PART-1)
        printf("\n");
    }
    printf("\n");

    //------------ SOFTWARE EXECUTION ------------
    printf("\nSOFTWARE EXECUTION --> \n");

    // Esegui lo shift circolare sull'array A e salva il risultato in C_SW
    
    for (int i = 0; i < HV_PART; i++) {
        C_SW[i] = A[i];
    }
    right_rotate_array(C_SW, HV_PART, shift_amount);

    // Print the result
    for (int i=0;i<HV_PART;i++){
      if (i == 0)
        printf("C = %08X", C_SW[i]);
      else
        printf("       %08X", C_SW[i]);
      if (i == HV_PART-1)
        printf("\n");
    }
    printf("\n");

    //------------ HDCU EXECUTION ------------
    printf("\nHDCU EXECUTION --> \n");

    // We need to specify the size of the hypervector in the CSR_MVSIZE register
    CSR_MVSIZE(HV_BYTE_SIZE); 

    void *_spmA = (void *)((int *)spmaddrA);
    void *_spmB = (void *)((int *)spmaddrB);
    void *_spmC = (void *)((int *)spmaddrC);

    // Load operands. The vector size is the third operand and is expressed in bytes
    hvmemld(_spmA, &A, sizeof(A));
    printf("C = "); // This is a workaround to avoid a bug in the HDCU, do not delete this line

    // Perform the permutation
    hvperm(_spmC, _spmA, (void*)shift_amount); 

    // Load the result from the SPM to the stack  
    hvmemstr(&C_HW[0], _spmC, sizeof(C_HW));

    // Print the result
    for (int i=0;i<HV_PART;i++){
      if (i == 0)
        printf("%08X", C_HW[i]);
      else
        printf("       %08X", C_HW[i]);
      if (i == HV_PART-1)
        printf("\n");
    }
    printf("\n");

    bool success = true;
    
    // Check if the results are the same
    for (int i=0;i<HV_PART;i++){
      if (C_SW[i] != C_HW[i]){
        success = false;
      }
    }

    if (success)
      printf("\n\e[92m PASSED\e[39m\n");
    else
      printf("\n\e[91m FAILED\e[39m\n");
  }

  sync_barrier();

  return 0;
}
