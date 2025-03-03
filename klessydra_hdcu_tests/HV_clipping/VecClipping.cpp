
//-----------------------------------------------------------------------------------------------------------------------------------------
// File:        VecClipping.cpp
// Creator:     Rocco Martino
// Date:        2021-06-30
// Description: This program performs Similarity calculation on hypervectors using both software and HDCU (HyperDimensional Computation Unit). 
//              It utilizes the Klessydra-T13 architecture, featuring 3 hardware threads (harts) with interrupt management.
//              The size of an Hypervector could vary in a very large range (10, 100, 1000, 10000). For this reason we 
//              will memorize the entire hv value in an C array. 
//              This program calculates clip an hypervector that represent an output of a bundling operation. 
//              In the following program there will be a bundling operation followed by a clipping operation.
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
 
#define COUNTER_BITS 4  // Precision of the counter;         

int main()
{
  // Klessydra-T13: 3 hardware threads (hart), IMT
  __asm__("csrw 0x300, 0x8;"); // Enable interrupts for all threads
  sync_barrier_reset();
  sync_barrier_thread_registration();

  if (Klessydra_get_coreID() == 0)
  {
    printf("\e[93mHVCLIP INSTRUCTION TEST\e[39m\n");
  
    // Seed for random number generation
    srand(TIME);
    
    // Generate a random number of part in which the hypervector is divided in range 1-10
    int HV_PART = rand() % 8 + 1;
    int HV_BYTE_SIZE = HV_PART*sizeof(uint32_t);
    int HV_BIT_SIZE = HV_BYTE_SIZE*8;

    // Definition of operands;
    uint32_t A[HV_PART*4] = {0};
    uint32_t B[HV_PART] = {0};
    uint32_t C_SW[HV_PART*4] = {0}; // Software bundling result
    uint32_t C_HW[HV_PART*4] = {0}; // HDCU bundling result
    uint32_t D_SW[HV_PART] = {0};   // Software clipping result
    uint32_t D_HW[HV_PART] = {0};   // HDCU clipping result

    // Generate random hypervectors
    for (int i=0;i<HV_PART*4;i++){
      A[i] = rand();
    }
    for (int i=0;i<HV_PART;i++){
      B[i] = rand();
    }

    // // Print random hypervectors in HEX
    // printf("\nRandom generate hypervectors:\n");
    // for (int i=0;i<HV_PART*4;i++){
    //   if (i == 0)
    //     printf("A = %08X", A[i]);
    //   else
    //     printf("    %08X", A[i]);
    //   if (i == HV_PART*4-1)
    //     printf("\n");
    // }
    // printf("\n");
    // for (int i=0;i<HV_PART;i++){
    //   if (i == 0)
    //     printf("B = %08X", B[i]);
    //   else
    //     printf("    %08X", B[i]);
    //   if (i == HV_PART-1)
    //     printf("\n");
    // }
    // printf("\n");

    //------------ SOFTWARE EXECUTION ------------

    // printf("\nSOFTWARE EXECUTION --> \n");

    // Bundling operation
    int HV_BUNDLED = 0; // Variable to store the greatest value of the hypervector after the bundling operation
    uint32_t j = HV_PART-1;
    for (int i = HV_PART*4-1; i >= 0; i--) {                            // Iterate over the hypervector  (from the last to the first element)
      uint32_t temp = 0;                                                // Temporary variable to build the new value
      uint32_t b = (B[j] >>  (HV_BIT_SIZE/HV_PART - 8 - 8*i)) & 0xFF;   // Extract 8 bit from B starting from the least significant bit
      if(i%4 == 0 && i != 0){                                           // Increment j every 4 iterations
        j--;                                       
      }
      for (int bit = 0; bit < HV_BIT_SIZE/HV_PART; bit += COUNTER_BITS) {
        uint32_t bits_of_A = (A[i] >> bit) & 0xF;                       // Extract 4 bits from A
        if (bits_of_A > HV_BUNDLED) {                                   // Chek if the bits_of_A is the greatest value
          HV_BUNDLED = bits_of_A;
        }
        uint32_t bit_of_B = (b >> (bit / COUNTER_BITS)) & 1;            // Extract 1 bit from B
        uint32_t result_bits = bits_of_A + bit_of_B;                    // Sum 4 bits of A with 1 bit of B
        if (result_bits > 15)                                           // If the sum is greater than 15, subtract 15                  
          result_bits -= 16;                                                              
        temp |= (result_bits << bit);                                   // Add the modified bits to temp, shifted to the correct position
      }
      C_SW[i] = temp;                                                      // Assign the result to C[i]
    }                           

    // Defining the MAJORITY_THRESHOLD
    int MAJORITY_THRESHOLD = ceil(HV_BUNDLED/2);

    // Clipping operation
    int k = 0;
    j = (HV_BIT_SIZE/HV_PART)-1;                                        // Initialize j to the most significant bit
    for (int i = 0; i < HV_PART*4; i++){
      if(i%4 == 0 && i != 0){                                           // Increment k every 4 iterations
        k++;   
        j = (HV_BIT_SIZE/HV_PART)-1;                                    // Reset j to the most significant bit                                    
      }
      for (int bit = 0; bit < HV_BIT_SIZE/HV_PART; bit += COUNTER_BITS) {
        uint32_t bits_of_C = ((C_SW[i] << bit) & 0xF0000000) >> 28;     // Extract 4 bits from C starting from the most significant bit
        if (bits_of_C > MAJORITY_THRESHOLD) {
          D_SW[k] |= (1 << j);                   
        }
        j--;
      }
    }

    // printf("\e[93mBUNDLING RESULT\e[39m\n");
        
    // for (int i=0;i<HV_PART*4;i++){
    //   if (i == 0)
    //     printf("C = %08X", C_SW[i]);
    //   else
    //     printf("    %08X", C_SW[i]);
    //   if (i == HV_PART*4-1)
    //     printf("\n");
    // }   

    // printf("\n\e[93mCLIPPING RESULT\e[39m\n");

    // for (int i=0;i<HV_PART;i++){
    //   if (i == 0)
    //     printf("D = %08X", D_SW[i]);
    //   else
    //     printf("    %08X", D_SW[i]);
    //   if (i == HV_PART-1)
    //     printf("\n");
    // }
    // printf("\n");

    //------------ VCU EXECUTION ------------
    
    // printf("\nHDCU EXECUTION --> \n");

    // Declare the size of the hypervectors in bytes;
    CSR_MVSIZE(HV_BYTE_SIZE);  

    void *_spmA = (void *)((int *)spmaddrA);
    void *_spmB = (void *)((int *)spmaddrB);
    void *_spmC = (void *)((int *)spmaddrC);

    // Load operands from the stack to the SPM. The size of the vector is the third operand and is expressed in bytes;
    hvmemld(_spmA, &A, sizeof(A));
    hvmemld(_spmB, &B, sizeof(B));
    printf("Test "); // This is a workaround to avoid a bug in the HDCU, do not delete this line

    // Vector Bundling;
    hvbundle(_spmC, _spmA, _spmB); 

    // Load the result from the SPM to the stack    
    hvmemstr(&C_HW[0], _spmC, sizeof(C_HW));        
    printf("Result: "); // This is a workaround to avoid a bug in the HDCU, do not delete this line
    
    // printf("\e[93mBUNDLING RESULT\e[39m\n");

    // for (int i=0;i<HV_PART*4;i++){
    //   if (i == 0)
    //     printf("C = %08X", C_HW[i]);
    //   else
    //     printf("    %08X", C_HW[i]);
    //   if (i == HV_PART*4-1)
    //     printf("\n");
    // }
    // printf("\n");
  
    // Vector Clipping;
    hvclip(_spmC, _spmC, (void*)HV_BUNDLED);     

    // Load the result from the SPM to the stack    
    hvmemstr(&D_HW[0], _spmC, sizeof(D_HW));       
    
    // printf("\e[93mCLIPPING RESULT\e[39m\n");
    // for (int i=0;i<HV_PART;i++){
    //   if (i == 0)
    //     printf("D = %08X", D_HW[i]);
    //   else
    //     printf("    %08X", D_HW[i]);
    //   if (i == HV_PART-1)
    //     printf("\n");
    // }
    // printf("\n");

    // Check if the results are the same
    bool check = true;
    for (int i=0;i<HV_PART;i++){
      if (D_SW[i] != D_HW[i]){
        check = false;
        break;
      }
    }
    if (check)
      printf("\e[32mPASSED\e[39m\n\n");
    else
      printf("\e[31mFAILED\e[39m\n\n");
  }

  sync_barrier();

  return 0;
}

