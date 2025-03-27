// sieve.c
#include <bitmap.h>
#include <primes_obj.h>
#include <iZ.h>

#include <sieve.h>

// Classic Sieve-iZ
SieveAlgorithm Sieve_iZ = {sieve_iZ, "Sieve-iZ"};

/**
 * @brief Generates a list of prime numbers up to a given limit using the Xp Wheel to
 * mark composites in the iZ set, the set of numbers of the form (6x +/- 1).
 *
 * @param n The upper limit for generating prime numbers.
 * @return A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *         or NULL if memory allocation fails.
 */
PRIMES_OBJ *sieve_iZ(uint64_t n)
{
    // Initialize primes object with enough initial estimation
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    // Memory allocation failed, check logs
    if (primes == NULL)
        return NULL;

    // Add 2, 3 to primes, the only non iZ primes
    primes_obj_append(primes, 2);
    primes_obj_append(primes, 3);

    // Calculate x_n, max x value < n
    uint64_t x_n = (n + 1) / 6 + 1;

    // Create bitmap X-Arrays x5, x7, each of size x_n + 1 bits, total n/3 bits
    BITMAP *x5 = bitmap_create(x_n + 1);
    BITMAP *x7 = bitmap_create(x_n + 1);

    // Set all bits initially as candidates for primes
    bitmap_set_all(x5);
    bitmap_set_all(x7);

    // Calculate n_sqrt: the upper bound for root primes
    uint64_t n_sqrt = sqrt(n) + 1;

    // Iterate through x values in range 0 < x < x_n
    // This main loop makes n/3 `is prime` operations
    for (uint64_t x = 1; x < x_n; x++)
    {
        // if x5[x], implying it's iZ- prime
        if (bitmap_get_bit(x5, x))
        {
            uint64_t z = iZ(x, -1);
            primes_obj_append(primes, z); // add z to primes, increment p_count

            // if z is root prime, mark its multiples in x5, x7
            if (z < n_sqrt)
            {
                bitmap_clear_mod_p(x5, z, z * x + x, x_n);
                bitmap_clear_mod_p(x7, z, z * x - x, x_n);
            }
        }

        // Do the same if x7[x], inverting the signs
        if (bitmap_get_bit(x7, x))
        {
            uint64_t z = iZ(x, 1);
            primes_obj_append(primes, z);

            if (z < n_sqrt)
            {
                bitmap_clear_mod_p(x5, z, z * x - x, x_n);
                bitmap_clear_mod_p(x7, z, z * x + x, x_n);
            }
        }
    }

    // Cleanup: free memory of x5, x7
    bitmap_free(x5);
    bitmap_free(x7);

    // Handle edge case: if last prime > n, remove it
    if (primes->p_array[primes->p_count - 1] > n)
        primes->p_count--;

    // Trim unused memory in primes object
    if (primes_obj_resize_to_p_count(primes) != 0)
    {
        // Error already logged in primes_obj_resize
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}

// Segmented Sieve-iZm Algorithm
SieveAlgorithm Sieve_iZm = {sieve_iZm, "Sieve-iZm"};

/**
 * @brief Segmented prime sieve using the iZm structure.
 *
 * @param n The upper limit for generating prime numbers.
 * @return A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *         or NULL if memory allocation fails.
 */
PRIMES_OBJ *sieve_iZm(uint64_t n)
{
    // 1. Initialization
    size_t x_n = (n + 1) / 6 + 1;

    // Initialize primes array with enough capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    // Memory allocation failed, check logs
    if (primes == NULL)
        return NULL;

    // add 2, 3 to the primes array
    primes_obj_append(primes, 2);
    primes_obj_append(primes, 3);

    // list of small primes to be pre-sieved
    uint64_t s_primes[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};

    // Calculate optimal segment size vx for x_n
    int vx_limit = 6; // max number of primes to be pre-sieved
    size_t vx = compute_limited_vx(x_n, vx_limit);

    int start_i = 2; // skip 2, 3

    // Add pre-sieved primes to primes array
    for (int i = 0; i < vx_limit; i++)
    {
        if (vx % s_primes[i] == 0)
        {
            primes_obj_append(primes, s_primes[i]);
            start_i++;
        }
        else
            break;
    }

    // Initialize x5, x7 bitmaps and their clones tmp5, tmp7
    BITMAP *x5, *x7, *tmp5, *tmp7;

    // Allocate memory for x5 and x7 with vx + 10 bits
    x5 = bitmap_create(vx + 10);
    x7 = bitmap_create(vx + 10);

    // 2. Preprocessing:
    // Generate pre-sieved segments of size vx in x5, x7
    construct_iZm_segment(vx, x5, x7);

    // Clone x5, x7 into tmp5, tmp7 for processing
    tmp5 = bitmap_clone(x5);
    tmp7 = bitmap_clone(x7);

    // 3. Process 1st segment to collect enough root primes
    for (uint64_t x = 2; x <= vx; x++)
    {
        if (bitmap_get_bit(tmp5, x)) // If iZ-[x] is prime
        {
            size_t p = iZ(x, -1);
            primes_obj_append(primes, p);

            // Mark composites of z within this segment if any
            if ((p * p) / 6 < vx)
            {
                bitmap_clear_mod_p(tmp5, p, p * x + x, vx);
                bitmap_clear_mod_p(tmp7, p, p * x - x, vx);
            }
        }

        if (bitmap_get_bit(tmp7, x)) // If iZ+[x] is prime
        {
            size_t p = iZ(x, 1);
            primes_obj_append(primes, p);

            if ((p * p) / 6 < vx)
            {
                bitmap_clear_mod_p(tmp5, p, p * x - x, vx);
                bitmap_clear_mod_p(tmp7, p, p * x + x, vx);
            }
        }
    }

    // 4. Processing remaining segments:
    int max_y = x_n / vx; // number of segments
    uint64_t limit = vx;  // upper bound for marking composites

    // Sieve and collect primes from the remaining segments
    for (int y = 1; y <= max_y; y++)
    {
        // Reset tmp data for each run
        tmp5 = bitmap_clone(x5);
        tmp7 = bitmap_clone(x7);

        // Either vx or x_n % vx in the last segment
        if (y == max_y)
            limit = x_n % vx;

        // Mark composites of the rest of root primes in current segment
        for (int i = start_i; i < primes->p_count; i++)
        {
            uint64_t p = primes->p_array[i];

            // Exit if p doesn't have composites in this range
            if ((p * p) / 6 > (y * vx + limit))
                break;

            // Locate the 1st composite of p in both iZm5 and iZm7
            uint64_t xp5 = solve_for_x(-1, p, vx, y);
            uint64_t xp7 = solve_for_x(1, p, vx, y);

            // Mark composites of p in the current segment
            bitmap_clear_mod_p(tmp5, p, xp5, limit);
            bitmap_clear_mod_p(tmp7, p, xp7, limit);
        }

        // Collect unmarked x values as primes, or output to disk
        uint64_t yvx = y * vx;
        for (uint64_t x = 1; x <= limit; x++)
        {
            if (bitmap_get_bit(tmp5, x)) // implying iZ- prime
                primes_obj_append(primes, iZ(x + yvx, -1));

            if (bitmap_get_bit(tmp7, x)) // implying iZ+ prime
                primes_obj_append(primes, iZ(x + yvx, 1));
        }
    }

    // 5. Clean up bitmaps and return results
    bitmap_free(x5);
    bitmap_free(x7);
    bitmap_free(tmp5);
    bitmap_free(tmp7);

    // Edge Case: If a prime or more > n got picked up, remove it
    while (primes->p_array[primes->p_count - 1] > n)
        primes->p_count--;

    // Trim unused memory in primes object
    if (primes_obj_resize_to_p_count(primes) < 0)
    {
        // Error: check logs
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}
