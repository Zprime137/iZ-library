#include <iZ.h>
#include <vx6.h>
#include <sieve.h>
#include <random_iZprime.h>

/**
 * @brief Tests the integrity of various sieve algorithms
 *
 * Verifies that all sieve algorithms in the provided models produce correct results
 * by testing them with powers of 10 (10^3, 10^6, 10^9).
 *
 * @param sieve_models Collection of sieve algorithm implementations to test
 */
void testing_sieve_integrity(SieveModels sieve_models)
{
    print_line(92);
    printf("Testing sieve algorithms integrity");
    print_line(92);
    for (int i = 3; i < 10; i += 3)
        test_sieve_integrity(sieve_models, int_pow(10, i));
}

/**
 * @brief Benchmarks the performance of various sieve algorithms
 *
 * Runs performance tests on all sieve algorithms in the provided models
 * using various input sizes.
 *
 * @param sieve_models Collection of sieve algorithm implementations to benchmark
 * @param save_results Flag indicating whether to save benchmark results to disk (1) or not (0)
 */
void testing_sieve_benchmarks(SieveModels sieve_models, int save_results)
{
    print_line(92);
    printf("Testing sieve algorithms benchmarks");
    print_line(92);
    benchmark_sieve(sieve_models, 10, 4, 9, save_results);
}

/**
 * @brief Tests and compares performance of segmented sieves
 *
 * Measures execution time for Sieve_iZ, SegmentedSieve, and Sieve_iZm with large values
 * (10^10, 2×10^10, and 3×10^10) to compare their performance characteristics.
 */
void testing_segmented_sieve(void)
{
    print_line(92);
    printf("Testing segmented sieve");
    print_line(92);
    // Testing for n = 10^10
    measure_sieve_time(Sieve_iZ, int_pow(10, 10));
    measure_sieve_time(SegmentedSieve, int_pow(10, 10));
    measure_sieve_time(Sieve_iZm, int_pow(10, 10));
    // Testing for n = 2*10^10
    // measure_sieve_time(SegmentedSieve, 2 * int_pow(10, 10));
    // measure_sieve_time(Sieve_iZm, 2 * int_pow(10, 10));
    // Testing for n = 3*10^10
    // measure_sieve_time(SegmentedSieve, 3 * int_pow(10, 10));
    // measure_sieve_time(Sieve_iZm, 3 * int_pow(10, 10));
}

/**
 * @brief Tests the vx6 sieve implementation with increasing values
 *
 * Runs the vx6 sieve algorithm with progressively larger numbers,
 * starting from 1000 and increasing by a factor of 10^9 each round.
 *
 * @param test_rounds Number of test rounds to perform
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
 * @brief Tests I/O operations for the vx6 sieve implementation
 *
 * Verifies file I/O functionality for the vx6 implementation by creating
 * test files with increasing sizes.
 *
 * @param test_rounds Number of test rounds to perform, each with progressively larger data
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
 * @brief Tests and benchmarks random prime generation algorithms
 *
 * Evaluates performance of prime generation algorithms with different
 * key sizes (1KB, 2KB, 4KB, 8KB) using multiple test rounds.
 */
void testing_prime_gen_algorithms(void)
{
    print_line(92);
    printf("Testing random prime generation algorithms for bit sizes 1024 bits");
    print_line(92);
    int Kb = 1024;
    int primality_check_rounds = 25;
    benchmark_random_prime_algorithms(Kb, primality_check_rounds, 1, 0);
}
