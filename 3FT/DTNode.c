/*--------------------------------------------------------------------*/
/* DTNode.c                                                           */
/* Author: Eesha Agarwal                                              */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "DTNode.h"
#include "FileNode.h"

/* A directory node structure represents a directory in the directory tree. */
struct DTNode {
   /* the full path of this directory */
   char* path;

   /* the parent directory of this directory
      NULL for the root of the directory tree */
   DTNode parent;

   /* the subdirectories of this directory
      stored in sorted order by pathname */
   DynArray_T DTChildren;

   /* the files in this directory stored in
      sorted order by pathname. */
   DynArray_T fileChildren;
};

/* Returns a path with contents n->path/dir
   or NULL if there is an allocation error.

   Allocates memory for the returned string,
   which is then owened by the caller. */
static char* DTNode_buildPath(DTNode n, const char* dir) {
   char* path;

   assert(dir != NULL);

   if(n == NULL)
      path = malloc(strlen(dir)+1);
   else
      path = malloc(strlen(n->path) + 1 + strlen(dir) + 1);

   if(path == NULL)
      return NULL;
   *path = '\0';

   if(n != NULL) {
      strcpy(path, n->path);
      strcat(path, "/");
   }
   strcat(path, dir);

   return path;
}

/* DTNode.h contains specification. */
DTNode DTNode_create(const char* dir, DTNode parent){

   DTNode new;

   assert(dir != NULL);

   new = malloc(sizeof(struct DTNode));

   /* In case there is insufficient memory for the new DTNode. */
   if(new == NULL) {
      return NULL;
   }

   new->path = DTNode_buildPath(parent, dir);

   /* In case there is insufficient memory for the new DTNode's path. */
   if(new->path == NULL) {
      free(new);
      return NULL;
   }

   new->parent = parent;

   /* Allocating memory to hold the directories which are children
      to a given DTNode. */
   new->DTChildren = DynArray_new(0);
   if(new->DTChildren == NULL) {
      free(new->path);
      free(new);
      return NULL;
   }

   /* Allocating memory to hold the files which are children
      to a given DTNode. */
   new->fileChildren = DynArray_new(0);
   if(new->fileChildren == NULL) {
      free(new->path);
      free(new->DTChildren);
      free(new);
      return NULL;
   }

   return new;
}

/* DTNode.h contains specification. */
size_t DTNode_destroy(DTNode n) {
   size_t i;
   size_t count = 0;
   DTNode dChild;
   FileNode fChild;

   assert(n != NULL);

   /* Recursively removing directory children. */
   for(i = 0; i < DynArray_getLength(n->DTChildren); i++)
   {
      dChild = DynArray_get(n->DTChildren, i);
      count += DTNode_destroy(dChild);
   }

   /* Removing all file children. */
   for(i = 0; i < DynArray_getLength(n->fileChildren); i++)
   {
      fChild = DynArray_get(n->fileChildren, i);
      FileNode_destroy(fChild);
      count++;
   }

   DynArray_free(n->DTChildren);
   DynArray_free(n->fileChildren);

   free(n->path);
   free(n);
   count++;

   return count;
}

/* DTNode.h contains specification. */
int DTNode_compare(DTNode node1, DTNode node2) {
   assert(node1 != NULL);
   assert(node2 != NULL);
   return strcmp(node1->path, node2->path);
}

/* DTNode.h contains specification. */
const char* DTNode_getPath(DTNode n) {
   assert(n != NULL);
   return n->path;
}

/* DTNode.h contains specification. */
size_t DTNode_getNumDTChildren(DTNode n) {
   assert(n != NULL);
   return DynArray_getLength(n->DTChildren);
}

size_t DTNode_getNumFileChildren(DTNode n) {
   assert(n != NULL);
   return DynArray_getLength(n->fileChildren);
}

/* DTNode.h contains specification. */
int DTNode_hasChild(DTNode n, const char* path, size_t* childID) {
   size_t index;
   int result;
   DTNode checker;
   FileNode fileChecker;

   assert(n != NULL);
   assert(path != NULL);

   /* Checking if there is a directory node child with childID. */
   checker = DTNode_create(path, NULL);
   if(checker == NULL) {
      return -1;
   }
   result = DynArray_bsearch(n->DTChildren, checker, &index,
                             (int (*)(const void*, const void*)) DTNode_compare);
   (void) DTNode_destroy(checker);

   /* Checking if there is a file node child with childID. */
   if (result != 1) {
      fileChecker = FileNode_create(path, NULL, NULL, 0);
      if(fileChecker == NULL) {
         return -1;
      }

      result = DynArray_bsearch(n->fileChildren, fileChecker, &index,
                                (int (*)(const void*, const void*)) FileNode_compare);
      (void) FileNode_destroy(fileChecker);
   }

   if(childID != NULL)
      *childID = index;
   return result;
}

/* DTNode.h contains specification. */
DTNode DTNode_getChild(DTNode n, size_t childID, boolean type) {
   assert(n != NULL);

   /* If child to be retrieved is a directory. */
   if (!type) {
      if (DynArray_getLength(n->DTChildren) > childID) {
         /* Returning DTNode child if found. */
         return DynArray_get(n->DTChildren, childID);
      } else {
         return NULL;
      }
   }

   /* If child to be retrieved is a file. */
   if (DynArray_getLength(n->fileChildren) > childID){
      /* Returning FileNode child if found. */
      return (DTNode) DynArray_get(n->fileChildren, childID);
   } else {
      return NULL;
   }
}

/* DTNode.h contains specification. */
DTNode DTNode_getParent(DTNode n) {
   assert(n != NULL);
   return n->parent;
}

/* DTNode.h contains specification. */
int DTNode_linkChildDirectory(DTNode parent, DTNode child) {
   size_t i;
   char* rest;

   assert(parent != NULL);
   assert(child != NULL);

   /* If child is already in the tree. */
   if(DTNode_hasChild(parent, child->path, NULL))
      return ALREADY_IN_TREE;
   i = strlen(parent->path);

   /* In case child's path is not parent's path + / + directory. */
   if(strncmp(child->path, parent->path, i))
      return PARENT_CHILD_ERROR;
   rest = child->path + i;
   if(strlen(child->path) >= i && rest[0] != '/')
      return PARENT_CHILD_ERROR;
   rest++;
   if(strstr(rest, "/") != NULL)
      return PARENT_CHILD_ERROR;

   child->parent = parent;

   /* In case DTNode child is already present in DTNode parent's children. */
   if(DynArray_bsearch(parent->DTChildren, child, &i,
                       (int (*)(const void*, const void*)) DTNode_compare) == 1)
      return ALREADY_IN_TREE;

   if(DynArray_addAt(parent->DTChildren, i, child) == TRUE)
      return SUCCESS;
   else
      return PARENT_CHILD_ERROR;
}

/* DTNode.h contains specification. */
int DTNode_linkChildFile(DTNode parent, FileNode child) {
   size_t i;
   char* rest;

   assert(parent != NULL);
   assert(child != NULL);

   /* If child is already in the tree. */
   if(DTNode_hasChild(parent, FileNode_getPath(child), NULL))
      return ALREADY_IN_TREE;
   i = strlen(parent->path);

   /* In case child's path is not parent's path + / + directory. */
   if(strncmp(FileNode_getPath(child), parent->path, i))
      return PARENT_CHILD_ERROR;
   rest = FileNode_getPath(child)+ i;
   if(strlen(FileNode_getPath(child)) >= i && rest[0] != '/')
      return PARENT_CHILD_ERROR;
   rest++;
   if(strstr(rest, "/") != NULL)
      return PARENT_CHILD_ERROR;

   (void) FileNode_setParent(child, parent);

   /* In case FileNode child is already present in DTNode parent's children. */
   if(DynArray_bsearch(parent->fileChildren, child, &i,
                       (int (*)(const void*, const void*)) FileNode_compare) == 1)
      return ALREADY_IN_TREE;

   if(DynArray_addAt(parent->fileChildren, i, child) == TRUE)
      return SUCCESS;
   else
      return PARENT_CHILD_ERROR;
}

/* DTNode.h contains specification. */
int  DTNode_unlinkChildDirectory(DTNode parent, DTNode child) {
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   if(DynArray_bsearch(parent->DTChildren, child, &i,
                       (int (*)(const void*, const void*)) DTNode_compare) == 0)
      return PARENT_CHILD_ERROR;

   (void) DynArray_removeAt(parent->DTChildren, i);
   return SUCCESS;
}

/* DTNode.h contains specification. */
int  DTNode_unlinkChildFile(DTNode parent, FileNode child) {
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   if(DynArray_bsearch(parent->fileChildren, child, &i,
                       (int (*)(const void*, const void*)) FileNode_compare) == 0)
      return PARENT_CHILD_ERROR;

   (void) DynArray_removeAt(parent->fileChildren, i);
   return SUCCESS;
}

/* DTNode.h contains specification. */
int DTNode_addChildDir(DTNode parent, const char* dir) {
   DTNode new;
   int result;

   assert(parent != NULL);
   assert(dir != NULL);

   new = DTNode_create(dir, parent);
   if(new == NULL)
      return PARENT_CHILD_ERROR;

   result = DTNode_linkChildDirectory(parent, new);
   if(result != SUCCESS)
      (void) DTNode_destroy(new);

   return result;
}

int DTNode_addChildFile(DTNode parent, const char* file,
                        void *contents, size_t length) {
   FileNode new;
   int result;

   assert(parent != NULL);
   assert(file != NULL);

   new = FileNode_create(file, parent, contents, length);
   if(new == NULL)
      return PARENT_CHILD_ERROR;

   result = DTNode_linkChildFile(parent, new);
   if(result != SUCCESS)
      (void) FileNode_destroy(new);

   return result;
}

/* DTNode.h contains specification. */
char* DTNode_toString(DTNode n) {
   char* copyPath;

   assert(n != NULL);

   copyPath = malloc(strlen(n->path)+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, n->path);
}
