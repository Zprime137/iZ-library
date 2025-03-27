// sieve.c
#include <bitmap.h>
#include <primes_obj.h>
#include <iZ.h>

#include <sieve.h>

// Sieve Algorithms:

// Traditional Sieve of Eratosthenes algorithm
SieveAlgorithm ClassicSieveOfEratosthenes = {classic_sieve_eratosthenes, "Classic Sieve of Eratosthenes"};

/**
 * @brief Traditional Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @param n The upper limit to find primes.
 * @return PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 */
PRIMES_OBJ *classic_sieve_eratosthenes(uint64_t n)
{
    // Initialize the primes object with an estimated capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    // Create a bitmap to mark prime numbers
    BITMAP *n_bits = bitmap_create(n + 1);
    bitmap_set_all(n_bits);

    uint64_t n_sqrt = sqrt(n);

    // Sieve algorithm to mark non-prime numbers
    for (uint64_t p = 2; p <= n; p++)
    {
        if (bitmap_get_bit(n_bits, p))
        {
            primes_obj_append(primes, p);
            if (p <= n_sqrt)
                bitmap_clear_mod_p(n_bits, p, p * p, n + 1);
        }
    }

    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    if (primes_obj_resize_to_p_count(primes) != 0)
    {
        // Error already logged in primes_obj_resize
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}

// Optimized Sieve of Eratosthenes algorithm
SieveAlgorithm SieveOfEratosthenes = {sieve_eratosthenes, "Sieve of Eratosthenes"};

/**
 * @brief Optimized Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @param n The upper limit to find primes.
 * @return A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *         or NULL if memory allocation fails.
 */
PRIMES_OBJ *sieve_eratosthenes(uint64_t n)
{
    // Initialize the primes object with an estimated capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    // Create a bitmap to mark prime numbers
    BITMAP *n_bits = bitmap_create(n + 1);
    bitmap_set_all(n_bits);

    uint64_t n_sqrt = sqrt(n);

    // Add 2 as the first prime
    primes_obj_append(primes, 2);

    // Sieve algorithm to mark non-prime numbers, skipping even numbers
    for (uint64_t p = 3; p <= n; p += 2)
    {
        if (bitmap_get_bit(n_bits, p))
        {
            primes_obj_append(primes, p);
            if (p <= n_sqrt)
                bitmap_clear_mod_p(n_bits, 2 * p, p * p, n + 1);
        }
    }

    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    if (primes_obj_resize_to_p_count(primes) != 0)
    {
        // Error already logged in primes_obj_resize
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}

// Segmented Sieve of Eratosthenes algorithm
SieveAlgorithm SegmentedSieve = {segmented_sieve, "Segmented Sieve"};

/**
 * @brief Segmented Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @param n The upper limit to find primes.
 * @return A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *         or NULL if memory allocation fails.
 */
PRIMES_OBJ *segmented_sieve(uint64_t n)
{
    // Initialize PRIMES_OBJ with an estimated capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);
    if (primes == NULL)
    {
        // Error already logged in primes_obj_init
        return NULL;
    }

    // Define the segment size; can be tuned based on memory constraints
    uint64_t segment_size = (uint64_t)sqrt(n);

    // Step 1: Sieve small primes up to sqrt(n) using the traditional sieve
    BITMAP *n_bits = bitmap_create(segment_size + 1); // +1 to include segment_size
    if (n_bits == NULL)
    {
        primes_obj_free(primes);
        return NULL;
    }
    bitmap_set_all(n_bits);

    primes->p_array[primes->p_count++] = 2; // Add 2 as the first prime

    // Sieve odd numbers starting from 3 up to segment_size
    for (uint64_t p = 3; p <= segment_size; p += 2)
    {
        if (bitmap_get_bit(n_bits, p))
        {
            primes_obj_append(primes, p);

            // Start marking multiples of p from p*p within the small_bits
            for (uint64_t multiple = p * p; multiple <= segment_size; multiple += 2 * p)
                bitmap_clear_bit(n_bits, multiple);
        }
    }

    // Step 2: Segmented sieve
    uint64_t low = segment_size + 1;
    uint64_t high = low + segment_size - 1;
    if (high > n)
        high = n;

    // Iterate over segments
    while (low <= n)
    {
        bitmap_set_all(n_bits);

        // Sieve the current segment using the small primes
        for (int i = 0; i < primes->p_count; i++)
        {
            uint64_t p = primes->p_array[i];
            if (p * p > high)
                break;

            // Find the minimum number in [low, high] that is a multiple of p
            uint64_t start = (low / p) * p;
            if (start < low)
                start += p;
            if (start < p * p)
                start = p * p;

            // Mark multiples of p within the segment
            for (uint64_t j = start; j <= high; j += p)
            {
                // Skip even multiples
                if (j % 2 == 0)
                    continue;

                size_t index = j - low;
                bitmap_clear_bit(n_bits, index);
            }
        }

        // Collect primes from the current segment
        for (uint64_t i = low; i <= high; i++)
        {
            // Skip even numbers
            if (i % 2 == 0)
                continue;

            if (bitmap_get_bit(n_bits, i - low))
                primes_obj_append(primes, i);
        }

        // Move to the next segment
        low = high + 1;
        high = low + segment_size - 1;
        if (high > n)
            high = n;
    }

    // Step 3: Finalize
    // Trim the primes array to the exact number of primes found
    if (primes_obj_resize_to_p_count(primes) != 0)
    {
        // Error already logged in primes_obj_resize
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}

// Wheel Sieve
SieveAlgorithm WheelSieve = {sieve_wheel, "Wheel Sieve"};

/**
 * @brief Generates a list of prime numbers up to a given limit using a wheel sieve algorithm.
 *
 * This function uses a static wheel sieve approach to generate prime numbers up to the specified limit `n`.
 * The algorithm optimizes the sieve of Eratosthenes by skipping multiples of small base primes.
 *
 * @param n The upper limit up to which prime numbers are to be generated.
 * @return A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *         or NULL if memory allocation fails.
 */
PRIMES_OBJ *sieve_wheel(uint64_t n)
{
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    uint64_t base_primes[] = {2, 3, 5};
    uint64_t base_size = 3;
    uint64_t wheel_size = 2 * 3 * 5; // Product of base primes

    primes_obj_append(primes, 2);
    primes_obj_append(primes, 3);
    primes_obj_append(primes, 5);

    BITMAP *is_prime = bitmap_create(wheel_size);
    BITMAP *sieve = bitmap_create(n + 1);
    bitmap_set_all(sieve);

    // sieve logic
    for (uint64_t i = 0; i < base_size; ++i)
    {
        uint64_t prime = base_primes[i];
        for (uint64_t j = prime * prime; j < wheel_size; j += 2 * prime)
            bitmap_clear_bit(is_prime, j);

        bitmap_set_bit(is_prime, prime);
    }

    for (uint64_t i = 0; i < base_size; ++i)
    {
        uint64_t prime = base_primes[i];
        for (uint64_t j = prime * prime; j <= n; j += prime)
            bitmap_clear_bit(sieve, j);
    }

    for (uint64_t p = 7; p <= sqrt(n); p += 2)
    {
        if (bitmap_get_bit(sieve, p))
        {
            for (uint64_t j = p * p; j <= n; j += 2 * p)
                bitmap_clear_bit(sieve, j);
        }
    }

    for (uint64_t p = 7; p <= n; p += 2)
    {
        if (bitmap_get_bit(sieve, p))
            primes_obj_append(primes, p);
    }

    // cleanup
    bitmap_free(is_prime);
    bitmap_free(sieve);

    // Resize primes array to fit the exact number of primes found
    if (primes_obj_resize_to_p_count(primes) != 0)
    {
        // Error already logged in primes_obj_resize
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}

// Sieve of Euler
SieveAlgorithm SieveOfEuler = {sieve_euler, "Sieve of Euler"};

/**
 * @brief Generates a list of prime numbers up to a given limit using the Euler Sieve algorithm.
 *
 * This function uses the Euler Sieve algorithm to find all prime numbers up to a specified limit `n`.
 * It initializes a bitmap to mark non-prime numbers and an array to store the prime numbers found.
 * The algorithm skips even numbers and starts the prime list with 2.
 *
 * @param n The upper limit up to which prime numbers are to be found.
 * @return A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *         or NULL if memory allocation fails.
 */
PRIMES_OBJ *sieve_euler(uint64_t n)
{
    // initialization
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);
    BITMAP *n_bits = bitmap_create(n + 1);
    bitmap_set_all(n_bits);

    // starting the prime list with 2 to skip reading even numbers
    primes_obj_append(primes, 2);

    // sieve logic
    for (uint64_t i = 3; i <= n; i += 2)
    {
        if (bitmap_get_bit(n_bits, i))
            primes_obj_append(primes, i);

        for (int j = 1; j < primes->p_count; ++j)
        {
            uint64_t p = primes->p_array[j];

            if (p * i > n)
                break;

            bitmap_clear_bit(n_bits, p * i);

            if (i % p == 0)
                break;
        }
    }

    // cleanup
    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    if (primes_obj_resize_to_p_count(primes) != 0)
    {
        // Error already logged in primes_obj_resize
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}

// Sieve of Atkin
SieveAlgorithm SieveOfAtkin = {sieve_atkin, "Sieve of Atkin"};

/**
 * @brief Generates a list of prime numbers up to a given limit using the Sieve of Atkin algorithm.
 *
 * This function implements the Sieve of Atkin, an efficient algorithm to find all prime numbers up to a specified integer `n`.
 * It initializes a bitmap to mark potential primes and applies the Atkin conditions to identify primes.
 * The function also marks multiples of squares of primes as non-prime and collects the primes into a dynamically allocated array.
 *
 * @param n The upper limit (inclusive) up to which prime numbers are to be found.
 * @return A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *         or NULL if memory allocation fails.
 */
PRIMES_OBJ *sieve_atkin(uint64_t n)
{
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);
    BITMAP *n_bits = bitmap_create(n + 1);

    uint64_t n_sqrt = sqrt(n) + 1;

    // Initialize 2 and 3 as primes
    primes_obj_append(primes, 2);
    primes_obj_append(primes, 3);

    // 1. Mark potential primes in the bitmap using the Atkin conditions
    for (uint64_t x = 1; x < n_sqrt; ++x)
    {
        for (uint64_t y = 1; y < n_sqrt; ++y)
        {
            uint64_t num = 4 * x * x + y * y;
            if (num <= n && (num % 12 == 1 || num % 12 == 5))
                bitmap_set_bit(n_bits, num); // Toggle the bit

            num = 3 * x * x + y * y;
            if (num <= n && num % 12 == 7)
                bitmap_set_bit(n_bits, num); // Toggle the bit

            num = 3 * x * x - y * y;
            if (x > y && num <= n && num % 12 == 11)
                bitmap_set_bit(n_bits, num); // Toggle the bit
        }
    }

    // 2. Remove composites by sieving
    for (uint64_t i = 5; i <= n_sqrt; i++)
    {
        if (bitmap_get_bit(n_bits, i))
        {
            // Mark multiples of i^2 as non-prime
            for (uint64_t j = i * i; j <= n; j += i)
                bitmap_clear_bit(n_bits, j);
        }
    }

    // 3. Collect primes from the bitmap
    for (uint64_t i = 5; i <= n; i += 2)
    {
        if (bitmap_get_bit(n_bits, i))
            primes_obj_append(primes, i);
    }

    // cleanup
    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    if (primes_obj_resize_to_p_count(primes) != 0)
    {
        // Error already logged in primes_obj_resize
        primes_obj_free(primes);
        return NULL;
    }

    return primes;
}
