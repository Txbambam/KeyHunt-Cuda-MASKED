

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h> // For pow()

#include "KeyHunt.h" // Include the new header

#define MAX_TARGETS 1024
#define BITS_PER_NIBBLE 4

// --- Helper function for hex conversion ---
int hexCharToNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // Invalid hex character
}

// --- Global variables for mask mode ---
char mask_str[KEY_SIZE_HEX + 1] = {0}; 
bool mask_mode = false;
MaskContext mask_context;
uint64_t total_iterations = 0;

// Function to parse the mask and prepare the MaskContext
int parseMask(const char* mask) {
    mask_context.unknown_count = 0;
    
    // Initialize the fixed key to zero
    for (int i = 0; i < KEY_WORDS; ++i) {
        mask_context.fixed_key[i] = 0;
    }

    if (strlen(mask) != KEY_SIZE_HEX) {
        fprintf(stderr, "Error: Mask must be exactly %d characters long.\n", KEY_SIZE_HEX);
        return 0;
    }

    // Iterate through the 64 nibbles (0 to 63, from MSB to LSB)
    for (int i = 0; i < KEY_SIZE_HEX; ++i) {
        char c = mask[i];
        
        // Nibble position in the 256-bit key (0 is MSB, 63 is LSB)
        int nibble_pos = KEY_SIZE_HEX - 1 - i;
        
        // Calculate the word index (0-7) and the shift (0, 4, 8, ... 28)
        int word_index = nibble_pos / 8; // Key is stored LSB first, so index 0 is least significant bits
        int shift = (nibble_pos % 8) * BITS_PER_NIBBLE;

        if (c == '?') {
            if (mask_context.unknown_count >= MAX_UNKNOWN_CHARS) {
                // Should not happen if KEY_SIZE_HEX == MAX_UNKNOWN_CHARS
                fprintf(stderr, "Error: Too many unknown characters in mask.\n");
                return 0;
            }
            // Store the position (0-63) of the '?'
            mask_context.variable_nibble_positions[mask_context.unknown_count++] = nibble_pos;
        } else {
            int nibble_value = hexCharToNibble(c);
            if (nibble_value == -1) {
                fprintf(stderr, "Error: Invalid character '%c' in mask at position %d. Use 0-9, a-f, or ?\n", c, i);
                return 0;
            }
            
            // Set the known nibble value in the fixed key
            mask_context.fixed_key[word_index] |= (uint32_t)nibble_value << shift;
        }
    }
    
    // Calculate total iterations: 16 ^ unknown_count
    if (mask_context.unknown_count > 16) {
        fprintf(stderr, "Warning: Mask mode with more than 16 unknown characters (2^64 keys) is not fully supported by current 64-bit CUDA indexing. Capping iterations at 2^64.\n");
        total_iterations = UINT64_MAX; // Max out 64-bit counter
    } else {
        total_iterations = (uint64_t)pow(16.0, mask_context.unknown_count);
    }

    printf("Mask Mode enabled. Unknown characters: %d (%.0f Keys)\n", 
           mask_context.unknown_count, (double)total_iterations);

    return 1;
}

// Main function (Simplified for brevity, focusing on the mask integration)
int main(int argc, char** argv) {
    // --- ARGUMENT PARSING START ---
    
    // Example placeholder for target key loading
    // target_keys[MAX_TARGETS]... 
    int num_targets = 1; // Assume 1 for simplicity

    // Default to a wide search range if no mask is provided (old behavior)
    KernelParams k_params;
    k_params.start_index = 0;
    k_params.end_index = 0; 

    for (int i = 1; i < argc; ++i) {
        // ... (Handling of existing arguments like -b, -t, -s, -e) ...

        // --- NEW MASK ARGUMENT HANDLING ---
        if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mask") == 0) {
            if (++i < argc) {
                strncpy(mask_str, argv[i], KEY_SIZE_HEX);
                mask_str[KEY_SIZE_HEX] = '\0'; // Ensure null termination
                mask_mode = true;
            } else {
                fprintf(stderr, "Error: -m/--mask requires a 64-character hex mask.\n");
                return 1;
            }
        }
    }

    if (mask_mode) {
        if (!parseMask(mask_str)) {
            return 1; // Error during mask parsing/validation
        }
        // In mask mode, total_iterations is calculated by parseMask
    } else {
        // Traditional range mode: calculate total_iterations based on start/end keys
        // total_iterations = k_params.end_index - k_params.start_index;
        // Assume default iteration if nothing is provided
        total_iterations = 1000000; 
        printf("Range Mode enabled. Searching 0x%llX iterations.\n", (unsigned long long)total_iterations);
    }
    
    // --- ARGUMENT PARSING END ---
    
    if (total_iterations == 0) {
        printf("No keys to check. Exiting.\n");
        return 0;
    }

    // --- CUDA SETUP & LAUNCH ---
    
    // Placeholder for device pointer allocation and data copying (d_targets)
    void* d_targets = NULL; // Must be allocated and copied to device in real code
    
    // If mask mode, we pass the MaskContext to the GPU
    MaskContext* d_mask = NULL;
    if (mask_mode) {
        // Allocate space on the device for the MaskContext and copy it
        // (This would use cudaMalloc and cudaMemcpy)
        // d_mask = (MaskContext*)cuda_allocate_and_copy(&mask_context, sizeof(MaskContext));
    }

    // Launch the kernel
    printf("Launching KeyHunt kernel...\n");
    KeyHunt_Launcher(
        d_targets, 
        num_targets,
        total_iterations, 
        0, // device_id
        &k_params, 
        d_mask,
        mask_mode
    );

    // --- CUDA CLEANUP ---
    
    // Free device and host memory (d_targets, d_mask, etc.)
    
    printf("Execution finished.\n");
    return 0;
}


	v->Search(nbCPUThread, gpuId, gridSize, should_exit);
	delete v;
	return 0;
#endif
}
