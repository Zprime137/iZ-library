#ifndef BITMAP_H
#define BITMAP_H

// #include <stddef.h> // For size_t
// #include <stdint.h> // For uint64_t
#include <utils.h>

#define BITMAP_EXT "bitmap"

/**
 * @brief Structure representing a dynamic array of bits.
 */
typedef struct
{
    size_t size;                                ///< Number of bits in the array.
    unsigned char *data;                        ///< Pointer to unsigned char.
    unsigned char sha256[SHA256_DIGEST_LENGTH]; ///< SHA-256 hash of the data for validation.
} BITMAP;

/**
 * @brief Creates a new bitmap with the specified size.
 *
 * @param size The number of bits in the bitmap.
 * @return A pointer to the newly created BITMAP structure.
 */
BITMAP *bitmap_create(size_t size);

/**
 * @brief Frees the memory allocated for the bitmap.
 *
 * @param bitmap A pointer to the BITMAP structure to be freed.
 */
void bitmap_free(BITMAP *bitmap);

/**
 * @brief Sets all bits in the bitmap to 1.
 *
 * @param bitmap A pointer to the BITMAP structure.
 */
void bitmap_set_all(BITMAP *bitmap);

/**
 * @brief Clears all bits in the bitmap (sets them to 0).
 *
 * @param bitmap A pointer to the BITMAP structure.
 */
void bitmap_clear_all(BITMAP *bitmap);

/**
 * @brief Sets a specific bit in the bitmap to 1.
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @param idx The index of the bit to be set.
 */
void bitmap_set_bit(BITMAP *bitmap, size_t idx);

/**
 * @brief Gets the value of a specific bit in the bitmap.
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @param idx The index of the bit to be retrieved.
 * @return The value of the bit (0 or 1).
 */
int bitmap_get_bit(BITMAP *bitmap, size_t idx);

/**
 * @brief Clears a specific bit in the bitmap (sets it to 0).
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @param idx The index of the bit to be cleared.
 */
void bitmap_clear_bit(BITMAP *bitmap, size_t idx);

/**
 * @brief Clears bits in the bitmap based on a modulus operation.
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @param p The modulus value.
 * @param start_idx The starting index for the operation.
 * @param limit The limit on the number of bits to be cleared.
 */
void bitmap_clear_mod_p(BITMAP *bitmap, uint64_t p, size_t start_idx, size_t limit);

/**
 * @brief Creates a clone of the given bitmap.
 *
 * @param bitmap A pointer to the BITMAP structure to be cloned.
 * @return A pointer to the newly created clone of the BITMAP structure.
 */
BITMAP *bitmap_clone(BITMAP *bitmap);

/**
 * @brief Copies a segment of bits from the source bitmap to the destination bitmap.
 *
 * @param dest A pointer to the destination BITMAP structure.
 * @param dest_idx The starting index in the destination bitmap.
 * @param src A pointer to the source BITMAP structure.
 * @param src_idx The starting index in the source bitmap.
 * @param length The number of bits to be copied.
 */
void bitmap_copy(BITMAP *dest, size_t dest_idx, BITMAP *src, size_t src_idx, size_t length);

/**
 * @brief Duplicates a segment of bits within the bitmap.
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @param start_idx The starting index of the segment to be duplicated.
 * @param vx_size The size of the segment to be duplicated.
 * @param y The number of times the segment should be duplicated.
 */
void bitmap_duplicate_segment(BITMAP *bitmap, size_t start_idx, size_t vx_size, size_t y);

/**
 * @brief Initializes the bitmap from a string representation.
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @param str The string representation of the bitmap.
 */
void bitmap_from_string(BITMAP *bitmap, const char *str);

/**
 * @brief Converts the bitmap to a string representation.
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @return A string representation of the bitmap.
 */
char *bitmap_to_string(BITMAP *bitmap);

/**
 * @brief Computes the SHA-256 hash of the bitmap data.
 *
 * @param bitmap A pointer to the BITMAP structure.
 */
void bitmap_compute_hash(BITMAP *bitmap);

/**
 * @brief Validates the SHA-256 hash of the bitmap data.
 *
 * @param bitmap A pointer to the BITMAP structure.
 * @return 1 if the hash is valid, 0 otherwise.
 */
int bitmap_validate_hash(BITMAP *bitmap);

/**
 * @brief Writes the bitmap to a file.
 *
 * @param file_name The name of the file.
 * @param bitmap A pointer to the BITMAP structure.
 * @return 0 on success, -1 on failure.
 */
int bitmap_write_file(const char *file_name, BITMAP *bitmap);

/**
 * @brief Reads a bitmap from a file.
 *
 * @param file_name The name of the file.
 * @return A pointer to the newly read BITMAP structure.
 */
BITMAP *bitmap_read_file(const char *file_name);

#endif // BITMAP_H
