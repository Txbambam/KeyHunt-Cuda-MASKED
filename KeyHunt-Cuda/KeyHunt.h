#ifndef KEYHUNT_H
#define KEYHUNT_H

#include <stdint.h>
#include <stdbool.h>

// Assuming the original implementation uses 256-bit keys represented by 8 uint32_t words.
#define KEY_WORDS 8
#define KEY_SIZE_BITS 256
#define KEY_SIZE_HEX 64
#define MAX_UNKNOWN_CHARS KEY_SIZE_HEX // 64 maximum '?' characters

// --- NEW STRUCTURE FOR MASK MODE ---
typedef struct {
    // Stores the known 256-bit key value (with 0s for all unknown '?' parts)
    uint32_t fixed_key[KEY_WORDS]; 
    
    // Total number of unknown hexadecimal characters (1 to 64)
    int unknown_count; 
    
    // Array to store the index of the 4-bit nibble within the 256-bit key 
    // that corresponds to each '?' character.
    // Index 0 is the most significant nibble (leftmost hex digit).
    // The position stored here is 0-63.
    int variable_nibble_positions[MAX_UNKNOWN_CHARS];
} MaskContext;

// Structure to pass execution parameters to the CUDA kernel
typedef struct {
    uint64_t start_index;
    uint64_t end_index;
    // ... potentially other non-mask parameters like targets per block, etc.
} KernelParams;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Launches the KeyHunt CUDA kernel.
 * * @param d_targets Device pointer to the public key targets.
 * @param num_targets Number of targets.
 * @param total_iterations Total number of keys to check.
 * @param device_id GPU device ID.
 * @param k_params Traditional kernel parameters (used when mask_mode is false).
 * @param d_mask Device pointer to the MaskContext (used when mask_mode is true).
 * @param mask_mode Flag to indicate if mask mode is active.
 */
void KeyHunt_Launcher(
    const void* d_targets, 
    int num_targets,
    uint64_t total_iterations, 
    int device_id,
    const KernelParams* k_params, 
    const MaskContext* d_mask,
    bool mask_mode
);

#ifdef __cplusplus
}
#endif

#endif // KEYHUNT_H

