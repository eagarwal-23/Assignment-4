/*--------------------------------------------------------------------*/
/* FileNode.h                                                         */
/* Author: Eesha Agarwal                                              */
/*--------------------------------------------------------------------*/

#ifndef FILENODE_INCLUDED
#define FILENODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "FTNode.h"

/*--------------------------------------------------------------------*/

/* A FileNode is an object that contains a path payload and references to
   the FileNode's parent (if it exists). */

/*--------------------------------------------------------------------*/

/* Given a parent DTNode, a directory string dir, contents, and the length of
   contents, returns a new FileNode structure or NULL if any allocation error
   occurs in creating the node or its fields. */

FileNode FileNode_create(const char* dir, DTNode parent,
                         void *contents, size_t length);

/*--------------------------------------------------------------------*/

/* Destroys the FileNode n. */
void FileNode_destroy(FileNode n);

/*--------------------------------------------------------------------*/

/* Returns FileNode n's path. */
const char* FileNode_getPath(FileNode n);

/*--------------------------------------------------------------------*/

/* Compares node1 and node2 based on their paths.
  Returns <0, 0, or >0 if node1 is less than,
  equal to, or greater than node2, respectively. */
int FileNode_compare(FileNode node1, FileNode node2);

/*--------------------------------------------------------------------*/

/* Returns the parent DTNode of n. */
DTNode FileNode_getParent(FileNode n);

/*--------------------------------------------------------------------*/

/* Returns the contents of FileNode n. */
void* FileNode_getContents(FileNode n);

/*--------------------------------------------------------------------*/

/* Returns the length of the contents of FileNode n. */
size_t FileNode_getLength(FileNode n);

/*--------------------------------------------------------------------*/

/* Replaces the contents of FileNode n with newContents, and
   the length of contents with newLength. Returns the old contents
   of n. */
void* FileNode_replaceContents(FileNode n, void *newContents,
                               size_t newLength);

/*--------------------------------------------------------------------*/

/* Updates the parent of FileNode n to be the input DTNode parent. */
void FileNode_setParent(FileNode n, DTNode parent);

/*--------------------------------------------------------------------*/

/* Returns a string representation of FileNode n, or NULL in case there
   is insufficient memory. */
char* FileNode_toString(FileNode n);

/*--------------------------------------------------------------------*/

#endif
