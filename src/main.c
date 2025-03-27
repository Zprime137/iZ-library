// main.c

#include <iZ.h>
#include <vx6.h>
#include <sieve.h>
#include <random_iZprime.h>
/**
 * @brief Tests the integrity of various sieve algorithms.
 *
 * This function validates the correctness of multiple sieve algorithms by testing them on
 * different numerical limits (n = 10^3, 10^6, and 10^9). It iterates over these limits
 * by increasing the exponent in steps of 3 and invokes the test_sieve_integrity function
 * for each case.
 *
 * @param sieve_models A structure containing different sieve algorithm implementations.
 */
void testing_sieve_integrity(SieveModels sieve_models)
{
    // Testing sieve algorithms integrity for n = 10^3, 10^6, 10^9
    print_line(92);
    printf("Testing sieve algorithms integrity");
    print_line(92);
    for (int i = 3; i < 10; i += 3)
        test_sieve_integrity(sieve_models, int_pow(10, i));
}

/**
 * @brief Benchmarks the performance of sieve algorithms.
 *
 * This function performs benchmarks on the sieve algorithms provided via the sieve_models parameter.
 * It benchmarks the algorithms for numbers in the range from 10^4 to 10^9 using the benchmark_sieve function.
 * Additionally, it measures and optionally saves the count of operations performed by each sieve algorithm
 * using the measure_sieve_w_op function.
 *
 * @param sieve_models A structure containing different sieve algorithm implementations.
 */
void testing_sieve_benchmarks(SieveModels sieve_models, int save_results)
{
    print_line(92);
    printf("Testing sieve algorithms benchmarks");
    print_line(92);
    // Benchmarking sieve algorithms from 10^4 to 10^9
    benchmark_sieve(sieve_models, 10, 4, 9, save_results);
    // Counting sieve operations for [Sieve of Eratosthenes, Sieve-iZ, Sieve-iZm]
    // measure_sieve_w_op(1); // pass 1 to save results, 0 to not save
}

/**
 * @brief Tests the performance of segmented sieve algorithms.
 *
 * This function evaluates the performance of different segmented sieve implementations such as Sieve_iZ,
 * SegmentedSieve, and Sieve_iZm for very high numerical limits. It tests these algorithms for n = 10^10,
 * and additionally for scaled limits (2*10^10 and 3*10^10) for the segmented sieve algorithms.
 */
void testing_segmented_sieve(void)
{
    print_line(92);
    printf("Testing segmented sieve");
    print_line(92);
    // Sieve testing for limit n = 10^10
    measure_sieve_time(Sieve_iZ, int_pow(10, 10));
    measure_sieve_time(SegmentedSieve, int_pow(10, 10));
    measure_sieve_time(Sieve_iZm, int_pow(10, 10));

    // Sieve testing for limit n = 2 * 10^10
    measure_sieve_time(SegmentedSieve, 2 * int_pow(10, 10));
    measure_sieve_time(Sieve_iZm, 2 * int_pow(10, 10));

    // Sieve testing for limit n = 3 * 10^10
    measure_sieve_time(SegmentedSieve, 3 * int_pow(10, 10));
    measure_sieve_time(Sieve_iZm, 3 * int_pow(10, 10));
}

/**
 * @brief Tests the vx6 sieve implementation.
 *
 * This function benchmarks the vx6 sieve algorithm by repeatedly testing it with progressively larger input sizes.
 * It starts with an initial buffer value ("1000") and, in each iteration, appends additional zeros to expand
 * the test size, invoking the test_vx6_sieve function on each modified buffer.
 */
void testing_vx6_sieve(int test_rounds)
{
    print_line(92);
    printf("Testing vx6 sieve");
    print_line(92);
    char buf[256] = "1000";
    for (int i = 0; i < test_rounds; i++)
    {
        test_vx6_sieve(buf);
        strcat(buf, "000000000");
    }
}

/**
 * @brief Tests the vx6 I/O operations.
 *
 * This function verifies the I/O operations related to the vx6 implementation. It does so by testing the writing
 * and reading of files using the test_vx6_file_io function over increasingly larger file sizes. The file
 * size is adjusted by appending additional zeros to a buffer that starts with the value "1000".
 */
void testing_vx6_io(int test_rounds)
{
    print_line(92);
    printf("Testing vx6 I/O operations");
    print_line(92);

    char buf[256] = "1000";
    char filename[256];

    for (int i = 0; i < test_rounds; i++)
    {
        sprintf(filename, "%s/test_%d", DIR_iZm, i);
        test_vx6_file_io(filename, buf);
        strcat(buf, "000000000");
    }
}

/**
 * @brief Benchmarks random prime generation algorithms.
 *
 * This function benchmarks the performance of random prime generation algorithms for various bit-sizes.
 * It tests for 1 KB, 2 KB, 4 KB, and 8 KB sizes (where 1 KB is defined to be 1024 bits) and performs a fixed
 * number of rounds for both primality checks and overall testing. The results can be optionally saved.
 *
 * Local Parameters:
 * - Kb: Represents one kilobyte in terms of bit-size (1024 bits).
 * - primality_check_rounds: Number of rounds to perform for primality testing (set to 25).
 * - test_rounds: Number of test iterations per bit-size (set to 5).
 * - save_results: Flag to determine if the results should be saved (0 indicates they should not be saved).
 */
void testing_prime_gen_algorithms(int save_results)
{
    print_line(92);
    printf("Testing random prime generation algorithms");
    print_line(92);
    // Random prime generation testing
    int Kb = 1024;
    int primality_check_rounds = 25;
    int test_rounds = 5;

    // Benchmark random prime generation algorithms for 1 KB bit-size
    benchmark_random_prime_algorithms(1 * Kb, primality_check_rounds, test_rounds, save_results);

    // Benchmark random prime generation algorithms for 2 KB bit-size
    benchmark_random_prime_algorithms(2 * Kb, primality_check_rounds, test_rounds, save_results);

    // Benchmark random prime generation algorithms for 4 KB bit-size
    benchmark_random_prime_algorithms(4 * Kb, primality_check_rounds, test_rounds, save_results);

    // Benchmark random prime generation algorithms for 8 KB bit-size
    benchmark_random_prime_algorithms(8 * Kb, primality_check_rounds, test_rounds, save_results);
}

// Entry point
int main(void)
{
    // Set log level to only log warnings and above
    log_set_log_level(LOG_DEBUG);

    SieveAlgorithm models_list[] = {
        SieveOfEratosthenes,
        // SieveOfEuler,
        // WheelSieve,
        // SieveOfAtkin,
        Sieve_iZ,
        SegmentedSieve,
        Sieve_iZm,
    };

    int models_count = sizeof(models_list) / sizeof(SieveAlgorithm);
    SieveModels sieve_models = {models_list, models_count};

    // testing integrity
    // testing_sieve_integrity(sieve_models);

    // Benchmarking sieve algorithms from 10^4 to 10^9
    testing_sieve_benchmarks(sieve_models, 1);

    // analyze_vx_potential_primes();

    // Testing vx6 sieve and I/O operations
    // testing_vx6_sieve(10);
    // testing_vx6_io(10);

    // Testing random prime generation algorithms
    // testing_prime_gen_algorithms(0);

    return 0;
}
