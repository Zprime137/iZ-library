// test_all.c - Test Harness for Sieve Algorithms and Related Functionalities

#include <iZ.h>
#include <vx6.h>
#include <sieve.h>
#include <random_iZprime.h>

// Test functions prototypes
void testing_sieve_integrity(SieveModels sieve_models);
void testing_sieve_benchmarks(SieveModels sieve_models, int save_results);
void testing_segmented_sieve(void);
void testing_vx6_sieve(int test_rounds);
void testing_vx6_io(int test_rounds);
void testing_prime_gen_algorithms(void);

int main(void)
{
    // Set log level to debug to show detailed test output
    log_set_log_level(LOG_DEBUG);

    // Define the list of sieve algorithms to test.
    SieveAlgorithm models_list[] = {
        SieveOfEratosthenes,
        SieveOfEuler,
        WheelSieve,
        SieveOfAtkin,
        Sieve_iZ,
        SegmentedSieve,
        Sieve_iZm};

    int models_count = sizeof(models_list) / sizeof(SieveAlgorithm);
    SieveModels sieve_models = {models_list, models_count};

    printf("=== Running All Tests ===\n");

    // Run the suite of tests.
    testing_sieve_integrity(sieve_models);
    testing_sieve_benchmarks(sieve_models, 0);
    testing_segmented_sieve();
    testing_vx6_sieve(1);
    testing_vx6_io(1);
    testing_prime_gen_algorithms();

    printf("=== Tests Completed ===\n");

    return 0;
}
