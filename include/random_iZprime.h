// iZm.h
#ifndef IZprime_H
#define IZprime_H

#include <utils.h> // For GMP library

// Random iZprime algorithm signature
void random_iZprime(
    mpz_t p,                    // Output: Random prime candidate
    int p_id,                   // Matrix ID -1 or 1
    int bit_size,               // Desired bit size of the prime
    int primality_check_rounds, // Number of Miller-Rabin testing rounds
    int cores_num               // Number of cores to use
);

// Random prime generation using GMP's mpz_nextprime
void gmp_random_nextprime(mpz_t p, int bit_size);

// Benchmarking random prime generation algorithms
void benchmark_random_prime_algorithms(int bit_size, int primality_check_rounds, int test_rounds, int save_results);

#endif // IZprime_H
