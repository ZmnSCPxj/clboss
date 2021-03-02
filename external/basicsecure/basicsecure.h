/* This library was written by ZmnSCPxj and is placed in the public domain.  */
#ifndef BASICSECURE_H
#define BASICSECURE_H

#ifdef __cplusplus
extern "C" {
#endif

/** basicsecure_eq
 *
 * @brief check if two memory areas are equal.
 *
 * @param p1 - pointer to first memory area.
 * @param p2 - pointer to second memory area.
 * @param size - size of both memory areas.
 *
 * @return 0 if any byte is not equal, 1 if all bytes equal
 *
 * @desc This is a constant-time compare operation.
 */
int basicsecure_eq(void const* p1, void const* p2, unsigned long size);

/** basicsecure_clear
 *
 * @brief zero out the specified memory area.
 *
 * @param p - pointer to memory area.
 * @param size - size of the memory area.
 *
 * @desc This is assured to not be optimized away and will
 * definitely clear the memory area.
 */
void basicsecure_clear(void* p, unsigned long size);

/** basicsecure_rand
 *
 * @brief Fill the memory area with random bytes.
 *
 * @param p - pointer to memory area.
 * @param size - size of the memory area.
 * @param comment - text string to print to stderr if
 * random number generation fails.
 *
 * @desc This will either provide you with good cryptographic
 * randomness, or abort the program if a way to acquire this
 * randomness is not possible.
 */
void basicsecure_rand(void* p, unsigned long size, char const* comment);

/** BASICSECURE_RAND
 *
 * @brief Fill the memory area with random bytes.
 *
 * @param p - pointer to memory area.
 * @param size - size of the memory area.
 *
 * @desc This will either provide you with good cryptographic
 * randomness, or abort the program if a way to acquire this
 * randomness is not possible.
 */
#define BASICSECURE_FILE_LINE_STAGE2(f, l) f ":" #l
#define BASICSECURE_FILE_LINE(f, l) BASICSECURE_FILE_LINE_STAGE2(f, l)
#define BASICSECURE_RAND(p, size) \
	basicsecure_rand((p), (size), BASICSECURE_FILE_LINE(__FILE__, __LINE__) ": " \
			"basicsecure_rand could not get random numbers, maybe unable " \
			"to access /dev/urandom ?")

#ifdef __cplusplus
}
#endif

#endif /* !defined(BASICSECURE_H) */
