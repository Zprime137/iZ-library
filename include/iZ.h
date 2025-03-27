// iZm.h
#ifndef IZ_H
#define IZ_H

#include <utils.h>
#include <bitmap.h>
#include <primes_obj.h>

// iZ functions
uint64_t iZ(uint64_t x, int i);
void iZ_gmp(mpz_t z_t, mpz_t x, int i);

// VX prime search space Analysis
void analyze_vx_potential_primes(void);

// Sieve Tools:
// Compute limited vx for a given range x_n
size_t compute_limited_vx(size_t x_n, int limit);

// Compute closest vx to the given bit-size
void gmp_compute_max_vx(mpz_t vx, int bit_size);

// Construct vx2 bitmaps for marking composites of 5, 7 once
void construct_vx2(BITMAP *x5, BITMAP *x7);

/**
 * @brief Generate the first iZm segment of size vx
 * @param vx the size of the segment
 * @param x5 bitmap for iZ-
 * @param x7 bitmap for iZ+
 */
void construct_iZm_segment(size_t vx, BITMAP *x5, BITMAP *x7);

uint64_t normalized_xp(int matrix_id, uint64_t p);

void normalized_xp_gmp(mpz_t x_p, int matrix_id, uint64_t p);

uint64_t solve_for_x(int matrix_id, uint64_t p, size_t vx, uint64_t y);

uint64_t solve_for_x_gmp(int matrix_id, uint64_t p, size_t vx, mpz_t y);

int modular_inverse(int a, int m);

void modular_inverse_gmp(mpz_t mod_inv, mpz_t a, mpz_t m);

uint64_t solve_for_y(int matrix_id, uint64_t p, size_t vx, uint64_t x);

#endif // IZ_H
