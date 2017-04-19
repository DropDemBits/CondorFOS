#include <condor.h>

#ifndef _VFS_H_
#define _VFS_H_

typedef struct FILE {
    size_t unknown;
} FILE;


//TODO: Fill methods
void open();
void close();
void read();
void write();

#endif /* _VFS_H_ */
