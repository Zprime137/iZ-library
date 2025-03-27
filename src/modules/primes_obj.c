// primes_obj.c
#include <primes_obj.h>

/**
 * @brief Initialize a primes object by allocating memory and setting initial values.
 *
 * @param initial_estimate Initial size estimate for the primes array.
 * @return PRIMES_OBJ* Pointer to the initialized PRIMES_OBJ, or NULL on failure.
 */
PRIMES_OBJ *primes_obj_init(int initial_estimate)
{
    if (initial_estimate <= 0)
    {
        log_error("Initial estimate must be positive.");
        return NULL;
    }

    // Allocate memory for the PRIMES_OBJ structure
    PRIMES_OBJ *primes_obj = malloc(sizeof(PRIMES_OBJ));
    if (primes_obj == NULL)
    {
        log_error("Memory allocation failed for PRIMES_OBJ.");
        return NULL;
    }

    // Initialize the prime count and capacity
    primes_obj->p_count = 0;
    primes_obj->capacity = initial_estimate;

    // Allocate memory for the primes array
    primes_obj->p_array = malloc(initial_estimate * sizeof(uint64_t));
    if (primes_obj->p_array == NULL)
    {
        log_error("Memory allocation failed for primes array.");
        free(primes_obj); // Free the previously allocated PRIMES_OBJ
        return NULL;
    }

    // Initialize the SHA-256 hash to zero
    memset(primes_obj->sha256, 0, SHA256_DIGEST_LENGTH);

    return primes_obj; // Success
}

/**
 * @brief Appends a prime number p to the primes_obj->primes array,
 * and increments the count of primes primes_obj->p_count.
 *
 * @param primes_obj Pointer to the PRIMES_OBJ structure.
 * @param p The prime number to be appended.
 */
void primes_obj_append(PRIMES_OBJ *primes_obj, uint64_t p)
{
    primes_obj->p_array[primes_obj->p_count++] = p; // add p to p_array, increment p_count
}

/**
 * @brief Resize the primes array to fit the current number of primes.
 *
 * @param primes_obj Pointer to an initialized PRIMES_OBJ.
 * @return int 0 on success, -1 on failure.
 */
int primes_obj_resize_to_p_count(PRIMES_OBJ *primes_obj)
{
    if (primes_obj == NULL)
    {
        log_error("Null pointer passed to primes_obj_resize.");
        return -1;
    }

    // Resize the primes array to fit exactly p_count
    uint64_t *temp = realloc(primes_obj->p_array, primes_obj->p_count * sizeof(uint64_t));
    if (temp == NULL)
    {
        log_error("Memory reallocation failed for primes array.");
        return -1;
    }

    primes_obj->p_array = temp;
    primes_obj->capacity = primes_obj->p_count;

    return 0; // Success
}

/**
 * @brief Free the resources allocated within a primes object, including the object itself.
 *
 * @param primes_obj Pointer to the PRIMES_OBJ to be freed.
 */
void primes_obj_free(PRIMES_OBJ *primes_obj)
{
    if (primes_obj == NULL)
        return;

    // Free the primes array if it has been allocated
    if (primes_obj->p_array != NULL)
    {
        free(primes_obj->p_array);
        primes_obj->p_array = NULL; // Prevent dangling pointer
    }

    // Clear the SHA-256 hash
    memset(primes_obj->sha256, 0, SHA256_DIGEST_LENGTH);

    // Free the PRIMES_OBJ structure itself
    free(primes_obj);
    primes_obj = NULL; // Prevent dangling pointer
}

// Compute the SHA-256 hash for the primes array
int primes_obj_compute_hash(PRIMES_OBJ *primes_obj)
{
    if (primes_obj == NULL || primes_obj->p_array == NULL || primes_obj->p_count == 0)
    {
        log_error("Invalid PRIMES_OBJ in primes_obj_compute_hash.");
        return -1;
    }

    // Compute SHA-256 hash
    hash_int_array(primes_obj->p_array, primes_obj->p_count * sizeof(uint64_t), primes_obj->sha256);
    return 0;
}

// Validate the SHA-256 hash
int primes_obj_validate_hash(PRIMES_OBJ *primes_obj)
{
    unsigned char computed_hash[SHA256_DIGEST_LENGTH];
    if (primes_obj == NULL || primes_obj->p_array == NULL || primes_obj->p_count == 0)
    {
        log_error("Invalid PRIMES_OBJ in primes_obj_validate_hash.");
        return -1;
    }

    SHA256((unsigned char *)primes_obj->p_array, primes_obj->p_count * sizeof(uint64_t), computed_hash);
    if (memcmp(computed_hash, primes_obj->sha256, SHA256_DIGEST_LENGTH) != 0)
    {
        log_error("SHA-256 hash mismatch.");
        return -1;
    }
    return 0;
}

// Write the PRIMES_OBJ to a binary file
int primes_obj_write_file(const char *file_path, PRIMES_OBJ *primes_obj)
{
    primes_obj_compute_hash(primes_obj);

    FILE *file = fopen(file_path, "wb");
    if (!file)
        return -1;

    fwrite(&primes_obj->p_count, sizeof(int), 1, file);
    fwrite(primes_obj->p_array, sizeof(uint64_t), primes_obj->p_count, file);
    fwrite(primes_obj->sha256, 1, SHA256_DIGEST_LENGTH, file);
    fclose(file);
    return 0;
}

// Read the PRIMES_OBJ from a binary file
PRIMES_OBJ *primes_obj_read_file(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
        return NULL;

    int p_count;
    fread(&p_count, sizeof(int), 1, file);

    PRIMES_OBJ *primes_obj = primes_obj_init(p_count);
    if (!primes_obj)
        return NULL;

    primes_obj->p_count = p_count;
    fread(primes_obj->p_array, sizeof(uint64_t), p_count, file);
    fread(primes_obj->sha256, 1, SHA256_DIGEST_LENGTH, file);
    fclose(file);

    if (primes_obj_validate_hash(primes_obj) != 0)
    {
        primes_obj_free(primes_obj);
        return NULL;
    }

    return primes_obj;
}
