#ifndef HDC_OP_HPP
#define HDC_OP_HPP

#include "hv_struct.hpp"
#include "hdc_defines.hpp"
extern "C" {            // Klessydra dsp_libraries are written in C and so they're imported as extern:
    #include "dsp_functions.h"
    #include "functions.h"
}

///--------------------------- Auxiliary Functions: --------------------------- 
void generate_quantization_levels(float min, float max, int levels, float LevelList[HD_LV_LEN]);
int get_quantized_level(float value, float quantization_levels[HD_LV_LEN], int levels);

// --------------------------- HDC Class: ------------------------------------- 
class HDC_op {
public:
    int HV_SIZE;           // HV size
    int HV_type;           // HV type: binary or bipolar
    int num_levels;        // Number of levels used in the model
    int num_features;      // Number of features used in the model
    int lv_technique;      // Level vector technique: 0: linear, 1: approximately linear, 2: thermometer encoding
    int density;           // Density of the HV (dense or sparse)
    float sparsity_factor; // Sparsity factor of the HV (used only for sparse HVs)
    int HV_similarity;     // HV similarity: 0: Hamming distance, 1: Cosine similarity
    int quant_min;
    int quant_max;
    int base_value;        // Base value of the HV (used only for bipolar HVs)

    // Constructor
    HDC_op(int dimensionality, int features, int levels);

    // Base HVs
    void generate_BaseHVs(HV baseVectors[DS_FEATURE_SIZE]);

    // Similarity
    int similarity(HV HV1, HV HV2);

    // Associative Search
    int Search(HV query, HV ClassVectors[HD_CV_LEN]);

    // Binding
    HV bind(HV HV1, HV HV2);

    // Permutation
    HV permutation(HV hv, int shift);

    // Bundling
    BundledHV bundle(const BundledHV& HV1, const HV& HV2) const;

    // Clipping
    HV clip(const BundledHV& bundled_hv, int HV_BUNDLED);

    // Level Vector Generation
    void generate_LevelVectors(HV LevelVectors[HD_LV_LEN]);

    // Encoding
    HV encoding(int FeatureVector[DS_FEATURE_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN]);

    // Accl Encoding
    HV accl_encoding(int quantized_features[DS_FEATURE_SIZE], int bv_start_addr, int lv_start_addr);

    // Temporal Encoding
    HV temporal_encoding(int quantized_features[DS_FEATURE_SIZE][N_GRAM_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN]);
    
    // Accl Temporal Encoding
    HV accl_temporal_encoding(int quantized_features[DS_FEATURE_SIZE][N_GRAM_SIZE], int bv_start_addr, int lv_start_addr);
    
    // Training
    BundledHV training(int quantized_features[DS_FEATURE_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN],  BundledHV ClassVectors[HD_CV_LEN], int class_label);

    // Accl Training
    BundledHV accl_training(int quantized_features[DS_FEATURE_SIZE],  int bv_start_addr, int lv_start_addr,  BundledHV ClassVectors[HD_CV_LEN], int class_label);

    // Inference
    int inference(int quantized_features[DS_FEATURE_SIZE], HV BaseVectors[DS_FEATURE_SIZE], HV LevelVectors[HD_LV_LEN],  HV ClassVectors[HD_CV_LEN]);

    // Accl Inference
    int accl_inference(int quantized_features[DS_FEATURE_SIZE],  int bv_start_addr, int lv_start_addr,  HV ClassVectors[HD_CV_LEN]);

};

#endif // HDC_OP_HPP


