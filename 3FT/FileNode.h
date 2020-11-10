/*--------------------------------------------------------------------*/
/* FileNode.h                                                         */
/* Author: Eesha Agarwal                                              */
/*--------------------------------------------------------------------*/

#ifndef FILENODE_INCLUDED
#define FILENODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "FTNode.h"

/*
   a FileNode is an object that contains a path payload and references to
   the FileNode's parent (if it exists) and children (if they exist).
*/

/*
   Given a parent Node, a directory string dir, contents, and length of contents,
   returns a new Node structure or NULL if any allocation error occurs in
   creating the node or its fields.

   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory string parameter,
   separated by a slash. It is also initialized with its parent link
   as the parent parameter value (but the parent itself is not changed
   to link to the new Node.  The children links are initialized but
   do not point to any children.
*/

FileNode FileNode_create(const char* dir, DTNode parent,
                         void *contents, size_t length);

/*
  Destroys the FileNode n.
*/
void FileNode_destroy(FileNode n);

/*
   Returns Node n's path.
*/
const char* FileNode_getPath(FileNode n);

/*
  Compares node1 and node2 based on their paths.
  Returns <0, 0, or >0 if node1 is less than,
  equal to, or greater than node2, respectively.
*/
int FileNode_compare(FileNode node1, FileNode node2);

/* Always returns 0 since FileNodes can only be leafs with
   no children. */
size_t FileNode_getNumChildren(FileNode n);
/*
   Returns the parent Node of n, if it exists, otherwise returns NULL
*/
DTNode FileNode_getParent(FileNode n);

void* FileNode_getContents(FileNode n);

size_t FileNode_getLength(FileNode n);

void* FileNode_replaceContents(FileNode n, void *newContents,
                               size_t newLength);

void FileNode_setParent(FileNode n, DTNode parent);
/*
  Returns a string representation n, or NULL if there is an allocation
  error.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char* FileNode_toString(FileNode n);

#endif
