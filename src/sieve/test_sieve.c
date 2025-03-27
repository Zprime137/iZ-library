#include <iZ.h>
#include <sieve.h>

/**
 * @brief Tests the integrity of different sieve models by comparing their hash values.
 *
 * This function iterates through a list of sieve models, computes the prime numbers up to `n` using each model,
 * and then compares the SHA-256 hash of the resulting prime numbers. If all hashes match, the integrity is confirmed.
 *
 * @param sieve_models A structure containing the list of sieve models to be tested.
 * @param n The upper limit for the prime number generation.
 * @return 0 if the integrity is confirmed, -1 if a hash mismatch is detected.
 */
int test_sieve_integrity(SieveModels sieve_models, uint64_t n)
{
    unsigned char results[sieve_models.models_count][32]; // Store hashes for each model

    for (int i = 0; i < sieve_models.models_count; i++)
    {
        SieveAlgorithm sieve_model = sieve_models.models_list[i];
        printf("Testing %s: n = %lld\n", sieve_model.name, n);

        // Call the sieve function
        PRIMES_OBJ *primes = sieve_model.function(n);
        primes_obj_compute_hash(primes);

        printf("Primes Count : %d <= %lld\n", primes->p_count, n);
        printf("Last Prime: %lld\n", primes->p_array[primes->p_count - 1]);
        print_sha256_hash(primes->sha256);

        // Store the hash in the results array
        memcpy(results[i], primes->sha256, 32);

        primes_obj_free(primes);
        printf("=================================\n");
    }

    // Compare all hashes to the first hash
    int all_hashes_match = 1;
    for (int i = 1; i < sieve_models.models_count; i++)
    {
        if (memcmp(results[0], results[i], 32) != 0)
        {
            all_hashes_match = 0;
            printf("Hash mismatch detected for %s\n", sieve_models.models_list[i].name);
        }
    }

    if (all_hashes_match)
    {
        printf("All hashes match. Integrity confirmed ^_^\n");
        return 0;
    }

    else
    {
        printf("Hash mismatch detected. Integrity not confirmed :\\\n");
        return -1;
    }
}

/**
 * @brief Measures the execution time of a given sieve algorithm.
 *
 * This function takes a sieve algorithm and an upper limit `n`, runs the algorithm,
 * and measures the time taken to execute it. It prints the algorithm name, the value of `n`,
 * the count of prime numbers found, the last prime number in the list, and the time taken in seconds.
 * It returns the execution time in microseconds.
 *
 * @param algorithm The sieve algorithm to be measured.
 * @param n The upper limit for the sieve algorithm.
 * @return The execution time in microseconds.
 */
size_t measure_sieve_time(SieveAlgorithm model, uint64_t n)
{
    clock_t start, end;
    double cpu_time_used;

    start = clock();                        // Start time
    PRIMES_OBJ *primes = model.function(n); // Run time
    end = clock();                          // End time

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("| %-16lld", n);
    printf("| %-16d", primes->p_count);
    printf("| %-16lld", primes->p_array[primes->p_count - 1]);
    printf("| %-16f\n", cpu_time_used);

    primes_obj_free(primes);
    return (size_t)(cpu_time_used * 1000000); // time in microseconds;
}

// Function to save benchmarks results to a file named by timestamp
/**
 * @brief Saves the results of the sieve models to a file.
 *
 * This function creates a directory if it does not exist, generates a timestamped
 * filename, and writes the results of the sieve models to this file. The results
 * include metadata about the test range and the results of each sieve model.
 *
 * @param sieve_models The sieve models containing the algorithms and their names.
 * @param all_results A 2D array containing the results of the sieve models.
 * @param base The base number used in the test range.
 * @param min_exp The minimum exponent used in the test range.
 * @param max_exp The maximum exponent used in the test range.
 */
static void save_sieve_results_file(SieveModels sieve_models, int all_results[][32], int base, int min_exp, int max_exp)
{
    struct stat st = {0};
    if (stat(DIR_output, &st) == -1)
        mkdir(DIR_output, 0700);

    // Get the current timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp) - 1, "%Y%m%d%H%M%S", t);

    // Create the output file path
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/sieve_results_%s.txt", DIR_output, timestamp);

    // Open the file for writing
    FILE *fp = fopen(file_path, "w");
    if (fp == NULL)
    {
        log_error("Failed to open file");
        return;
    }

    // Write the test range metadata
    fprintf(fp, "Test Range: %d^%d:%d^%d\n", base, min_exp, base, max_exp);

    // Write the results to the file
    for (int i = 0; i < sieve_models.models_count; i++)
    {
        SieveAlgorithm model = sieve_models.models_list[i];

        fprintf(fp, "%s: [", model.name);
        for (int j = 0; j <= max_exp - min_exp; j++)
        {
            fprintf(fp, "%d", all_results[i][j]);
            if (j < max_exp - min_exp)
                fprintf(fp, ", ");
        }
        fprintf(fp, "]\n");
    }

    // Close the file
    fclose(fp);
    printf("\nResults saved to %s\n", file_path);
}

/**
 * @brief Benchmarks the performance of different sieve algorithms over a range of exponents.
 *
 * This function measures the execution time of various sieve algorithms for a range of values
 * determined by the base raised to the power of exponents from min_exp to max_exp. The results
 * are printed and optionally saved to a file.
 *
 * @param sieve_models A structure containing the list of sieve algorithms to benchmark.
 * @param base The base value to be raised to the power of exponents.
 * @param min_exp The minimum exponent value.
 * @param max_exp The maximum exponent value.
 * @param save_results A flag indicating whether to save the results to a file named by timestamp in the output directory.
 */
void benchmark_sieve(SieveModels sieve_models, int base, int min_exp, int max_exp, int save_results)
{
    int all_results[sieve_models.models_count][32];

    for (int i = 0; i < sieve_models.models_count; i++)
    {
        SieveAlgorithm model = sieve_models.models_list[i];

        // results array: [time in microsecond]
        int results[32];
        int k = 0;

        printf("\nAlgorithm: %s", model.name);
        print_line(75);
        printf("| %-16s", "n");
        printf("| %-16s", "Primes Count");
        printf("| %-16s", "Last Prime");
        printf("| %-16s", "Time (s)");
        print_line(75);

        for (int j = min_exp; j <= max_exp; j++)
            results[k++] = measure_sieve_time(model, pow(base, j)); // returns time in microseconds

        print_line(75);

        for (int j = 0; j < k; j++)
            all_results[i][j] = results[j];

        // print results array
        printf("Results summary of %s\n", model.name);
        printf("Test range: [%d^%d : %d^%d]\n", base, min_exp, base, max_exp);
        printf("Execution time in microseconds: [%d", results[0]);
        for (int j = 1; j < k; j++)
            printf(", %d", results[j]);
        printf("]\n");
        fflush(stdout);
    }

    // save results in a file named by timestamp
    if (save_results)
        save_sieve_results_file(sieve_models, all_results, base, min_exp, max_exp);
}

/**
 * @brief Measures the number of mark-composite operations of sieve-iZ algorithms against sieve-Eratosthenes.
 *
 * This function runs the sieve algorithms (sieve-Eratosthenes, sieve-iZ, sieve-iZm) for a range of incrementing exponents of 10
 * and counts the number of mark-composite operations required to mark all composite numbers.
 * The results are printed to the stdout, and optionally saved to a file.
 */
void measure_sieve_w_op(int save_results)
{
    printf("Analyzing 'Mark-Composite' Write Operations W(n)\n");
    printf("| %-24s", "n");
    printf("| %-24s", "Sieve-Eratosthenes");
    printf("| %-24s", "Sieve-iZ");
    printf("| %-24s", "Sieve-iZm");
    print_line(100);

    // create results array to store each algorithms counts
    int result_i = 0;
    size_t eratosthenes_results[10];
    size_t iZ_results[10];
    size_t iZm_results[10];

    for (int i = 3; i <= 10; i++)
    {
        uint64_t n = int_pow(10, i);
        uint64_t n_sqrt = sqrt(n);

        PRIMES_OBJ *primes = NULL;
        primes = Sieve_iZm.function(n);
        uint64_t x_n = n / 6 + 1;
        uint64_t vx = compute_limited_vx(x_n, 6);

        size_t eratosthenes_count = (n - 9) / 6; // skipping marks of 2, counting the marks of 3
        size_t iZ_count = 0;
        size_t iZm_count = 0;

        // iterate over root primes skipping 2, 3
        for (int j = 2; j < primes->p_count; j++)
        {
            uint64_t p = primes->p_array[j];

            // break if p^2 > n
            if (p > n_sqrt)
                break;

            eratosthenes_count += (n - p * p) / (2 * p); // optimized, counting only odd marks of p

            uint16_t x_p = p / 6;
            iZ_count += (2 * (x_n - (p * x_p))) / p;

            if (vx % p != 0)
            {
                iZm_count += (2 * (x_n - (p * x_p))) / p;
            }
        }

        eratosthenes_results[result_i] = eratosthenes_count;
        iZ_results[result_i] = iZ_count;
        iZm_results[result_i] = iZm_count;
        result_i++;

        printf("| %-24lld", n);
        printf("| %-24ld", eratosthenes_count);
        printf("| %-24ld", iZ_count);
        printf("| %-24ld\n", iZm_count);

        // free memory
        primes_obj_free(primes);
    }

    // save results in a file named by timestamp
    if (save_results)
    {
        struct stat st = {0};
        if (stat(DIR_output, &st) == -1)
            mkdir(DIR_output, 0700);

        // Create the output file path
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s/sieve_w_op.txt", DIR_output);

        // Open the file for writing
        FILE *fp = fopen(file_path, "w");
        if (fp == NULL)
        {
            log_error("Failed to open file");
            return;
        }

        // Write the results to the file
        fprintf(fp, "| %-24s", "n");
        fprintf(fp, "| %-24s", "Sieve-Eratosthenes");
        fprintf(fp, "| %-24s", "Sieve-iZ");
        fprintf(fp, "| %-24s", "Sieve-iZm");
        fprintf(fp, "\n");

        for (int i = 0; i < result_i; i++)
        {
            uint64_t n = int_pow(10, i + 3);
            fprintf(fp, "| %-24lld", n);
            fprintf(fp, "| %-24ld", eratosthenes_results[i]);
            fprintf(fp, "| %-24ld", iZ_results[i]);
            fprintf(fp, "| %-24ld\n", iZm_results[i]);
        }

        // Close the file
        fclose(fp);
        printf("\nResults saved to %s\n\n", file_path);
    }
}
