#ifndef HV_STRUCT_HPP
#define HV_STRUCT_HPP

#include <cstdio>
#include <cstdlib>
#include "hdc_defines.hpp"

struct HV {
    int chunk[HV_CHUNKS];

    // Default constructor: Initializes all data elements to zero
    HV();

    // Copy constructor
    HV(const HV& other);

    // Define the assignment operator
    HV& operator=(const HV& other);

    // Define the randomize function
    void randomize();

    // Print Operator, bit by bit
    void print();
};

struct BundledHV {
    int bundled_chunk[HV_CHUNKS * COUNTER_BITS]; // 4 bits per element

    // Default constructor: Initializes all data elements to zero
    BundledHV();

    // Copy constructor
    BundledHV(const BundledHV& other);

    // Define the assignment operator
    BundledHV& operator=(const BundledHV& other);

    // Define the assignment operator
    BundledHV& operator=(const HV& other);

    // Print Operator, bit by bit
    void print();
};

#endif // HV_STRUCT_HPP

