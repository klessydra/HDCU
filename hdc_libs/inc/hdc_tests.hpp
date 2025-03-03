#ifndef TESTS_HPP
#define TESTS_HPP
#include "hv_struct.hpp"
#include "hdc_class.hpp"

// --------------------------- Performance counting: --------------------------- 
// Count functions:
inline void start_count();
inline int finish_count();
inline int   perf=0;
inline int*  ptr_perf = &perf;
inline int cycle_inv, cycle_other;

inline void start_count(){
  int enable_perf_cnt=0;
  __asm__("csrrw zero, mcycle, zero;"       // reset cycle count
          "li %[enable], 0x000003E7;"           // enable performance counters
          "csrrw zero, 0x7A0, %[enable]"        // enable performance counters
          :
          :[enable] "r" (enable_perf_cnt)
          );
}

inline int finish_count(){
  __asm__("csrrw zero, 0x7A0, 0x00000000");  // disable performance counters
  __asm__("csrrw %[perf], mcycle, zero;"
          "sw %[perf], 0(%[ptr_perf]);"
          :
          :[perf] "r" (perf),   [ptr_perf] "r" (ptr_perf)
          );
 return perf;
}

// --------------------------- Binding Test: ---------------------------
void test_binding() {
    HV hv1;
    HV hv2;
    hv1.randomize();
    hv2.randomize();
    HV acc_binded_hv;
    CSR_MVSIZE(HV_CHUNKS * 4);

    void* _spmA = (void*)((int*)spmaddrA);
    void* _spmB = (void*)((int*)spmaddrB);
    void* _spmC = (void*)((int*)spmaddrC);
    hvmemld(_spmA, &hv1.chunk[0], sizeof(hv1));
    hvmemld(_spmB, &hv2.chunk[0], sizeof(hv2));


    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);
    printf("\e[91m--- Test BINDING ---\e[39m\n");
    
    start_count();
    HV binded_hv = hdc.bind(hv1, hv2);
    int std_cycle = finish_count();

    start_count();
    hvbind(_spmC, _spmA, _spmB);
    hvmemstr(&acc_binded_hv.chunk[0], _spmC, sizeof(binded_hv));
    int accl_cycle = finish_count();
    
    printf("Standard Execution: %d cycles\n", std_cycle);
    printf("Accelerated Execution: %d cycles\n", accl_cycle);
    printf("Speed Up Factor: %f\n", (float)std_cycle/accl_cycle);
    
    // ---- TEST CHECK ----
    printf("TEST CHECK -->  ");
    bool check = true;
    for (int i = 0; i < HV_CHUNKS; i++) {
        if (binded_hv.chunk[i] != acc_binded_hv.chunk[i]) {
            check = false;
            break;
        }
    }
    if (check)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
}

// --------------------------- Permutation Test: ---------------------------
void test_permutation() {
    HV hv1;
    HV hv2;
    hv1.randomize();
    hv2.randomize();
    HV acc_perm_hv;
    CSR_MVSIZE(HV_CHUNKS * 4);
    int shift_amount = 5; // Example shift amount

    void* _spmA = (void*)((int*)spmaddrA);
    void* _spmB = (void*)((int*)spmaddrB);
    void* _spmC = (void*)((int*)spmaddrC);
    hvmemld(_spmA, &hv1.chunk[0], sizeof(hv1));
    hvmemld(_spmB, &hv2.chunk[0], sizeof(hv2));


    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);
    printf("\e[91m--- Test PERMUTATION ---\e[39m\n");
    
    start_count();
    HV perm_hv = hdc.permutation(hv1, shift_amount);
    int std_cycle = finish_count();

    start_count();
    hvperm(_spmC, _spmA, (void*)shift_amount);
    hvmemstr(&acc_perm_hv.chunk[0], _spmC, sizeof(acc_perm_hv));
    int accl_cycle = finish_count();
    
    printf("Standard Execution: %d cycles\n", std_cycle);
    printf("Accelerated Execution: %d cycles\n", accl_cycle);
    printf("Speed Up Factor: %f\n", (float)std_cycle/accl_cycle);

    // ---- TEST CHECK ----
    printf("TEST CHECK -->  ");
    bool check = true;
    for (int i = 0; i < HV_CHUNKS; i++) {
        if (perm_hv.chunk[i] != acc_perm_hv.chunk[i]) {
            check = false;
            break;
        }
    }
    if (check)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
}

// --------------------------- Bundling Test: ---------------------------
void test_bundling() {
    printf("\e[91m--- Test BUNDLING ---\e[39m\n");

    // ---- Initialization ----
    HV hv1;
    HV hv2;
    BundledHV acc_bundled_hv;
    hv1.randomize();
    hv2.randomize();

    // ---- SPM Initialization ----
    void* _spmA = (void*)((int*)spmaddrA);
    void* _spmB = (void*)((int*)spmaddrB);
    void* _spmC = (void*)((int*)spmaddrC);
    BundledHV bundled_hv;
    bundled_hv = hv1;
    hvmemld(_spmA, &bundled_hv.bundled_chunk[0], sizeof(bundled_hv));
    hvmemld(_spmB, &hv2.chunk[0], sizeof(hv2));
    CSR_MVSIZE(HV_CHUNKS * 4);

    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);
    
    // ---- Standard Bundling ----
    start_count();
    bundled_hv = hdc.bundle(bundled_hv, hv2);
    int std_cycle = finish_count();

    // ---- Accelerated Bundling ----
    start_count();
    hvbundle(_spmC, _spmA, _spmB);
    hvmemstr(&acc_bundled_hv.bundled_chunk[0], _spmC, sizeof(acc_bundled_hv));
    int accl_cycle = finish_count();

    // ---- SPEED UP Evaluation ----
    printf("Standard Execution: %d cycles\n", std_cycle);
    printf("Accelerated Execution: %d cycles\n", accl_cycle);
    printf("Speed Up Factor: %f\n", (float)std_cycle/accl_cycle);

    // ---- TEST CHECK ----
    printf("TEST CHECK -->  ");
    bool check = true;
    for (int i = 0; i < HV_CHUNKS; i++) {
        if (bundled_hv.bundled_chunk[i] != acc_bundled_hv.bundled_chunk[i]) {
            check = false;
            break;
        }
    }
    if (check)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
}

// --------------------------- Clipping Test: ---------------------------
void test_clipping() {
    printf("\e[91m--- Test CLIPPING ---\e[39m\n");
    // ---- Initialization ----
    HV hv1;
    HV hv2;
    hv1.randomize();
    hv2.randomize();
    HV accl_clipped_hv;
    // ---- SPM Initialization ----
    void* _spmA = (void*)((int*)spmaddrA);
    void* _spmB = (void*)((int*)spmaddrB);
    void* _spmC = (void*)((int*)spmaddrC);
    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);

    BundledHV bundled_hv;
    bundled_hv = hv1;
    bundled_hv = hdc.bundle(bundled_hv, hv2);

    //Load in SPM
    hvmemld(_spmA, &bundled_hv.bundled_chunk[0], sizeof(bundled_hv));
    hvmemld(_spmB, &hv2.chunk[0], sizeof(hv2));
    CSR_MVSIZE(HV_CHUNKS * 4);

    // ---- Standard Clipping ----
    start_count();
    HV clipped_hv = hdc.clip(bundled_hv, 2);
    int std_cycle = finish_count();

    // ---- Accelerated Clipping ----
    start_count();
    hvclip(_spmC, _spmA, (void*)2);
    hvmemstr(&accl_clipped_hv.chunk[0], _spmC, sizeof(accl_clipped_hv));
    int accl_cycle = finish_count();

    // ---- SPEED UP Evaluation ----
    printf("Standard Execution: %d cycles\n", std_cycle);
    printf("Accelerated Execution: %d cycles\n", accl_cycle);
    printf("Speed Up Factor: %f\n", (float)std_cycle/accl_cycle);

    // ---- TEST CHECK ----
    printf("TEST CHECK -->  ");
    bool check = true;
    for (int i = 0; i < HV_CHUNKS; i++) {
        if (clipped_hv.chunk[i] != accl_clipped_hv.chunk[i]) {
            check = false;
            break;
        }
    }
    if (check)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
}

// --------------------------- Similarity Test: ---------------------------
void test_similarity() {
    printf("\e[91m--- Test SIMILARITY ---\e[39m\n");
    HV hv1;
    HV hv2;
    hv1.randomize();
    hv2.randomize();
    int acc_sim;
    CSR_MVSIZE(HV_CHUNKS * 4);

    void* _spmA = (void*)((int*)spmaddrA);
    void* _spmB = (void*)((int*)spmaddrB);
    void* _spmC = (void*)((int*)spmaddrC);
    hvmemld(_spmA, &hv1.chunk[0], sizeof(hv1));
    hvmemld(_spmB, &hv2.chunk[0], sizeof(hv2));

    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);

    
    // ---- Standard SIMILARITY ----
    start_count();
    int hamming_distance = hdc.similarity(hv1, hv2);
    int std_cycle = finish_count();

    // ---- Accelerated Similarity ----
    start_count();
    hvsim(_spmC, _spmA, _spmB);
    hvmemstr(&acc_sim, _spmC, sizeof(int));
    int accl_cycle = finish_count();

    // ---- SPEED UP Evaluation ----
    printf("Standard Execution: %d cycles\n", std_cycle);
    printf("Accelerated Execution: %d cycles\n", accl_cycle);
    printf("Speed Up Factor: %f\n", (float)std_cycle/accl_cycle);

    // ---- TEST CHECK ----
    printf("TEST CHECK -->  ");
    if (hamming_distance == acc_sim)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
}

// --------------------------- Associative Search Test: ---------------------------
void test_search() {
    printf("\e[91m--- Test ASS. SEARCH ---\e[39m\n");
    
    HV queryHV;
    HV associativeMemory[HD_CV_LEN];
    
    queryHV.randomize();
    for (int i = 0; i < HD_CV_LEN; i++)
        associativeMemory[i].randomize();
    
    int acc_bestIndex;

    CSR_MVSIZE(HV_CHUNKS * 4);
    CSR_MPSCLFAC(HD_CV_LEN);

    void* _spmA = (void*)((int*)spmaddrA);
    void* _spmB = (void*)((int*)spmaddrB);
    void* _spmC = (void*)((int*)spmaddrC);
    hvmemld(_spmA, &queryHV.chunk[0], sizeof(queryHV));
    hvmemld(_spmB, &associativeMemory[0].chunk[0], sizeof(associativeMemory));

    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);

    
    // ---- Standard SIMILARITY ----
    start_count();
    int std_bestIndex = hdc.Search(queryHV, associativeMemory);
    int std_cycle = finish_count();

    // ---- Accelerated Similarity ----
    start_count();
    kdotp(_spmC, _spmA, _spmB);
    hvmemstr(&acc_bestIndex, _spmC, sizeof(int));
    int accl_cycle = finish_count();

    // ---- SPEED UP Evaluation ----
    printf("Standard Execution: %d cycles\n", std_cycle);
    printf("Accelerated Execution: %d cycles\n", accl_cycle);
    printf("Speed Up Factor: %f\n", (float)std_cycle/accl_cycle);

    // ---- TEST CHECK ----
    printf("TEST CHECK -->  ");
    if (std_bestIndex == acc_bestIndex)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
}


// --------------------------- Encoding Test: ---------------------------
void test_encoding()
{
    printf("\e[91m--- Test ENCODING ---\e[39m\n");
    // Generate the quantization levels
    float quantization_levels[HD_LV_LEN];
    generate_quantization_levels(0.0, 1.0, HD_LV_LEN, quantization_levels);
    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);

    // Test the encoding function
    float feature_vector[DS_FEATURE_SIZE];

    // Generate random elements between 0 and 1:
    for (int i=0;i<DS_FEATURE_SIZE;i++)
        feature_vector[i] = (float)rand()/(float)(RAND_MAX);

    HV base_vectors[DS_FEATURE_SIZE];
    HV level_vectors[HD_LV_LEN];
    BundledHV zero_HV;

    // Generate base and level vectors
    hdc.generate_BaseHVs(base_vectors);
    hdc.generate_LevelVectors(level_vectors);
    
    // Store the base and level vectors in the SPM and initialize the spmD with zeros
    hvmemld((void*)((int*)spmaddrD), &zero_HV.bundled_chunk[0], sizeof(zero_HV));
    for (int i = 0; i < DS_FEATURE_SIZE; i++)
        hvmemld((void*)((int*)spmaddrA+i*HV_CHUNKS * 4), &base_vectors[i].chunk[0], HV_CHUNKS * 4);
    for (int i = 0; i < HD_LV_LEN; i++)
        hvmemld((void*)((int*)spmaddrB+i*HV_CHUNKS * 4), &level_vectors[i].chunk[0], HV_CHUNKS * 4);


    // 1) Compute the quantization level of each feature using get_quantized_level function
    int quantized_features[DS_FEATURE_SIZE];

    for (int i = 0; i < DS_FEATURE_SIZE; i++)
    {
        quantized_features[i] = get_quantized_level(feature_vector[i], quantization_levels, HD_LV_LEN);
        #if DEBUG==1
        printf("Feature %d: -> quantized level: %d\n", i, quantized_features[i]);
        #endif
    }   


    #if DEBUG==1
        printf("Quantized features: ");
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            printf("%d\n", quantized_features[i]);
    #endif

    // Test the encoding function with the standard version
    // start_count();
    HV encoded_hv = hdc.encoding(quantized_features, base_vectors, level_vectors);
    // int std_cycle = finish_count();

    #if DEBUG==1
        printf("Encoded HV -->  ");
        encoded_hv.print();
    #endif
    
    // Test the encoding function with the accelerated version
    // start_count();
    HV accl_encoded_HV = hdc.accl_encoding(quantized_features, spmaddrA, spmaddrB);
    // int accl_cycle = finish_count();

    #if DEBUG==1
        printf("Accl Encoded HV -->  ");
        accl_encoded_HV.print();
    #endif

    // ---- SPEED UP Evaluation ----
    //printf("Standard Execution: %d cycles\n", std_cycle);
    //printf("Accelerated Execution: %d cycles\n", accl_cycle);
    //printf("Speed Up Factor: %f\n", (float)std_cycle/accl_cycle);

    // TEST CHECK
	printf("TEST CHECK -->  ");
    bool passed = true;
	for (int i=0; i< HV_CHUNKS; i++){
		if (encoded_hv.chunk[i] != accl_encoded_HV.chunk[i]){
			passed = false;
			break;
    	}
	}
	if (passed)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");

}

// --------------------------- Temporal Encoding Test: ---------------------------
void test_temporal_encoding()
{
    // printf("\e[91m--- Test TEMPORAL ENCODING, N_GRAM_SIZE:%d ---\e[39m\n", N_GRAM_SIZE);
    // // Generate the quantization levels
    // float quantization_levels[HD_LV_LEN];
    // generate_quantization_levels(0.0, 1.0, HD_LV_LEN, quantization_levels);
    // HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);

    // // Test the encoding function
    // float feature_vector[DS_FEATURE_SIZE][N_GRAM_SIZE] = {{0.1, 0.2, 0.3}, {0.2, 0.3, 0.4}, {0.3, 0.4, 0.5}, {0.4, 0.5, 0.6}};
    // HV base_vectors[DS_FEATURE_SIZE];
    // HV level_vectors[HD_LV_LEN];
    // BundledHV zero_HV;
    // // Generate base and level vectors
    // hdc.generate_BaseHVs(base_vectors);
    // hdc.generate_LevelVectors(level_vectors);
    
    // // Store the base and level vectors in the SPM and initialize the spmD with zeros
    // hvmemld((void*)((int*)spmaddrD), &zero_HV.bundled_chunk[0], sizeof(zero_HV));
    // for (int i = 0; i < DS_FEATURE_SIZE; i++)
    //     hvmemld((void*)((int*)spmaddrA+i*HV_CHUNKS * 4), &base_vectors[i].chunk[0], HV_CHUNKS * 4);
    // for (int i = 0; i < HD_LV_LEN; i++)
    //     hvmemld((void*)((int*)spmaddrB+i*HV_CHUNKS * 4), &level_vectors[i].chunk[0], HV_CHUNKS * 4);

    // start_count();
    // HV encoded_hv = hdc.temporal_encoding(feature_vector, base_vectors, level_vectors, quantization_levels);
    // int std_cycle = finish_count();
    // #if DEBUG==1
    //     printf("Encoded HV -->  ");
    //     encoded_hv.print();
    // #endif

    
    // // Test the encoding function
    // start_count();
    // HV accl_encoded_HV = hdc.accl_temporal_encoding(feature_vector, spmaddrA, spmaddrB, quantization_levels);
    // int accl_cycle = finish_count();
    // #if DEBUG==1
    //     printf("Accl Encoded HV -->  ");
    //     accl_encoded_HV.print();
    // #endif

    // // ---- SPEED UP Evaluation ----
    // printf("Standard Execution: %d cycles\n", std_cycle);
    // printf("Accelerated Execution: %d cycles\n", accl_cycle);

    // // TEST CHECK
	// printf("TEST CHECK -->  ");
    // bool passed = true;
	// for (int i=0; i< HV_CHUNKS; i++){
	// 	if (encoded_hv.chunk[i] != accl_encoded_HV.chunk[i]){
	// 		passed = false;
	// 		break;
    // 	}
	// }
	// if (passed)
    //     printf("\e[32mTEST PASSED\e[39m\n\n");
    // else
    //     printf("\e[31mTEST FAILED\e[39m\n\n");

}

// --------------------------- Training Test: ---------------------------
void test_training()
{
    printf("\e[91m--- Test TRAINING ---\e[39m\n");
    // Generate the quantization levels
    float quantization_levels[HD_LV_LEN];
    generate_quantization_levels(0.0, 1.0, HD_LV_LEN, quantization_levels);
    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);

    // Test the encoding function
    float feature_vector[DS_FEATURE_SIZE];
    // Generate random elements between 0 and 1:
    for (int i=0;i<DS_FEATURE_SIZE;i++)
        feature_vector[i] = (float)rand()/(float)(RAND_MAX);

    HV base_vectors[DS_FEATURE_SIZE];
    HV level_vectors[HD_LV_LEN];
    BundledHV ClassVectors[HD_CV_LEN];
    BundledHV accl_ClassVectors[HD_CV_LEN];
    BundledHV zero_HV;
    // Generate base and level vectors
    hdc.generate_BaseHVs(base_vectors);
    hdc.generate_LevelVectors(level_vectors);
    
    // Store the base and level vectors in the SPM and initialize the spmD with zeros
    hvmemld((void*)((int*)spmaddrD), &accl_ClassVectors[0].bundled_chunk[0], sizeof(accl_ClassVectors));
    for (int i = 0; i < DS_FEATURE_SIZE; i++)
        hvmemld((void*)((int*)spmaddrA+i*HV_CHUNKS * 4), &base_vectors[i].chunk[0], HV_CHUNKS * 4);
    for (int i = 0; i < HD_LV_LEN; i++)
        hvmemld((void*)((int*)spmaddrB+i*HV_CHUNKS * 4), &level_vectors[i].chunk[0], HV_CHUNKS * 4);

    // 1) Compute the quantization level of each feature using get_quantized_level function
    int quantized_features[DS_FEATURE_SIZE];

    for (int i = 0; i < DS_FEATURE_SIZE; i++)
    {
        quantized_features[i] = get_quantized_level(feature_vector[i], quantization_levels, HD_LV_LEN);
        #if DEBUG==1
        printf("Feature %d: -> quantized level: %d\n", i, quantized_features[i]);
        #endif
    }   


    #if DEBUG==1
        printf("Quantized features: ");
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            printf("%d\n", quantized_features[i]);
    #endif

    //start_count();
    BundledHV ClassHV = hdc.training(quantized_features, base_vectors, level_vectors, ClassVectors, 1);
    //int std_cycle = finish_count();
    #if DEBUG==1
        printf("ClassHV  -->  ");
        ClassHV.print();
        printf("Software Training Completed\n"); 
    #endif

    
    // Test the encoding function
    //start_count();
    BundledHV accl_ClassHV = hdc.accl_training(quantized_features, spmaddrA, spmaddrB, accl_ClassVectors, 1);
    //int accl_cycle = finish_count();
    #if DEBUG==1
        printf("Accl ClassHV -->  ");
        accl_ClassHV.print();
        printf("Hardware Training Completed\n");
    #endif

    // ---- SPEED UP Evaluation ----
    // printf("Standard Execution: %d cycles\n", std_cycle);
    // printf("Accelerated Execution: %d cycles\n", accl_cycle);

    // TEST CHECK
    printf("TEST CHECK -->  ");
    bool passed = true;
    for (int i = 0; i < HV_CHUNKS * 4; i++) {
        if (ClassHV.bundled_chunk[i] != accl_ClassHV.bundled_chunk[i]) {
            passed = false;
            break;
        }
    }
    if (passed)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
        

}

// --------------------------- Inference Test: ---------------------------
void test_inference()
{
    printf("\e[91m--- Test INFERENCE ---\e[39m\n");
    // Generate the quantization levels
    float quantization_levels[HD_LV_LEN];
    generate_quantization_levels(0.0, 1.0, HD_LV_LEN, quantization_levels);
    HDC_op hdc(HV_SIZE_BIT, DS_FEATURE_SIZE, HD_LV_LEN);

    float feature_vector[DS_FEATURE_SIZE];
    // Generate random elements between 0 and 1:
    for (int i=0;i<DS_FEATURE_SIZE;i++)
        feature_vector[i] = (float)rand()/(float)(RAND_MAX);

    HV base_vectors[DS_FEATURE_SIZE];
    HV level_vectors[HD_LV_LEN];
    HV ClassVectors[HD_CV_LEN];

    for (int i = 0; i < HD_CV_LEN; i++)
        ClassVectors[i].randomize();
        
    BundledHV zero_HV;
    // Generate base and level vectors
    hdc.generate_BaseHVs(base_vectors);
    hdc.generate_LevelVectors(level_vectors);
    
    // Store the base, level vectors and ClassVectors in the SPM and initialize the spmD with zeros
    hvmemld((void*)((int*)spmaddrD), &zero_HV.bundled_chunk[0], sizeof(zero_HV));
    int class_offset = HV_CHUNKS*4*2;
    for (int i = 0; i < HD_CV_LEN; i++)
        hvmemld((void*)((int*)spmaddrD + class_offset + i*HV_CHUNKS * 4), &ClassVectors[i].chunk[0], HV_CHUNKS * 4);
    for (int i = 0; i < DS_FEATURE_SIZE; i++)
        hvmemld((void*)((int*)spmaddrA+i*HV_CHUNKS * 4), &base_vectors[i].chunk[0], HV_CHUNKS * 4);
    for (int i = 0; i < HD_LV_LEN; i++)
        hvmemld((void*)((int*)spmaddrB+i*HV_CHUNKS * 4), &level_vectors[i].chunk[0], HV_CHUNKS * 4);

    // 1) Compute the quantization level of each feature using get_quantized_level function
    int quantized_features[DS_FEATURE_SIZE];

    for (int i = 0; i < DS_FEATURE_SIZE; i++)
    {
        quantized_features[i] = get_quantized_level(feature_vector[i], quantization_levels, HD_LV_LEN);
        #if DEBUG==1
        printf("Feature %d: -> quantized level: %d\n", i, quantized_features[i]);
        #endif
    }   


    #if DEBUG==1
        printf("Quantized features: ");
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            printf("%d\n", quantized_features[i]);
    #endif

    //start_count();
    int prediction = hdc.inference(quantized_features, base_vectors, level_vectors, ClassVectors);
    //int std_cycle  = finish_count();
    // printf("Software Inference Completed\n"); 
    
    // Test the encoding function
    //start_count();
    int accl_prediction = hdc.accl_inference(quantized_features, spmaddrA, spmaddrB, ClassVectors);
    //int accl_cycle = finish_count();
    // printf("Hardware Inference Completed\n");

    // ---- SPEED UP Evaluation ----
    //printf("Standard Execution: %d cycles\n", std_cycle);
    //printf("Accelerated Execution: %d cycles\n", accl_cycle);
    // TEST CHECK
    printf("TEST CHECK -->  ");
    bool passed = true;
    if (prediction != accl_prediction) {
        passed = false;
    }
    if (passed)
        printf("\e[32mTEST PASSED\e[39m\n\n");
    else
        printf("\e[31mTEST FAILED\e[39m\n\n");
        
}

void clean_SPMs()
{   
    // Create a zero HV and load it in all the SPMs
    BundledHV zero_HV[10];
    hvmemld((void*)((int*)spmaddrA), &zero_HV[0].bundled_chunk[0], sizeof(zero_HV));
    hvmemld((void*)((int*)spmaddrB), &zero_HV[0].bundled_chunk[0], sizeof(zero_HV));
    hvmemld((void*)((int*)spmaddrC), &zero_HV[0].bundled_chunk[0], sizeof(zero_HV));
    hvmemld((void*)((int*)spmaddrD), &zero_HV[0].bundled_chunk[0], sizeof(zero_HV));
}
#endif // TESTS_HPP



