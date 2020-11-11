/*--------------------------------------------------------------------*/
/* FileNode.c                                                        */
/* Author: Eesha Agarwal                                              */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "FileNode.h"
#include "DTNode.h"

/* A FileNode structure represents a file in the file tree. */
struct FileNode {
/* the full path of this directory */
   char* path;

/* the parent directory of this directory
   NULL for the root of the directory tree */
   DTNode parent;

/* a pointer to the contents of the file. */
   void *contents;

/* length of the contents of the file. */
   size_t length;
};

/* Returns a path with contents n->path/dir
   or NULL if there is an allocation error.

   Allocates memory for the returned string,
   which is then owened by the caller. */
static char* FileNode_buildPath(DTNode n, const char* file) {
   char* path;

   assert(file != NULL);

   if(n == NULL)
      path = malloc(strlen(file)+1);
   else
      path = malloc(strlen(DTNode_getPath(n)) + 1 + strlen(file) + 1);

   if(path == NULL)
      return NULL;
   *path = '\0';

   if(n != NULL) {
      strcpy(path, DTNode_getPath(n));
      strcat(path, "/");
   }
   strcat(path, file);

   return path;
}

/* FileNode.h contains specification. */
FileNode FileNode_create(const char* dir, DTNode parent,
                         void *contents, size_t length){

   FileNode new;

   assert(dir != NULL);

   new = malloc(sizeof(struct FileNode));
   if(new == NULL)
      return NULL;

   new->path = FileNode_buildPath(parent, dir);

   if(new->path == NULL) {
      free(new);
      return NULL;
   }

   new->parent = parent;
   new->contents = contents;
   new->length = length;

   return new;
}

/* FileNode.h contains specification. */
void FileNode_destroy(FileNode n) {
   free(n->path);
   free(n);
}

/* FileNode.h contains specification. */
int FileNode_compare(FileNode node1, FileNode node2) {
   assert(node1 != NULL);
   assert(node2 != NULL);
   return strcmp(node1->path, node2->path);
}

/* FileNode.h contains specification. */
const char* FileNode_getPath(FileNode n) {
   assert(n != NULL);
   return n->path;
}

/* FileNode.h contains specification. */
DTNode FileNode_getParent(FileNode n) {
   assert(n != NULL);
   return n->parent;
}

/* FileNode.h contains specification. */
void* FileNode_getContents(FileNode n) {
   assert(n != NULL);
   return n->contents;
}

/* FileNode.h contains specification. */
size_t FileNode_getLength(FileNode n) {
   assert(n != NULL);
   return n->length;
}

/* FileNode.h contains specification. */
void* FileNode_replaceContents(FileNode n, void *newContents,
                               size_t newLength) {
   void* oldContents;
   assert(n != NULL);
   oldContents = n->contents;
   n->contents = newContents;
   n->length = newLength;
   return oldContents;
}

/* FileNode.h contains specification. */
void FileNode_setParent(FileNode n, DTNode parent) {
   assert(n != NULL);
   assert(parent != NULL);
   n->parent = parent;
}

/* FileNode.h contains specification. */
char* FileNode_toString(FileNode n) {
   char* copyPath;

   assert(n != NULL);

   copyPath = malloc(strlen(n->path)+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, n->path);
}
