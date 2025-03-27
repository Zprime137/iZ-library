#include <vx6.h>
#include <iZ.h>
#include <sieve.h>

// Cached primes object
static PRIMES_OBJ *cached_vx6_primes = NULL;

// Cache primes below vx6 into cached_vx6_primes
static void cache_primes_obj(void)
{
    if (cached_vx6_primes == NULL)
        cached_vx6_primes = sieve_iZ(vx6);
}

// Clear cached primes object
static void clear_cached_primes_obj(void)
{
    if (cached_vx6_primes != NULL)
    {
        primes_obj_free(cached_vx6_primes);
        cached_vx6_primes = NULL;
    }
}

// Cached pre-sieved bitmaps for x5 and x7
static BITMAP *cached_x5 = NULL;
static BITMAP *cached_x7 = NULL;

// Cache pre-sieved bitmaps for x5 and x7
static void cache_vx6_bitmaps(void)
{
    if (cached_x5 == NULL)
    {
        cached_x5 = bitmap_create(vx6 + 100);
        cached_x7 = bitmap_create(vx6 + 100);
        construct_iZm_segment(vx6, cached_x5, cached_x7);
    }
}

// Clear cached bitmaps
static void clear_cached_vx6_bitmaps(void)
{
    if (cached_x5 != NULL)
    {
        bitmap_free(cached_x5);
        bitmap_free(cached_x7);
        cached_x5 = NULL;
        cached_x7 = NULL;
    }
}

/**
 * @brief Initialize the vx6 object.
 *
 * @param y A character pointer representing a numeric string.
 * @return VX_OBJ* A pointer to the initialized VX_OBJ structure.
 *        NULL if memory allocation fails or y is not a numeric string.
 */
VX_OBJ *vx6_init(const char *y)
{
    // check if y is numeric string
    if (!is_numeric_str(y))
    {
        printf("Invalid y string in vx6_init\n");
        return NULL;
    }

    VX_OBJ *vx_obj = malloc(sizeof(VX_OBJ));
    if (vx_obj == NULL)
    {
        log_error("Memory allocation failed in vx6_init\n");
        return NULL;
    }

    vx_obj->vx = vx6;
    vx_obj->y = y;
    vx_obj->p_count = 0;
    vx_obj->p_gaps = malloc(vx6 / 2 * P_GAPS_SIZE); // initial estimate

    return vx_obj;
}

// Resize p_gaps array to fit the actual count vx_obj->p_count
void vx6_resize_p_gaps(VX_OBJ *vx_obj)
{
    if (vx_obj == NULL)
        return;

    vx_obj->p_gaps = realloc(vx_obj->p_gaps, vx_obj->p_count * sizeof(int));
}

// Free all memory allocated for vx_obj
void vx6_free(VX_OBJ *vx_obj)
{
    if (vx_obj == NULL)
        return;

    if (vx_obj->x5)
        bitmap_free(vx_obj->x5);
    if (vx_obj->x7)
        bitmap_free(vx_obj->x7);
    if (vx_obj->p_gaps)
        free(vx_obj->p_gaps);

    free(vx_obj);
    vx_obj = NULL;
}

/**
 * vx6_write_file - Write a VX_OBJ structure to a binary file.
 *
 * @vx_obj: Pointer to a VX_OBJ structure containing data to be written.
 * @filename: The name (or path) of the file to write to. If the filename does not include the
 *            ".vx6" extension, it is automatically appended.
 *
 * This function serializes the VX_OBJ structure into a binary file. It writes the following data:
 *   - The length of the y string (including the terminating null character) followed by the y string.
 *   - The p_count value indicating the number of elements in the p_gaps array.
 *   - The p_gaps array itself.
 *   - A SHA256 hash computed over the p_gaps array for data integrity, which is then written to the file.
 *
 * Return: Returns 1 on successful write, and 0 if any error occurs (e.g., invalid parameters,
 *         failure to open the file, or file write errors).
 */
int vx6_write_file(VX_OBJ *vx_obj, const char *filename)
{
    if (vx_obj == NULL || filename == NULL)
        return 0;

    // check if filename includes the extension .vx6, if not append it
    if (strstr(filename, VX6_EXT) == NULL)
        strcat(filename, VX6_EXT);

    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        log_error("Could not open file %s for writing\n", filename);
        return 0;
    }

    // Write the length of the y string including null terminator
    size_t y_len = strlen(vx_obj->y) + 1;
    fwrite(&y_len, sizeof(size_t), 1, file);

    // Write the y string
    fwrite(vx_obj->y, sizeof(char), y_len, file);

    // Write p_count
    fwrite(&vx_obj->p_count, sizeof(size_t), 1, file);

    // Write p_gaps array
    fwrite(vx_obj->p_gaps, P_GAPS_SIZE, vx_obj->p_count, file);

    // Calculate and write SHA256 hash of p_gaps
    hash_int_array((int *)vx_obj->p_gaps, vx_obj->p_count * P_GAPS_SIZE, vx_obj->sha256);

    // Write hash
    fwrite(vx_obj->sha256, SHA256_DIGEST_LENGTH, 1, file);

    fclose(file);
    return 1;
}

/**
 * vx6_read_file - Read a VX_OBJ structure from a binary file.
 *
 * @vx_obj: Pointer to a VX_OBJ structure where the read data will be stored.
 * @filename: The name (or path) of the file to read from. If the filename does not include the
 *            ".vx6" extension, it is automatically appended.
 *
 * This function reads the VX_OBJ structure from a binary file by performing the following steps:
 *   - Reads the length of the y string and allocates memory for it, then reads the y string.
 *   - Reads the p_count value to determine the number of elements in the p_gaps array.
 *   - Reads the p_gaps array (assuming the p_gaps field in vx_obj has been properly allocated).
 *   - Reads the previously stored SHA256 hash and computes a new hash on the read p_gaps array.
 *   - Compares the computed hash with the read hash to validate data integrity.
 *
 * Return: Returns 1 if the file is successfully read and the hash validation passes, and 0
 *         if any error occurs (e.g., invalid parameters, file read errors, memory allocation failure,
 *         or hash mismatch).
 */
int vx6_read_file(VX_OBJ *vx_obj, const char *filename)
{
    if (vx_obj == NULL || filename == NULL)
        return 0;

    // check if filename includes the extension .vx6, if not append it
    if (strstr(filename, VX6_EXT) == NULL)
        strcat((char *)filename, VX6_EXT);

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Could not open file %s for reading\n", filename);
        return 0;
    }

    // Read the length of the y string and allocate memory
    size_t y_len;
    fread(&y_len, sizeof(size_t), 1, file);
    vx_obj->y = malloc(y_len);

    if (vx_obj->y == NULL)
    {
        log_error("Memory allocation failed for y in vx6_read_file\n");
        fclose(file);
        return 0;
    }
    fread(vx_obj->y, sizeof(char), y_len, file);

    // Read p_count
    if (fread(&vx_obj->p_count, sizeof(size_t), 1, file) != 1)
    {
        fclose(file);
        return 0;
    }

    // Read p_gaps array (assumes vx_obj->p_gaps has been allocated with enough size)
    if (fread(vx_obj->p_gaps, P_GAPS_SIZE, vx_obj->p_count, file) != vx_obj->p_count)
    {
        fclose(file);
        return 0;
    }

    // Read hash
    if (fread(vx_obj->sha256, SHA256_DIGEST_LENGTH, 1, file) != 1)
    {
        fclose(file);
        return 0;
    }

    // Validate hash
    unsigned char computed_hash[SHA256_DIGEST_LENGTH];
    hash_int_array((int *)vx_obj->p_gaps, vx_obj->p_count * P_GAPS_SIZE, computed_hash);

    if (validate_sha256_hash(vx_obj->sha256, computed_hash) == 0)
    {
        printf("Error: SHA-256 hash mismatch\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

/**
 * vx6_sieve - Sieve composites and detect prime gaps in the vx6 segment.
 *
 * This function performs a sieving process on a set of candidates defined by the VX_OBJ
 * structure. It begins by caching small primes and uses these to mark composite numbers in
 * two separate bitmaps using different modular congruences. The algorithm computes a series
 * of big integers using the GMP library to derive thresholds and limits for the sieve.
 *
 * For candidates not eliminated by direct marking, the function applies a probabilistic
 * Miller-Rabin primality test (with a configurable number of rounds) to confirm their primality.
 * Prime gaps are accumulated by tracking the interval distances between consecutive primes detected.
 * During the process, performance metrics such as the number of marking operations and the number
 * of primality tests executed are recorded and printed for analysis.
 *
 * Parameters:
 * @param vx_obj        Pointer to the VX_OBJ structure holding the sieving bitmaps, gap tracking,
 *                      and the string representation of y.
 * @param p_test_rounds Number of rounds to be used by the Miller-Rabin primality test.
 * @param filename      Optional file path to which the detected prime gaps will be written.
 */
void vx6_sieve(VX_OBJ *vx_obj, int p_test_rounds, const char *filename)
{
    // caches primes < vx6 and the pre-sieved bitmaps
    cache_primes_obj();
    cache_vx6_bitmaps();

    // clone the pre-sieved cached bitmaps
    vx_obj->x5 = bitmap_clone(cached_x5);
    vx_obj->x7 = bitmap_clone(cached_x7);

    // count of operations, for performance analysis
    int mark_ops = 0;   // count of mark operations
    int p_test_ops = 0; // primality test operations

    // initialize mpz_t y from vx_obj->y
    mpz_t y;
    mpz_init(y);
    mpz_set_str(y, vx_obj->y, 10);

    // compute yvx = y * vx6
    mpz_t yvx;
    mpz_init(yvx);
    mpz_mul_ui(yvx, y, vx6);

    // compute upper_limit = sqrt(iZ(vx6 * (y+1), 1))
    mpz_t upper_limit;
    mpz_init(upper_limit);
    mpz_set(upper_limit, yvx);
    mpz_add_ui(upper_limit, upper_limit, vx6);
    iZ_gmp(upper_limit, upper_limit, 1);
    mpz_sqrt(upper_limit, upper_limit);

    // initialize GMP reusable variables p, x_p
    mpz_t p, x_p;
    mpz_init(p);
    mpz_init(x_p);

    // flag to determine if probabilistic primality test is needed
    int is_large_limit = 1;

    int start_i = 8; // to skip primes < 23
    // mark composites of cached primes skipping start_i
    for (int i = start_i; i < cached_vx6_primes->p_count; i++)
    {
        // break if p > upper_limit
        mpz_set_ui(p, cached_vx6_primes->p_array[i]);
        if (mpz_cmp(p, upper_limit) > 0)
        {
            // turn off is_large_limit, results are deterministic
            is_large_limit = 0;
            break;
        }

        int int_p = cached_vx6_primes->p_array[i];
        int int_xp;

        int_xp = solve_for_x_gmp(-1, int_p, vx6, y);
        bitmap_clear_mod_p(vx_obj->x5, int_p, int_xp, vx6);
        int_xp = solve_for_x_gmp(1, int_p, vx6, y);
        bitmap_clear_mod_p(vx_obj->x7, int_p, int_xp, vx6);

        mark_ops += 2 * vx6 / int_p;
    }

    if (!p_test_rounds)
        p_test_rounds = 25; // default value if null or 0

    int gap = 18; // skipping 3 * (4 + 2) = 18, as x < 3 can't be prime
    // perform probabilistic sieve for remaining candidates if is_large_limit
    for (int x = 4; x <= vx6; x++)
    {
        gap += 4;

        // check if iZ(x + vx6 * y, -1) is prime, if not clear x in x5
        if (bitmap_get_bit(vx_obj->x5, x))
        {
            int is_prime = 1;

            if (is_large_limit)
            {
                // compute x_p = x + vx6 * y
                mpz_add_ui(x_p, yvx, x);
                iZ_gmp(p, x_p, -1); // compute p = iZ(x_p, -1)
                is_prime = mpz_probab_prime_p(p, p_test_rounds);
                p_test_ops++;
            }

            if (is_prime)
            {
                vx_obj->p_gaps[vx_obj->p_count++] = gap;
                gap = 0; // reset gap
            }
            else
            {
                bitmap_clear_bit(vx_obj->x5, x);
            }
        }

        gap += 2;

        // same for iZ+
        if (bitmap_get_bit(vx_obj->x7, x))
        {
            int is_prime = 1;

            if (is_large_limit)
            {
                mpz_add_ui(x_p, yvx, x);
                iZ_gmp(p, x_p, 1); // compute p = iZ(x_p, 1)
                is_prime = mpz_probab_prime_p(p, p_test_rounds);
                p_test_ops++;
            }

            if (is_prime)
            {
                vx_obj->p_gaps[vx_obj->p_count++] = gap;
                gap = 0; // reset gap
            }
            else
            {
                bitmap_clear_bit(vx_obj->x7, x);
            }
        }
    }

    // resize p_gaps array to fit the actual count
    vx6_resize_p_gaps(vx_obj);

    // write p_gaps to a file if filename is provided
    if (filename)
        vx6_write_file(vx_obj, filename);

    // clear GMP variables
    mpz_clear(p);
    mpz_clear(x_p);
    mpz_clear(y);
    mpz_clear(yvx);
    mpz_clear(upper_limit);

    // print sieve statistics
    print_line(92);
    printf("Sieve Operation Statistics");
    print_line(92);
    printf("#marking operations: %d\n", mark_ops);
    printf("#primality testing operations: %d\n", p_test_ops);
    printf("#p_count: %d\n", vx_obj->p_count);
    print_line(92);
}

// print p_gaps array
void print_p_gaps(uint16_t *p_gaps, size_t p_count)
{
    printf("p_gaps: [%d, ", p_gaps[0]);
    for (int i = 1; i < (int)p_count; i++)
        printf("%d, ", p_gaps[i]);
    printf("...]\n");
}

/**
 * test_vx6_sieve - Tests the sieving functionality of the vx6 algorithm.
 *
 * This function performs the following steps:
 *   - Initializes a VX_OBJ structure using the provided string 'y'.
 *   - Measures the execution time of the vx6_sieve function.
 *   - Prints sieve statistics including identifier, prime count, and execution time.
 *   - Computes a base value using GMP arithmetic and verifies the first 10 primes by
 *     incrementing the base with each prime gap and validating primality with the Miller-Rabin test.
 *   - Outputs prime gap details and their validation status.
 *
 * Parameters:
 *   @y: A character pointer representing a numeric string used to initialize the VX_OBJ.
 *
 * Returns:
 *   An integer value indicating the validity of the prime gaps:
 *     - 1 if all tested prime gaps are correct.
 *     - 0 if any tested prime gap fails the primality test.
 */
int test_vx6_sieve(char *y)
{
    // Initialize VX_OBJ structure
    VX_OBJ *vx_obj = vx6_init(y);

    clock_t start, end;
    double cpu_time_used;

    start = clock();             // Start time
    vx6_sieve(vx_obj, 25, NULL); // Run time
    end = clock();               // End time

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // print sieve statistics
    print_line(92);
    printf("Testing vx6_sieve(%s)", y);
    print_line(92);
    printf("y: %-12s\n", vx_obj->y);
    printf("p_count: %d\n", vx_obj->p_count);
    printf("Execution time: %f seconds", cpu_time_used);
    print_line(92);

    // print primes statistics #primes, #twin, #cousin, #sexy
    print_vx_header();
    print_vx_stats(vx6, vx_obj->x5, vx_obj->x7);

    mpz_t base;
    mpz_init(base);
    mpz_set_str(base, y, 10);
    mpz_mul_ui(base, base, vx6);
    iZ_gmp(base, base, 1);

    print_line(92);
    printf("Testing first 10 primes: base value + p_gaps[i < 10]");
    print_line(92);
    gmp_printf("base value iZ(vx * y, 1): %Zd\n", base);

    // print first 10 p_gaps
    print_p_gaps(vx_obj->p_gaps, 10);
    print_line(92);

    // confirm the p_gaps are correct
    int valid = 1;
    for (int i = 0; i < 10; i++)
    {
        // increment by next gap and test primality
        mpz_add_ui(base, base, vx_obj->p_gaps[i]);

        // perform miller-rabin primality test
        if (mpz_probab_prime_p(base, 25))
            gmp_printf("%Zd is prime\n", base);
        else
        {
            valid = 0;
            gmp_printf("%Zd is not prime\n", base);
        }
    }

    if (valid)
        printf("p_gaps seems correct\n\n");
    else
        printf("Some p_gaps are incorrect\n\n");

    vx6_free(vx_obj);
    mpz_clear(base);

    return valid;
}

/**
 * test_write_read_vx6_file - Tests the process of writing and reading a vx6 object to/from a file.
 *
 * This function performs the following operations:
 *   - Initializes a VX_OBJ structure using the provided string 'y'.
 *   - Invokes the vx6_sieve function to populate the object.
 *   - Writes the VX_OBJ to a file specified by 'filename'.
 *   - Reads the VX_OBJ from the file into a new VX_OBJ structure.
 *   - Prints basic information from the read VX_OBJ, such as its vx identifier and y string.
 *
 * Parameters:
 *   @filename: A constant character pointer representing the file path where the vx6 object is stored.
 *   @y:        A character pointer representing a numeric string used to initialize the VX_OBJ.
 *
 * Returns:
 *   An integer value to indicate success or failure:
 *     - 1 if both the write and read operations are successful.
 *     - 0 if either the write or read operation fails.
 */
int test_vx6_file_io(const char *filename, char *y)
{
    VX_OBJ *vx_obj_write = vx6_init(y);

    vx6_sieve(vx_obj_write, 25, NULL);

    // write vx6 object to a file
    vx6_write_file(vx_obj_write, filename);
    if (vx6_write_file(vx_obj_write, filename))
    {
        printf("Success: VX6 object written to file: %s\n", filename);
    }
    else
    {
        printf("Error: Could not write vx6 object to file: %s\n", filename);
        return 0;
    }

    // read vx6 object from a file
    VX_OBJ *vx_obj_read = vx6_init("0");
    if (vx6_read_file(vx_obj_read, filename))
    {
        printf("Success: VX6 object read from file: %s\n", filename);
    }
    else
    {
        printf("Error: Could not read vx6 object from file: %s\n", filename);
        return 0;
    }

    // print sieve statistics
    print_line(92);
    printf("VX6 Info");
    print_line(92);
    printf("y: %s\n", vx_obj_read->y);
    printf("p_count: %d\n", vx_obj_read->p_count);
    fflush(stdout);

    // free vx_obj objects
    vx6_free(vx_obj_write);
    vx6_free(vx_obj_read);

    return 1;
}

// Analyze and print primes parity in given vx vectors in iZm5 and iZm7.
/**
 * @brief Print the header for vx statistics.
 */
void print_vx_header(void)
{
    print_line(92);
    printf("| %-12s", "Range");          // range of natural numbers
    printf("| %-8s|%-8s", "iZ-", "iZ+"); // how many primes in iZ- and iZ+
    printf("| %-12s", "#(Primes)");      // total #primes
    printf("| %-12s", "#(Twins)");       // total #twin primes
    printf("| %-12s", "#(Cousins)");     // total #cousin primes
    printf("| %-12s", "#(Sexy)");        // total #sexy primes
    print_line(92);
}

/**
 * @brief Analyze and print primes parity in given vx vectors in iZm5 and iZm7.
 *
 * @param vx The vx size, covering 6 * vx natural numbers.
 * @param x5 A BITMAP pointer representing the primes in iZm5.
 * @param x7 A BITMAP pointer representing the primes in iZm7.
 */
void print_vx_stats(size_t vx, BITMAP *x5, BITMAP *x7)
{
    // Initialize counters
    int primes_count = 0;
    int iZm5 = 0; // how many primes in iZm5
    int iZm7 = 0; // how many primes in iZm7
    int twin_count = 0;
    int cousins_count = 0;
    int sexy_count = 0;

    // Iterate over [1:vx] in both bitmaps x5 (iZ-) and x7 (iZ+)
    for (uint64_t x = 1; x <= vx; x++)
    {
        // Check if x is prime in iZ-
        if (bitmap_get_bit(x5, x))
        {
            primes_count++;
            iZm5++;
        }
        // Check if x is prime in iZ+
        if (bitmap_get_bit(x7, x))
        {
            primes_count++;
            iZm7++;
        }

        // Twin primes: check iZm5 & iZm7 at the same x
        if (bitmap_get_bit(x5, x) && bitmap_get_bit(x7, x))
            twin_count++;

        // Cousin primes: (p in x5 at x) and (p in x7 at x-1), etc.
        if (bitmap_get_bit(x5, x) && bitmap_get_bit(x7, x - 1))
            cousins_count++;

        // Sexy primes can occur in iZm5 at [x,x-1] or iZm7 at [x,x-1]
        if (bitmap_get_bit(x5, x) && bitmap_get_bit(x5, x - 1))
            sexy_count++;

        if (bitmap_get_bit(x7, x) && bitmap_get_bit(x7, x - 1))
            sexy_count++;
    }

    // Print results as a formatted row
    printf("| %-12zu", 6 * vx); // range of natural numbers
    printf("| %-8d|%-8d", iZm5, iZm7);
    printf("| %-12d", primes_count);
    printf("| %-12d", twin_count);
    printf("| %-12d", cousins_count);
    printf("| %-12d\n", sexy_count);
}
