#ifndef IZ_VX6_H
#define IZ_VX6_H

#include <utils.h>

/**
 * @brief Structure representing a horizontal segment in iZm.
 */
typedef struct
{
    int vx;                                     ///< The horizontal vector size.
    char *y;                                    ///< Pointer to the y string.
    int p_count;                                ///< Number of elements in the p_gaps array.
    BITMAP *x5;                                 ///< Pointer to the x5 BITMAP.
    BITMAP *x7;                                 ///< Pointer to the x7 BITMAP.
    uint16_t *p_gaps;                           ///< Pointer to the p_gaps array.
    unsigned char sha256[SHA256_DIGEST_LENGTH]; ///< SHA-256 hash of the p_gaps data for validation.
} VX_OBJ;

#define VX6_EXT ".vx6"
#define P_GAPS_SIZE sizeof(uint16_t)

/**
 * @brief Prints the header for VX statistics.
 */
void print_vx_header(void);

/**
 * @brief Prints VX statistics.
 *
 * @param vx The horizontal vector size.
 * @param x5 Pointer to the BITMAP representing x5.
 * @param x7 Pointer to the BITMAP representing x7.
 */
void print_vx_stats(size_t vx, BITMAP *x5, BITMAP *x7);

/**
 * @brief Initializes a VX_OBJ structure.
 *
 * Allocates and initializes a new VX_OBJ structure using the provided y string.
 *
 * @param y A C-string used for initialization.
 * @return Pointer to the newly created VX_OBJ, or NULL if allocation fails.
 */
VX_OBJ *vx6_init(const char *y);

/**
 * @brief Frees a VX_OBJ structure.
 *
 * Frees all memory associated with the VX_OBJ.
 *
 * @param vx_obj Pointer to the VX_OBJ to free.
 */
void vx6_free(VX_OBJ *vx_obj);

/**
 * @brief Writes VX data to a file.
 *
 * Writes the contents of the VX_OBJ structure to the specified file.
 *
 * @param vx_obj Pointer to the VX_OBJ containing the data.
 * @param filename C-string representing the file name.
 * @return 0 on success, or a non-zero error code on failure.
 */
int vx6_write_file(VX_OBJ *vx_obj, const char *filename);

/**
 * @brief Reads VX data from a file.
 *
 * Populates a VX_OBJ structure with data read from the specified file.
 *
 * @param vx_obj Pointer to the VX_OBJ to populate.
 * @param filename C-string representing the file name.
 * @return 0 on success, or a non-zero error code on failure.
 */
int vx6_read_file(VX_OBJ *vx_obj, const char *filename);

/**
 * @brief Performs the sieve operation on a VX_OBJ.
 *
 * Applies a sieving algorithm to the VX_OBJ with the given number of test rounds and
 * writes the output to the specified file.
 *
 * @param vx_obj Pointer to the VX_OBJ to process.
 * @param p_test_rounds Number of test rounds to perform for prime testing.
 * @param filename C-string representing the file name to write the output.
 */
void vx6_sieve(VX_OBJ *vx_obj, int p_test_rounds, const char *filename);

/**
 * @brief Tests the VX6 sieve algorithm.
 *
 * @param y C-string used for test input.
 * @return 0 on success, or a non-zero error code on failure.
 */
int test_vx6_sieve(char *y);

/**
 * @brief Tests writing and reading of a VX6 file.
 *
 * Validates that VX6 file I/O works by writing to and reading from the file.
 *
 * @param dir C-string representing the directory where the file is stored.
 * @param y C-string used for test input.
 * @return 0 on success, or a non-zero error code on failure.
 */
int test_vx6_file_io(const char *dir, char *y);

#endif // IZ_VX6_H
