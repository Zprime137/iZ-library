// sieve.h
#ifndef SIEVE_H
#define SIEVE_H

#include <utils.h>
#include <primes_obj.h>

// Sieve function type, takes an upper limit and returns a PRIMES_OBJ pointer
typedef PRIMES_OBJ *(*sieve_fn)(uint64_t);

// Structure to associate the sieve function with its name
typedef struct
{
    sieve_fn function;
    const char *name;
} SieveAlgorithm;

// Structure to hold a list of sieve algorithms
typedef struct
{
    SieveAlgorithm *models_list;
    int models_count;
} SieveModels;

// Sieve Algorithms:
//
// Traditional Sieve of Eratosthenes algorithm
extern SieveAlgorithm ClassicSieveOfEratosthenes;
PRIMES_OBJ *classic_sieve_eratosthenes(uint64_t n);

// Optimized Sieve of Eratosthenes algorithm
extern SieveAlgorithm SieveOfEratosthenes;
PRIMES_OBJ *sieve_eratosthenes(uint64_t n);

// Segmented Sieve of Eratosthenes algorithm
extern SieveAlgorithm SegmentedSieve;
PRIMES_OBJ *segmented_sieve(uint64_t n);

// Sieve of Euler algorithm
extern SieveAlgorithm SieveOfEuler;
PRIMES_OBJ *sieve_euler(uint64_t n);

// Sieve of Atkin algorithm
extern SieveAlgorithm SieveOfAtkin;
PRIMES_OBJ *sieve_atkin(uint64_t n);

// Wheel Sieve algorithm
extern SieveAlgorithm WheelSieve;
PRIMES_OBJ *sieve_wheel(uint64_t n);

// Classic Sieve-iZ algorithm
extern SieveAlgorithm Sieve_iZ;
PRIMES_OBJ *sieve_iZ(uint64_t n);

// Segmented Sieve-iZm algorithm
extern SieveAlgorithm Sieve_iZm;
PRIMES_OBJ *sieve_iZm(uint64_t n);

// Test Sieve Algorithms for Integrity
int test_sieve_integrity(SieveModels sieve_models, uint64_t n);

// Measure the execution time to compute the primes up to n using given a sieve model
size_t measure_sieve_time(SieveAlgorithm sieve_model, uint64_t n);

// Benchmark the sieve algorithms for a given range of exponents
void benchmark_sieve(SieveModels sieve_models, int base, int min_exp, int max_exp, int save_results);

/**
 * @brief Measures the number of mark-composite operations of sieve-iZ algorithms against sieve-Eratosthenes.
 *
 * This function runs the sieve algorithms (sieve-Eratosthenes, sieve-iZ, sieve-iZm) for a range of incrementing exponents of 10
 * and counts the number of mark-composite operations required to mark all composite numbers.
 * The results are printed to the stdout, and optionally saved to a file.
 */
void measure_sieve_w_op(int save_results);

#endif // SIEVE_H
