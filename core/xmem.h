#ifndef XMEM_H
#define XMEM_H

#ifdef __cplusplus
extern "C" {
#endif

// managed memory array
// (XMEM_SIZE) must be defined in the project options in a #define statement
volatile unsigned char memory[XMEM_SIZE];

// allocate block or change size of already allocated one
// when calling for initial allocation the variable must be previously initialised with NULL
// the new memory block is cleared
// if the block is being extended, the extension area is cleared
// will update the supplied variable with the pointer, or NULL in case of unsuccessful allocation
void xalloc(unsigned char **v, unsigned long sz);

// free allocated block
// will do nothing if the block is not allocated
// will update the variable with NULL
void xfree(unsigned char **v);

// return the actual size of an allocated block
unsigned long xblksize(unsigned char *v);

// reinitialise the entire memory
// will return the size of the memory
unsigned long xmeminit(void);

// defragment the memory by combining unused blocks
void xdefrag(void);

// return the size of the largest continuous currently available block
unsigned long xavail(void);

// return the total size of the currently available memory (could be fragmented in many separate blocks)
unsigned long xtotal(void);

#ifdef __cplusplus
}
#endif

#endif // XMEM_H
