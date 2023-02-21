#include <string.h>
#include "xmem.h"

// IMPORTANT
// (XMEM_SIZE) must be defined defined outside in the project's pre-processor definitions
#ifndef XMEM_SIZE
    #define XMEM_SIZE 16384 // default RAM size (in bytes) available to Rittle
#endif

// memory block header structure
typedef struct {
    unsigned char **cv;		// C variable (NULL if the block is free)
	unsigned long len;		// length of the allocated block
	unsigned long need;		// actually needed size for the block (smaller or equal to .len)
	unsigned char *data;	// pointer to the allocated data block
} xhdr_t;

// block headers
static unsigned char *dcur=(unsigned char *)memory;
static xhdr_t *hcur=(xhdr_t *)(memory+XMEM_SIZE);


unsigned long xmeminit(void) {
    memset((unsigned char *)memory,0,XMEM_SIZE);
	dcur=(unsigned char *)memory;
	hcur=(xhdr_t *)(memory+XMEM_SIZE);
    return XMEM_SIZE;
}


// internal function
// find header based on data pointer; return NULL if the header can not be found
xhdr_t *findxhdr(unsigned char *da) {
	xhdr_t *h=(xhdr_t *)(memory+XMEM_SIZE);
	while(--h>=hcur) {
		if(da==h->data) return h;
	}
	return NULL;	// unknown header (memory leak?)
}


// internal function
// try to find a free data block with suitable size among the already allocated headers
// return NULL if no such block exist
xhdr_t *findxavl(unsigned long sz) {
	xhdr_t *hbest=NULL;
	xhdr_t *h=(xhdr_t *)(memory+XMEM_SIZE);
	while(--h>=hcur) {		// search among the currently allocated but unused blocks
		if(h->cv==NULL && h->len>=sz) {
			if(hbest) {
				if((sz-(h->len))<(sz-(hbest->len))) hbest=h;	// try to find the smallest free block with size bigger than (sz)
				if(sz==(hbest->len)) break;						// exact size found
			}
			else hbest=h;
		}
	}
	return hbest;
}


unsigned long xblksize(unsigned char *v) {
	xhdr_t *h=findxhdr(v);
	return (h? h->len : 0);
}


unsigned long xavail(void) {
	xdefrag();
	unsigned long t=(unsigned char *)hcur-dcur;
	xhdr_t *h=(xhdr_t *)(memory+XMEM_SIZE);
	while(--h>=hcur) {
		if(h->cv==NULL && h->len>t) t=h->len;
	}
	return t;
}


unsigned long xtotal(void) {
	xdefrag();
	unsigned long t=0;
	xhdr_t *h=(xhdr_t *)(memory+XMEM_SIZE);
	while(--h>=hcur) {
		if(h->cv==NULL) t+=h->len; else t+=(h->len-h->need);
	}
	return (t+((unsigned char *)hcur-dcur)+1);
}


void xdefrag(void) {
    xhdr_t *h=(xhdr_t *)(memory+XMEM_SIZE);
	while(--h>=hcur) {		// combine neighbouring unused blocks
		if(h->cv==NULL) {
			char flag=0;
			unsigned char *p=h->data+h->len;
			xhdr_t *ht=(xhdr_t *)(memory+XMEM_SIZE);
			while(--ht>=hcur) {
				if(ht->data==p && ht->cv==NULL) {		// this one is suitable for combining with (h)
					h->len+=ht->len;
					memmove((unsigned char *)((xhdr_t *)(hcur+1)),(unsigned char *)hcur,((ht-hcur)*sizeof(xhdr_t)));
					hcur++;
					ht=(xhdr_t *)(memory+XMEM_SIZE);	// restart the inner loop
					flag=1;								// a change has been made
				}
			}
			if(flag) h=(xhdr_t *)(memory+XMEM_SIZE);	// restart the outer loop
		}
	}
}


void xfree(unsigned char **v) {
    if(!v || *v==NULL || *v<memory || *v>(memory+XMEM_SIZE)) return;
	xhdr_t *h=findxhdr(*v);
	if(h) {
		h->cv=NULL;
		h->need=0;
	}
	*v=NULL;
}


void xalloc(unsigned char **v, unsigned long sz) {
	if(!v || !sz) {                 // size zero is equivalent to free the memory
		xfree(v);
		return;
	}
	if(*v && (*v<memory || *v>(memory+XMEM_SIZE))) return;	// the supplied pointer is not within the memory[] array
	if(sz & 3) sz=((sz>>2)+1)<<2;	// always align data length to 4-byte boundary
    char retryf=0;
    retry:

    if(*v) {                        // re-allocation of an existing block
        xhdr_t *z=findxhdr(*v);		// find the current header
		if(z) {
			if(z->len<sz) {			// data relocation will be needed
				xhdr_t *h=findxavl(sz);
				if(!h) {			// new block will be needed for this size
					unsigned char *vt=*v;
					*v=NULL;
					xalloc(v,sz);	// allocate a new block with the needed size
					if(*v) {
						memcpy(*v,z->data,z->len);			// move the existing data to new location
						memset((*v+z->len),0,(sz-z->len));	// clear the expansion area
						z->cv=NULL;	// free up the old block
					}
					else {			// unable to allocate the needed size
						*v=vt;
						xfree(v);
					}
				}
				else {				// reusing already allocated block and relocating the data
					memcpy(h->data,z->data,z->len);
					memset((h->data+z->len),0,(sz-z->len));	// clear the expansion area
					*v=h->data;
					h->need=sz;
					h->cv=z->cv;
					z->cv=NULL;		// free up the old block
				}
			}
			else {					// nothing is needed since the current block can be expanded to the needed size
				memset((z->data+sz),0,(z->len-sz));	// clear the expansion area
				z->need=sz;
			}
		}
		else *v=NULL;				// the supplied (*v) was not currently allocated known block
    }

    else {                          // new allocation
		xhdr_t *h=findxavl(sz);
		if(!h) {					// new block will have to be allocated
			unsigned long htf, hf=-1;
			xhdr_t *ht=(xhdr_t *)(memory+XMEM_SIZE);
			while(--ht>=hcur) {		// check to see if some of the currently allocated and used blocks have enough extra space
				htf=ht->len-ht->need;
				if(htf>=sz && htf<hf) {
					h=ht;
					if(hf==sz) break; else hf=htf;	// immediately break on the first exact fit
				}
			}
			if(h && ((unsigned char *)hcur-dcur)>=sizeof(xhdr_t)) {			// reusing free part of an existing block and creating a new header for it
				hcur--;
				hcur->len=sz;
				hcur->need=sz;
				hcur->cv=v;
				hcur->data=h->data+(h->len-sz);
				h->len-=sz;					// trim the existing old block down by (sz) bytes
				memset(hcur->data,0,sz);	// clear the data area
				*v=hcur->data;
			}
			else if(((unsigned char *)hcur-dcur)>=(sz+sizeof(xhdr_t))) {	// allocating a completely new block
				hcur--;
				hcur->len=sz;
				hcur->need=sz;
				hcur->cv=v;
				hcur->data=dcur;
				dcur+=sz;
				memset(hcur->data,0,sz);	// clear the newly allocated block
				*v=hcur->data;
			}
			else {                          // can't find enough memory - try defragmenting first
                xdefrag();
                retryf=!retryf;
                if(retryf) goto retry;
                *v=NULL;					// unable to allocate block with this size
            }
		}
		else {								// reusing already allocated block
			memset(h->data,0,h->len);		// clear the block
			h->cv=v;
			h->need=sz;
			*v=h->data;
		}
    }

}
