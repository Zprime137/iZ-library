#include <utils.h>
#include <random_iZprime.h>
#include <sys/time.h>

// List of prime generation algorithms: [iZp, gmp, ssl]
typedef enum
{
    iZp,    // random_iZprime
    GMP,    // gmp_nextprime
    OpenSSL // BN_generate_prime_ex
} PRIME_GEN_ALGORITHM;

// RANDOM_PRIME_RESULT structure
typedef struct
{
    PRIME_GEN_ALGORITHM algorithm;
    int bit_size;
    int cores_num;
    char **primes_list; // Array of string primes
    double *time_array; // Array of execution times
    int results_count;  // Number of results stored in the arrays
} RANDOM_PRIME_RESULT;

// create results list structure
typedef struct
{
    RANDOM_PRIME_RESULT *results;
    int results_count;
} RESULTS_LIST;

/**
 * @brief Frees all memory associated with a RESULTS_LIST structure
 *
 * This function cleans up all dynamically allocated memory within a RESULTS_LIST structure,
 * including each RANDOM_PRIME_RESULT's primes_list and time_array, and the results array itself.
 * It performs null checks to ensure safety.
 *
 * @param list Pointer to the RESULTS_LIST structure to be freed
 */
void free_results_list(RESULTS_LIST *list)
{
    if (!list)
        return;

    for (int i = 0; i < list->results_count; i++)
    {
        RANDOM_PRIME_RESULT *res = &list->results[i];
        if (res->primes_list)
        {
            for (int j = 0; j < res->results_count; j++)
            {
                free(res->primes_list[j]);
            }
            free(res->primes_list);
        }
        if (res->time_array)
        {
            free(res->time_array);
        }
    }
    free(list->results);
}

/**
 * @brief Prints the contents of a RESULTS_LIST structure
 *
 * This function displays the results stored in a RESULTS_LIST structure in a formatted way.
 * For each result in the list, it prints:
 * - The algorithm used (iZ, GMP, or OpenSSL)
 * - Bit size
 * - Number of cores used
 * - For each prime number generated:
 *   - The prime number itself
 *   - Its size in base 2 and base 10
 *   - Time taken to generate it
 * - The average time taken across all results
 *
 * @param list Pointer to the RESULTS_LIST structure to be printed
 */
void print_results_list(const RESULTS_LIST *list)
{
    if (!list)
        return;

    for (int i = 0; i < list->results_count; i++)
    {
        const RANDOM_PRIME_RESULT *res = &list->results[i];
        print_line(64);
        printf("Algorithm: ");
        if (res->algorithm == iZp)
            printf("iZ: random_iZprime\n");
        else if (res->algorithm == GMP)
            printf("GMP: gmp_random_nextprime\n");
        else if (res->algorithm == OpenSSL)
            printf("OpenSSL: BN_generate_prime_ex\n");

        printf("Bit Size: %d\n", res->bit_size);
        printf("Cores Number: %d\n", res->cores_num);

        double total_time = 0;
        for (int j = 0; j < res->results_count; j++)
        {
            int prime_digits = strlen(res->primes_list[j]);
            int prime_bit_size = prime_digits * 3.32;

            print_line(32);
            printf("Prime Result %d: %s\n", j + 1, res->primes_list[j]);
            printf("Base2 Size: %d\n", prime_bit_size);
            printf("Base10 Size: %d\n", prime_digits);
            printf("Time: %f seconds\n", res->time_array[j]);
            total_time += res->time_array[j];
        }
        if (res->results_count)
        {
            print_line(32);
            printf("Average Time: %f seconds\n", total_time / res->results_count);
        }
    }
}

/**
 * @brief Measures the time to generate random primes using different algorithms.
 *
 * This function performs multiple tests to measure the performance of prime number
 * generation algorithms (iZp, GMP, or OpenSSL). For each test round, it:
 * 1. Generates a prime number using the algorithm specified in the result struct
 * 2. Measures the time taken for generation
 * 3. Stores the generated prime and timing information
 *
 * The function allocates memory for storing the results, which should be freed by the caller
 * when no longer needed.
 *
 * @param result Pointer to a RANDOM_PRIME_RESULT structure that will be populated with
 *               the generated primes and timing data. The algorithm field must be set before calling.
 * @param test_rounds Number of prime generations to perform
 * @param primality_check_rounds Number of rounds to use for primality testing (used by iZp algorithm)
 */
void measure_prime_gen_time(RANDOM_PRIME_RESULT *result, int test_rounds, int primality_check_rounds)
{
    if (!result || test_rounds <= 0)
        return;

    result->results_count = test_rounds;
    result->primes_list = malloc(test_rounds * sizeof(char *));
    result->time_array = malloc(test_rounds * sizeof(double));

    for (int i = 0; i < test_rounds; i++)
    {
        struct timeval start, end;
        double elapsed_seconds = 0;
        char *prime_str = NULL;

        if (result->algorithm == iZp)
        {
            // iZp algorithm: use random_iZprime
            mpz_t p;
            mpz_init(p);
            gettimeofday(&start, NULL);
            random_iZprime(p, -1, result->bit_size, primality_check_rounds, result->cores_num);
            gettimeofday(&end, NULL);
            prime_str = mpz_get_str(NULL, 10, p);
            mpz_clear(p);
        }
        else if (result->algorithm == GMP)
        {
            // GMP algorithm: use gmp_random_nextprime
            mpz_t p;
            mpz_init(p);
            gettimeofday(&start, NULL);
            gmp_random_nextprime(p, result->bit_size);
            gettimeofday(&end, NULL);
            prime_str = mpz_get_str(NULL, 10, p);
            mpz_clear(p);
        }
        else if (result->algorithm == OpenSSL)
        {
            // OpenSSL algorithm: use BN_generate_prime_ex
            BIGNUM *prime = BN_new();
            gettimeofday(&start, NULL);
            BN_generate_prime_ex(prime, result->bit_size, 1, NULL, NULL, NULL);
            gettimeofday(&end, NULL);
            prime_str = BN_bn2dec(prime);
            BN_free(prime);
        }

        elapsed_seconds = (end.tv_sec - start.tv_sec) +
                          (end.tv_usec - start.tv_usec) / 1e6;
        result->primes_list[i] = prime_str;
        result->time_array[i] = elapsed_seconds;
    }
}

/**
 * @brief Benchmarks different random prime generation algorithms and compares their performance.
 *
 * This function evaluates the performance of multiple prime generation implementations:
 * - GMP (GNU Multiple Precision Arithmetic Library)
 * - OpenSSL (if bit_size <= 2048)
 * - iZp (custom implementation) with 1, 4, and 8 cores
 *
 * The benchmark measures execution time for generating random primes of the specified bit
 * size across multiple test rounds. Results are displayed to standard output and optionally
 * saved to a timestamped file in the output directory.
 *
 * @param bit_size The number of bits for the generated prime numbers
 * @param primality_check_rounds Number of rounds for prime verification tests
 * @param test_rounds Number of times to repeat the prime generation for each algorithm
 * @param save_results Non-zero to save results to a file, zero to only display to stdout
 *
 * @note OpenSSL benchmarks are skipped for bit sizes > 2048 due to excessive runtime
 * @note Results are saved in a directory specified by DIR_output global variable
 * @note Memory allocated during the function is properly freed before returning
 */
void benchmark_random_prime_algorithms(int bit_size, int primality_check_rounds, int test_rounds, int save_results)
{
    // Number of benchmark variations:
    // 0: GMP, 1: OpenSSL, 2: iZp (1 core), 3: iZp (4 cores), 4: iZp (8 cores)
    const int total_variations = 5;
    RESULTS_LIST results_list;
    results_list.results_count = total_variations;
    results_list.results = malloc(total_variations * sizeof(RANDOM_PRIME_RESULT));

    int idx = 0;
    // GMP benchmark
    results_list.results[idx].algorithm = GMP;
    results_list.results[idx].bit_size = bit_size;
    results_list.results[idx].cores_num = 1;
    measure_prime_gen_time(&results_list.results[idx], test_rounds, primality_check_rounds);

    // OpenSSL benchmark if bit_size <= 2048, otherwise it takes too long
    if (bit_size <= 2048)
    {
        idx++;
        results_list.results[idx].algorithm = OpenSSL;
        results_list.results[idx].bit_size = bit_size;
        results_list.results[idx].cores_num = 1;
        measure_prime_gen_time(&results_list.results[idx], test_rounds, primality_check_rounds);
    }

    // iZp benchmark with 1 core
    idx++;
    results_list.results[idx].algorithm = iZp;
    results_list.results[idx].bit_size = bit_size;
    results_list.results[idx].cores_num = 1;
    measure_prime_gen_time(&results_list.results[idx], test_rounds, primality_check_rounds);

    // iZp benchmark with 4 cores
    idx++;
    results_list.results[idx].algorithm = iZp;
    results_list.results[idx].bit_size = bit_size;
    results_list.results[idx].cores_num = 4;
    measure_prime_gen_time(&results_list.results[idx], test_rounds, primality_check_rounds);

    // iZp benchmark with 8 cores
    idx++;
    results_list.results[idx].algorithm = iZp;
    results_list.results[idx].bit_size = bit_size;
    results_list.results[idx].cores_num = 8;
    measure_prime_gen_time(&results_list.results[idx], test_rounds, primality_check_rounds);

    // Print the results to stdout
    print_results_list(&results_list);

    // If save_results is non-zero, save the output to a file
    if (save_results)
    {
        struct stat st = {0};
        // Create the output directory if it doesn't exist.
        if (stat(DIR_output, &st) == -1)
            mkdir(DIR_output, 0700);

        // Get the current timestamp for file naming.
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp) - 1, "%Y%m%d%H%M%S", t);

        // Build the output file path.
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s/random_prime_results_%s.txt", DIR_output, timestamp);

        FILE *fp = fopen(file_path, "w");
        if (fp == NULL)
        {
            perror("fopen");
        }
        else
        {
            fprintf(fp, "Target Bit Size: %d\n", results_list.results[0].bit_size);
            for (int i = 0; i < results_list.results_count; i++)
            {
                RANDOM_PRIME_RESULT *res = &results_list.results[i];
                fprintf(fp, "\n-----\n");
                fprintf(fp, "Algorithm: ");
                if (res->algorithm == iZp)
                    fprintf(fp, "iZ: random_iZprime\n");
                else if (res->algorithm == GMP)
                    fprintf(fp, "GMP: gmp_random_nextprime\n");
                else if (res->algorithm == OpenSSL)
                    fprintf(fp, "OpenSSL: BN_generate_prime_ex\n");

                fprintf(fp, "Cores Number: %d\n\n", res->cores_num);
                double total_time = 0;
                for (int j = 0; j < res->results_count; j++)
                {
                    fprintf(fp, "Prime Result %d: %s\n", j + 1, res->primes_list[j]);
                    total_time += res->time_array[j];
                }
                fprintf(fp, "\nTime Results (seconds): [%f", res->time_array[0]);
                for (int j = 1; j < res->results_count; j++)
                {
                    fprintf(fp, ", %f", res->time_array[j]);
                }
                fprintf(fp, "]\n");
                if (res->results_count)
                {
                    fprintf(fp, "Average Time: %f seconds\n", total_time / res->results_count);
                }
            }
            fclose(fp);
            printf("\n\nResults saved to %s\n", file_path);
        }
    }

    // Free allocated memory
    free_results_list(&results_list);
}

/**
 * @brief Generates a random prime number using GMP's mpz_nextprime function.
 *
 * This function generates a random prime number of a given bit size using GMP's mpz_nextprime
 * function. It initializes a random base using GMP's random state and searches for the next prime
 * number. The prime number is stored in the provided mpz_t variable.
 *
 * @param p The mpz_t variable to store the generated prime number.
 * @param bit_size The bit size of the prime number to be generated.
 */
void gmp_random_nextprime(mpz_t p, int bit_size)
{
    // Set the initial random number within the magnitude range
    mpz_t base;
    mpz_init(base);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_seed_randstate(state); // seed random state

    // Generate a random number in the given range
    mpz_urandomb(base, state, bit_size);
    // Find the next prime number after the random base
    mpz_nextprime(p, base);

    mpz_clear(base);
    gmp_randclear(state);
}
