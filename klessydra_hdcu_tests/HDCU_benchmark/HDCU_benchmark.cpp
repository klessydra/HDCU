#include "hdc_tests.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    __asm__("csrw 0x300, 0x8;"); // Enable interrupts for all threads
    sync_barrier_reset();
    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0)
    {
        // Print a nicer header
        printf("\e[93m");
        printf("*********************************************\n");
        printf("*                                           *\n");
        printf("*              \e[94mHDCU TEST SUITE\e[93m              *\n");
        printf("*                                           *\n");
        printf("*        Features: \e[92m%-4d\e[93m                     *\n", DS_FEATURE_SIZE);
        printf("*        Classes:  \e[92m%-4d\e[93m                     *\n", HD_CV_LEN);
        printf("*        HV Size:  \e[92m%-4d bits\e[93m                *\n", HV_SIZE_BIT);
        printf("*                                           *\n");
        printf("*********************************************\e[39m\n\n");

        
        clean_SPMs();
        test_binding();
        clean_SPMs();
        test_permutation();
        clean_SPMs();
        test_bundling();
        clean_SPMs();
        test_clipping();
        clean_SPMs();
        test_similarity();
        clean_SPMs();
        test_search();   
        clean_SPMs();
        test_encoding();
        // clean_SPMs();
        // test_temporal_encoding();
        clean_SPMs();
        test_training();
        clean_SPMs();
        test_inference();
        clean_SPMs();
     }

    sync_barrier();

    return 0;
}
