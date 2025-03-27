// primes_obj.h
#ifndef PRIMES_OBJ_H
#define PRIMES_OBJ_H

#include <utils.h>

/**
 * @brief Structure representing a collection of uint64_t prime numbers.
 */
typedef struct
{
    int capacity;                               ///< Current capacity of the primes array.
    int p_count;                                ///< Number of primes currently stored.
    uint64_t *p_array;                          ///< Pointer to the dynamically allocated primes uint64_t array.
    unsigned char sha256[SHA256_DIGEST_LENGTH]; ///< SHA-256 hash of p_array for validation.
} PRIMES_OBJ;

/**
 * @brief Initializes a new PRIMES_OBJ structure with an initial estimate of capacity.
 *
 * @param initial_estimate The initial estimate for the capacity of the primes array.
 * @return A pointer to the newly created PRIMES_OBJ structure.
 */
PRIMES_OBJ *primes_obj_init(int initial_estimate);

/**
 * @brief Appends a prime number p to the primes_obj->primes array,
 * and increments the models_count of primes primes_obj->p_count.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @param p The prime number to be appended.
 */
void primes_obj_append(PRIMES_OBJ *primes_obj, uint64_t p);

/**
 * @brief Resizes the primes array to match the current number of primes stored.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @return 0 on success, non-zero on failure.
 */
int primes_obj_resize_to_p_count(PRIMES_OBJ *primes_obj);

/**
 * @brief Frees the memory allocated for the PRIMES_OBJ structure.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure to be freed.
 */
void primes_obj_free(PRIMES_OBJ *primes_obj);

/**
 * @brief Computes the SHA-256 hash of the primes array.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @return 0 on success, non-zero on failure.
 */
int primes_obj_compute_hash(PRIMES_OBJ *primes_obj);

/**
 * @brief Validates the SHA-256 hash of the primes array.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @return 0 if the hash is valid, non-zero if invalid.
 */
int primes_obj_validate_hash(PRIMES_OBJ *primes_obj);

/**
 * @brief Writes the PRIMES_OBJ structure to a file.
 *
 * @param file_path The path to the file where the structure will be written.
 * @param primes_obj A pointer to the PRIMES_OBJ structure to be written.
 * @return 0 on success, non-zero on failure.
 */
int primes_obj_write_file(const char *file_path, PRIMES_OBJ *primes_obj);

/**
 * @brief Reads a PRIMES_OBJ structure from a file.
 *
 * @param file_path The path to the file from which the structure will be read.
 * @return A pointer to the newly read PRIMES_OBJ structure, or NULL on failure.
 */
PRIMES_OBJ *primes_obj_read_file(const char *file_path);

#endif // PRIMES_OBJ_H
