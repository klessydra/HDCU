#include "hv_struct.hpp"

// Default constructor: Initializes all data elements to zero
HV::HV() {
    for (int i = 0; i < HV_CHUNKS; ++i) {
        chunk[i] = 0;
    }
}

// Copy constructor
HV::HV(const HV& other) {
    for (int i = 0; i < HV_CHUNKS; ++i) {
        chunk[i] = other.chunk[i];
    }
}

// Define the assignment operator
HV& HV::operator=(const HV& other) {
    if (this != &other) {
        for (int i = 0; i < HV_CHUNKS; ++i) {
            chunk[i] = other.chunk[i];
        }
    }
    return *this;
}

// Define the randomize function
void HV::randomize() {
    for (int i = 0; i < HV_CHUNKS; ++i) {
        int random_number = rand();
        chunk[i] = random_number;
    }
}

// Print Operator, bit by bit
void HV::print() {
    printf("[");
    for (int i = 0; i < HV_CHUNKS; ++i) {
        for (int j = 31; j >= 0; --j) {
            printf("%d", (chunk[i] >> j) & 1);
        }
    }
    printf("]\n");
}

// Default constructor: Initializes all data elements to zero
BundledHV::BundledHV() {
    for (int i = 0; i < HV_CHUNKS * COUNTER_BITS; ++i) {
        bundled_chunk[i] = 0;
    }
}

// Copy constructor
BundledHV::BundledHV(const BundledHV& other) {
    for (int i = 0; i < HV_CHUNKS * COUNTER_BITS; ++i) {
        bundled_chunk[i] = other.bundled_chunk[i];
    }
}

// Define the assignment operator
BundledHV& BundledHV::operator=(const BundledHV& other) {
    if (this != &other) {
        for (int i = 0; i < HV_CHUNKS * COUNTER_BITS; ++i) {
            bundled_chunk[i] = other.bundled_chunk[i];
        }
    }
    return *this;
}

// Define the assignment operator
BundledHV& BundledHV::operator=(const HV& other) {
    int bundled_index = 0;
    int bit_position = 28;

    for (int i = 0; i < HV_CHUNKS; ++i) {  // Iterate over each 32-bit chunk in HV
        for (int bit = 31; bit >= 0; --bit) {  // Iterate over each bit in the 32-bit chunk
            if ((other.chunk[i] >> bit) & 1) {
                bundled_chunk[bundled_index] |= (0x1 << bit_position);
            } else {
                bundled_chunk[bundled_index] &= ~(0xF << bit_position);
            }
            bit_position -= 4;
            if (bit_position < 0) {
                bit_position = 28;
                bundled_index++;
            }
        }
    }

    return *this;
}

// Print Operator, bit by bit
void BundledHV::print() {
    printf("[");
    for (int i = 0; i < HV_CHUNKS * COUNTER_BITS; ++i) {
        for (int j = 28; j >= 0; j -= 4) {
            printf("%d", (bundled_chunk[i] >> j) & 0xF);
        }
    }
    printf("]\n");
}
