// utils.h
#ifndef UTILS_H
#define UTILS_H

// necessary includes across files

#include <stdio.h>    // For printf, FILE, fopen, fwrite, fread, etc.
#include <stdlib.h>   // For malloc, free, etc.
#include <stdint.h>   // For fixed-width integer types like uint64_t
#include <stddef.h>   // For size_t
#include <string.h>   // For string manipulation functions like snprintf
#include <unistd.h>   // For checking file existence and permissions
#include <assert.h>   // For assertions
#include <math.h>     // For math functions like sqrt
#include <time.h>     // For time functions
#include <fcntl.h>    // For file operations
#include <sys/stat.h> // For creating directories (mkdir)
#include <pthread.h>  // For multithreading

// GMP library for arbitrary precision arithmetic
#include <gmp.h>

// OpenSSL includes for cryptographic functions
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <openssl/rand.h>

// Custom modules
#include <logger.h>     // For logging errors
#include <bitmap.h>     // For bitmap module
#include <primes_obj.h> // For PRIMES_OBJ structure

// Global Directories
#define DIR_output "output"
#define DIR_iZm DIR_output "/iZm"

// VX6 constant value
#define vx6 5 * 7 * 11 * 13 * 17 * 19 // 1616615

// minimum and maximum macros
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

int create_dir(const char *dir);

uint64_t pi_n(int64_t n);
uint64_t int_pow(uint64_t base, int exp);
int is_numeric_str(const char *str);

// utility functions
void print_line(int length);

/**
 * @brief Compute the SHA-256 hash for an array of integers.
 *
 * @param array Pointer to the array of integers.
 * @param size Size of the array.
 * @param hash Pointer to the SHA-256 hash array.
 * @return int 0 on success, -1 on failure.
 */
int hash_int_array(int *array, size_t size, unsigned char *hash);

/**
 * @brief Validate the SHA-256 hash.
 *
 * @param hash1 Pointer to the first SHA-256 hash array.
 * @param hash2 Pointer to the second SHA-256 hash array.
 * @return int 1 on success, 0 on failure.
 */
int validate_sha256_hash(const unsigned char *hash1, const unsigned char *hash2);

/**
 * @brief Print the SHA-256 hash.
 *
 * @param hash Pointer to the SHA-256 hash array.
 */
void print_sha256_hash(const unsigned char *hash);

/**
 * @brief Seed the GMP random state.
 *
 * @param state The GMP random state object to seed.
 */
void gmp_seed_randstate(gmp_randstate_t state);

#endif // UTILS_H
