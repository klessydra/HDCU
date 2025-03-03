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
  __asm__("csrrw zero, mcycle, zero;"           // reset cycle count
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

// Function to generate the quantization levels
void generate_quantization_levels(float min, float max, int levels, float LevelList[HD_LV_LEN]) {
    double length = max - min;
    double gap = length / levels;
    for (int level = 0; level < levels - 1; ++level) {
        LevelList[level] = min + level * gap;
    }
    LevelList[levels - 1] = max;
}

// Function to get the quantized level for a given value
int get_quantized_level(float value, float quantization_levels[HD_LV_LEN], int levels) {
    for (int i = 0; i < levels; ++i) {
        if (value <= quantization_levels[0]) {
            return 0;
        }
        if (value <= quantization_levels[i]) {
            return i - 1;
        }
    }
    return levels - 1;
}

// Constructor
HDC_op::HDC_op(int dimensionality, int features, int levels) {
    HV_SIZE = dimensionality;
    num_levels = levels;
    num_features = features;
}

// Base HVs
void HDC_op::generate_BaseHVs(HV baseVectors[DS_FEATURE_SIZE]) {
    for (int vec = 0; vec < DS_FEATURE_SIZE; vec++)
        baseVectors[vec].randomize();
}

// Similarity
int HDC_op::similarity(HV HV1, HV HV2) {
    HV xor_HV;
    int hammingDistance = 0;

    for (int i = 0; i < HV_SIZE / 32; i++)
        xor_HV.chunk[i] = HV1.chunk[i] ^ HV2.chunk[i];

    for (int i = 0; i < HV_SIZE / 32; i++) {
        long long x = xor_HV.chunk[i];
        int count;
        for (count = 0; x; count++)
            x &= x - 1;
        xor_HV.chunk[i] = count;
    }
    for (int i = 0; i < HV_SIZE / 32; i++) {
        hammingDistance += xor_HV.chunk[i];
    }
    return hammingDistance;
}

// Binding
HV HDC_op::bind(HV HV1, HV HV2) {
    HV Binded_HV;
    for (int i = 0; i < HV_CHUNKS; i++) {
        Binded_HV.chunk[i] = HV1.chunk[i] ^ HV2.chunk[i];
    }
    return Binded_HV;
}

// Permutation
HV HDC_op::permutation(HV hv, int shift) {
    // The number of bits to shift within the bounds of the HV size
    int effective_shift = shift % (HV_CHUNKS * 32);

    if (effective_shift == 0) return hv;

    // Temporary array to store the original values of HV chunks
    uint32_t temp[HV_CHUNKS];

    // Copy the HV chunks into the temporary array
    for (int i = 0; i < HV_CHUNKS; i++) {
        temp[i] = hv.chunk[i];
    }

    // Shift bits within each chunk and carry over the overflow bits
    uint32_t overflow_bits = 0;
    uint32_t new_overflow_bits;
    for (int i = 0; i< HV_CHUNKS; i++) {
        new_overflow_bits = temp[i] << (32 - effective_shift);
        hv.chunk[i] = (temp[i] >> effective_shift) | overflow_bits;
        overflow_bits = new_overflow_bits;
    }

    // Integrate the last shifted bits into the first chunk
    hv.chunk[0] |= overflow_bits;

    return hv;
}

// Bundling
BundledHV HDC_op::bundle(const BundledHV& HV1, const HV& HV2) const {
    int j = HV_CHUNKS - 1;
    BundledHV Bundled_HV;
    for (int i = HV_CHUNKS * 4 - 1; i >= 0; i--) {
        int temp = 0;

        int shift_amount = 32 - 8 - 8 * (i % 4);
        int b = (HV2.chunk[j] >> shift_amount) & 0xFF;
        if (i % 4 == 0 && i != 0) {
            j--;
        }
        for (int bit = 28; bit >= 0; bit -= COUNTER_BITS) {
            int bits_of_A = (HV1.bundled_chunk[i] >> bit) & 0xF;
            int bit_of_B = (b >> (bit / COUNTER_BITS)) & 1;
            int result_bits = bits_of_A + bit_of_B;
            if (result_bits > 15)
                result_bits -= 16;
            temp |= (result_bits << bit);
        }
        Bundled_HV.bundled_chunk[i] = temp;
    }
    return Bundled_HV;
}

// Clipping
HV HDC_op::clip(const BundledHV& bundled_hv, int HV_BUNDLED) {
    int MAJORITY_THRESHOLD = (HV_BUNDLED / 2);
    HV D_SW;

    int k = 0;
    int j = 31;
    for (int i = 0; i < HV_CHUNKS * 4; i++) {
        if (i % 4 == 0 && i != 0) {
            k++;
            j = 31;
        }
        for (int bit = 28; bit >= 0; bit -= COUNTER_BITS) {
            int bits_of_C = (bundled_hv.bundled_chunk[i] >> bit) & 0xF;
            if (bits_of_C > MAJORITY_THRESHOLD) {
                D_SW.chunk[k] |= (1 << j);
            }
            j--;
        }
    }
    return D_SW;
}

// Search
int HDC_op::Search(HV QueryHV, HV associativeMemory[HD_CV_LEN]) {
    
    HV xor_HV;
    int hammingDistance = 0;
    int bestDistance = 10000;
    int bestIndex = 0;

    for (int j = 0; j < HD_CV_LEN; j++) {
        for (int i = 0; i < HV_SIZE / 32; i++){
            xor_HV.chunk[i] = associativeMemory[j].chunk[i] ^ QueryHV.chunk[i];
        }
        for (int i = 0; i < HV_SIZE / 32; i++) {
            long long x = xor_HV.chunk[i];
            int count;
            for (count = 0; x; count++){
                x &= x - 1;
            }
            xor_HV.chunk[i] = count;
        }
        for (int i = 0; i < HV_SIZE / 32; i++) {
            hammingDistance += xor_HV.chunk[i];
        }
        if (hammingDistance < bestDistance) {
            bestDistance = hammingDistance;
            bestIndex = j;
        }
        hammingDistance = 0;
    }

    return bestIndex;
}


// --------------------Level HVs----------------------
void HDC_op::generate_LevelVectors(HV LevelVectors[HD_LV_LEN])
{                   
    // Linear encoding
    // the first level vector is randomly initialized 
    int change_ratio;
    change_ratio = HV_SIZE_BIT / 2;   
    int indexVector[HV_SIZE_BIT];
    for (int i = 0; i < HV_SIZE_BIT; i++)
        indexVector[i] = 1;

    // Flipping random change_ratio bits
    LevelVectors[0].randomize();

    // The other level vectors are obtained flipping a number of bits equal to int(HD_DIM / (2 * totalLevel))
    // starting from the previous level vector. However, the same element can not be flipped 2 times
    change_ratio = HV_SIZE_BIT / (2 * num_levels);
    for (int level = 1; level < num_levels; level++)
    {
        LevelVectors[level] = LevelVectors[level - 1];
        int i=0;
        while (i < change_ratio)
        {
            int index = rand() % HV_SIZE_BIT;
            if (indexVector[index] == 1)
            {                
                indexVector[index] = 0;
                i++;

                if (LevelVectors[level - 1].chunk[index / 32] & (1 << (index % 32))) {
                    // If the bit is 1, set it to 0
                    LevelVectors[level].chunk[index / 32] &= ~(1 << (index % 32));
                } else {
                    // If the bit is 0, set it to 1
                    LevelVectors[level].chunk[index / 32] |= 1 << (index % 32);
                }


            }
        }
    }

} 

// --------------------SPATIAL ENCODING----------------------
// From a given input feature vector, generate the corresponding HDC vector
// This is done as follows:
// 1) Compute the quantization level of each feature
// 2) Generate the corresponding level vector
// 3) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
// 4) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
// Input: quantized_features, BaseVectors, LevelVectors, quantized_levels (array of thresholds for each quantization level)
// Output: HDC vector
HV HDC_op::encoding(int quantized_features[DS_FEATURE_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN])
{
    BundledHV Encoded_HV;

    #if DEBUG==1 
        printf("\e[92m----------\n\e[39m");
        printf("Encoding...\n");
        printf("Feature vector: ");
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            printf("%f ", quantized_features[i]);
        printf("\n");
    #endif
        
    start_count();
    // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
    HV binded_feature[DS_FEATURE_SIZE];
    for (int i = 0; i < DS_FEATURE_SIZE; i++)
    {
        binded_feature[i] = this->bind(LevelVectors[quantized_features[i]], BaseVectors[i]);
        #if DEBUG==1
            printf("Binding...\n");
            printf("Level vector %d: ", quantized_features[i]);
            LevelVectors[quantized_features[i]].print();
            printf("\n");
            printf("Base vector %d: ", i);
            BaseVectors[i].print();
            printf("\n");
            printf("Binded level vector %d with base vector %d: ", quantized_features[i], i);
            binded_feature[i].print();
            printf("\n");
        #endif
    }
    
    // 3) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        Encoded_HV = this->bundle(Encoded_HV, binded_feature[i]);
        #if DEBUG==1
        printf("Accumulated FeatureHV %d through bundling\n", i);
        #endif
    }

    #if DEBUG==1
        printf("Encoded HV: ");
        Encoded_HV.print();
    #endif

    // 4) Clip the HDC vector
    HV Clipped_HV;
    Clipped_HV = this->clip(Encoded_HV,DS_FEATURE_SIZE);
    #if DEBUG==1
        printf("Clipped HV: ");
        Clipped_HV.print();
    #endif
    int std_cycle = finish_count();
    printf("Standard Execution: %d cycles\n", std_cycle);

    // 4) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    return Clipped_HV;
}


HV HDC_op::accl_encoding(int quantized_features[DS_FEATURE_SIZE], int bv_start_addr, int lv_start_addr)
{
    BundledHV Encoded_HV;
    
    #if DEBUG==1 
        printf("\e[92m----------\n\e[39m");
        printf("Encoding...\n");
        printf("Feature vector: ");
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            printf("%f ", quantized_features[i]);
        printf("\n");
    #endif


    start_count();
    CSR_MVSIZE(HV_CHUNKS * 4);
    // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)

    #if DEBUG==1
        HV appoggio;
        BundledHV appoggio_bundled;
    #endif

    for (int i = 0; i < DS_FEATURE_SIZE; i++)
    {
        hvbind((void*)((int*)spmaddrC+i*HV_CHUNKS * 4), (void*)((int*)lv_start_addr+quantized_features[i]*HV_CHUNKS * 4), (void*)((int*)bv_start_addr+i*HV_CHUNKS * 4));
        #if DEBUG==1
            printf("Binding -> ");
            printf("Level vector %d: ", quantized_features[i]);
            hvmemstr(&appoggio.chunk[0], (void*)((int*)lv_start_addr+quantized_features[i]*HV_CHUNKS * 4), sizeof(appoggio));
            appoggio.print();
            printf("\n");
            printf("with base vector %d: ", i);
            hvmemstr(&appoggio.chunk[0], (void*)((int*)bv_start_addr+i*HV_CHUNKS * 4), sizeof(appoggio));
            appoggio.print();
            printf("\n");
            printf("Result: ");
            hvmemstr(&appoggio.chunk[0], (void*)((int*)spmaddrC+i*HV_CHUNKS * 4), sizeof(appoggio));
            appoggio.print();
            printf("\n-----------------\n");
        #endif
    }
    
    // 3) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        hvbundle((void*)((int*)spmaddrD), (void*)((int*)spmaddrD), (void*)((int*)spmaddrC + i * HV_CHUNKS * 4)) ;
        #if DEBUG==1
            printf("Accumulated FeatureHV %d through bundling :", i);
            hvmemstr(&appoggio_bundled.bundled_chunk[0], (void*)((int*)spmaddrD), sizeof(appoggio_bundled));
            appoggio_bundled.print();
            printf("\n-----------------\n");
        #endif
    }

    #if DEBUG==1
        hvmemstr(&Encoded_HV.bundled_chunk[0], (void*)((int*)spmaddrD), sizeof(Encoded_HV));
        printf("Encoded HV: ");
        Encoded_HV.print();
        printf("\n");
    #endif

    // // 4) Clip the HDC vector
    HV Clipped_HV;
    hvclip((void*)((int*)spmaddrC), (void*)((int*)spmaddrD), (void*)(DS_FEATURE_SIZE));
    hvmemstr(&Clipped_HV.chunk[0], (void*)((int*)spmaddrC), sizeof(Clipped_HV));
    int accl_cycle = finish_count();
    printf("Accelerated Execution: %d cycles\n", accl_cycle);
    #if DEBUG==1
        printf("Clipped HV: ");
        Clipped_HV.print();
        printf("\n");
    #endif

    // 4) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    return Clipped_HV;
}
// --------------------End Encoding----------------------

// --------------------Temporal Encoding----------------------
HV HDC_op::temporal_encoding(int quantized_features[DS_FEATURE_SIZE][N_GRAM_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN])
{
    BundledHV Encoded_HV;
    HV Clipped_HV;

    /*for (int k = 0; k < N_GRAM_SIZE; k++)
    {
        // Initializations
        int quantized_features[DS_FEATURE_SIZE];
        HV binded_feature[DS_FEATURE_SIZE];
        HV temporal_HV[DS_FEATURE_SIZE];        

        // 1) Compute the quantization level of each feature using get_quantized_level function
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            quantized_features[i] = get_quantized_level(quantized_features[i][k], LevelList, HD_LV_LEN);
            
        // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            binded_feature[i] = this->bind(LevelVectors[quantized_features[i]], BaseVectors[i]);

        // 3) Perform the temporal encoding by permuting each binded feature by the corresponding time index
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            temporal_HV[i] = this->permutation(binded_feature[i], k);

        // 4) Bundle all the HV the HDC vector representation of the input feature vector
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            Encoded_HV = this->bundle(Encoded_HV, temporal_HV[i]);        
    }

    // 5) Clip the HDC vector
    Clipped_HV = this->clip(Encoded_HV, DS_FEATURE_SIZE*N_GRAM_SIZE);*/
    return Clipped_HV;
}


HV HDC_op::accl_temporal_encoding(int quantized_features[DS_FEATURE_SIZE][N_GRAM_SIZE], int bv_start_addr, int lv_start_addr)
{
    BundledHV Encoded_HV;
    HV Clipped_HV;

/*    for (int k = 0; k < N_GRAM_SIZE; k++)
    { 
        // 1) Compute the quantization level of each feature using get_quantized_level function
        int quantized_features[DS_FEATURE_SIZE];
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            quantized_features[i] = get_quantized_level(quantized_features[i][k], HD_LV_LEN);

        CSR_MVSIZE(HV_CHUNKS * 4);
        // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            hvbind((void*)((int*)spmaddrC+i*HV_CHUNKS * 4), (void*)((int*)lv_start_addr+quantized_features[i]*HV_CHUNKS * 4), (void*)((int*)bv_start_addr+i*HV_CHUNKS * 4));

        // 3) Perform the temporal encoding by permuting each binded feature by the corresponding time index
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            hvperm((void*)((int*)spmaddrC + i * HV_CHUNKS * 4), (void*)((int*)spmaddrC + i * HV_CHUNKS * 4), (void*)k);

        // 4) Bundle all the HV the HDC vector representation of the input feature vector
        for (int i = 0; i < DS_FEATURE_SIZE; i++)
            hvbundle((void*)((int*)spmaddrD), (void*)((int*)spmaddrD), (void*)((int*)spmaddrC + i * HV_CHUNKS * 4)) ;
    }

    // 5) Clip the HDC vector
    hvclip((void*)((int*)spmaddrC), (void*)((int*)spmaddrD), (void*)(DS_FEATURE_SIZE*N_GRAM_SIZE));
    hvmemstr(&Clipped_HV.chunk[0], (void*)((int*)spmaddrC), sizeof(Clipped_HV));*/
    return Clipped_HV;
}
// --------------------End Temporal Encoding----------------------

// --------------------Training----------------------
BundledHV HDC_op::training(int quantized_features[DS_FEATURE_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN], BundledHV ClassVectors[HD_CV_LEN], int class_label)
{
    // We first encode the input feature vector into an HDC vector
    //HV encoded_hv = this->encoding(quantized_features, BaseVectors, LevelVectors);

    BundledHV Encoded_HV;
    
    start_count();
    // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
    HV binded_feature[DS_FEATURE_SIZE];
    for (int i = 0; i < DS_FEATURE_SIZE; i++)
    {
        binded_feature[i] = this->bind(LevelVectors[quantized_features[i]], BaseVectors[i]);
    }
    
    // 3) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        Encoded_HV = this->bundle(Encoded_HV, binded_feature[i]);
    }

    // 4) Clip the HDC vector
    HV Clipped_HV;
    Clipped_HV = this->clip(Encoded_HV,DS_FEATURE_SIZE);

    // We can now bundle the HDC vector with the corresponding class vector
    ClassVectors[class_label] = this->bundle(ClassVectors[class_label], Clipped_HV);
    int std_cycle = finish_count();
    printf("Standard Execution: %d cycles\n", std_cycle);

    return ClassVectors[class_label];
}

BundledHV HDC_op::accl_training(int quantized_features[DS_FEATURE_SIZE], int bv_start_addr, int lv_start_addr, BundledHV ClassVectors[HD_CV_LEN], int class_label)
{
    
    // We first encode the input feature vector into an HDC vector
    // this->accl_encoding(quantized_features, bv_start_addr, lv_start_addr);   // Note: at the end of encoding, the HDC vector is stored in _spmC if the last flag is set to 0

    BundledHV Encoded_HV;
    start_count();
    CSR_MVSIZE(HV_CHUNKS * 4);

    // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        hvbind((void*)((int*)spmaddrC+i*HV_CHUNKS * 4), (void*)((int*)lv_start_addr+quantized_features[i]*HV_CHUNKS * 4), (void*)((int*)bv_start_addr+i*HV_CHUNKS * 4));
    }
    
    // 3) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        hvbundle((void*)((int*)spmaddrD), (void*)((int*)spmaddrD), (void*)((int*)spmaddrC + i * HV_CHUNKS * 4)) ;
    }

    // 4) Clip the HDC vector
    HV Clipped_HV;
    hvclip((void*)((int*)spmaddrC), (void*)((int*)spmaddrD), (void*)(DS_FEATURE_SIZE));
    //hvmemstr(&Clipped_HV.chunk[0], (void*)((int*)spmaddrC), sizeof(Clipped_HV));

    int offset = class_label*HV_CHUNKS * 4;
    // We can now bundle the HDC vector with the corresponding class vector using the hvbundle instruction
    hvbundle((void*)((int*)spmaddrD+offset), (void*)((int*)spmaddrD+offset), (void*)((int*)spmaddrC)) ;
    hvmemstr(&ClassVectors[class_label].bundled_chunk[0], (void*)((int*)spmaddrD+offset), sizeof(ClassVectors[class_label]));
    int accl_cycle = finish_count();
    printf("Accelerated Execution: %d cycles\n", accl_cycle);

    return ClassVectors[class_label];
}
// --------------------End Training----------------------

// --------------------Inference----------------------
int HDC_op::inference(int quantized_features[DS_FEATURE_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN], HV ClassVectors[HD_CV_LEN])
{
    //HV encoded_hv = this->encoding(quantized_features, BaseVectors, LevelVectors);
    // Encoding
    

    BundledHV Encoded_HV;
    start_count();    
    // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
    HV binded_feature[DS_FEATURE_SIZE];
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        binded_feature[i] = this->bind(LevelVectors[quantized_features[i]], BaseVectors[i]);
    }
    
    // 3) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        Encoded_HV = this->bundle(Encoded_HV, binded_feature[i]);
    }

    // 4) Clip the HDC vector
    HV Clipped_HV;
    Clipped_HV = this->clip(Encoded_HV,DS_FEATURE_SIZE);

    int minimum_distance = HV_SIZE_BIT;
    int predicted_class = -1;
    for (int i = 0; i < HD_CV_LEN; i++){
        int hamming_distance = this->similarity(Clipped_HV, ClassVectors[i]);
        if (hamming_distance < minimum_distance)
        {
            minimum_distance = hamming_distance;
            predicted_class = i;
        }
    }

    int std_cycle = finish_count();
    printf("Standard Execution: %d cycles\n", std_cycle);


    return predicted_class;
}

int HDC_op::accl_inference(int quantized_features[DS_FEATURE_SIZE], int bv_start_addr, int lv_start_addr, HV ClassVectors[HD_CV_LEN])
{
    int minimum_distance = HV_SIZE_BIT;
    int predicted_class = -1;
    int hamming_distance;
    
    // We first encode the input feature vector into an HDC vector
    //this->accl_encoding(quantized_features, bv_start_addr, lv_start_addr);   // Note: at the end of encoding, the HDC vector is stored in _spmC if the last flag is set to 0

    BundledHV Encoded_HV;

    start_count();
    CSR_MVSIZE(HV_CHUNKS * 4);
    // 2) BIND the level vector with the corresponding base vector (dependent by the number of the feature)
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        hvbind((void*)((int*)spmaddrC+i*HV_CHUNKS * 4), (void*)((int*)lv_start_addr+quantized_features[i]*HV_CHUNKS * 4), (void*)((int*)bv_start_addr+i*HV_CHUNKS * 4)); 
    }
    
    // 3) Bundle all the level vectors together to obtain the HDC vector representation of the input feature vector
    for (int i = 0; i < DS_FEATURE_SIZE; i++){
        hvbundle((void*)((int*)spmaddrD), (void*)((int*)spmaddrD), (void*)((int*)spmaddrC + i * HV_CHUNKS * 4)) ;  
    }

    // 4) Clip the HDC vector
    HV Clipped_HV;
    hvclip((void*)((int*)spmaddrC), (void*)((int*)spmaddrD), (void*)(DS_FEATURE_SIZE));
    // hvmemstr(&Clipped_HV.chunk[0], (void*)((int*)spmaddrC), sizeof(Clipped_HV));

    int class_offset = HV_CHUNKS*4*2;
    
    for (int i = 0; i < HD_CV_LEN; i++){
        hvsim((void*)((int*)spmaddrC+HV_CHUNKS * 4), (void*)((int*)spmaddrD + class_offset + i*HV_CHUNKS * 4), (void*)((int*)spmaddrC)) ;
        hvmemstr(&hamming_distance, (void*)((int*)spmaddrC+HV_CHUNKS * 4), sizeof(int));
        if (hamming_distance < minimum_distance)
        {
            minimum_distance = hamming_distance;
            predicted_class = i;
        }
    }

    int accl_cycle = finish_count();
    printf("Accelerated Execution: %d cycles\n", accl_cycle);

    return predicted_class;
}
// --------------------End Inference----------------------

// --------------------End HDC Class----------------------

