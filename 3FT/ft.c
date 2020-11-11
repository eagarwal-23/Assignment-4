/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Eesha Agarwal                                              */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "dynarray.h"
#include "ft.h"
#include "DTNode.h"
#include "FileNode.h"
#include "FTNode.h"

/* A File Tree is an AO with 4 state variables: */

/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* a pointer to a root DTNode in the hierarchy */
static DTNode root;
/* a pointer to a root FileNode in the hierarchy */
static FileNode fileRoot;
/* a counter of the number of Nodes in the hierarchy */
static size_t count;

/* Starting at the parameter curr, traverses as far down
  the hierarchy as possible while still matching the path
  parameter.

  Returns a pointer to the farthest matching Node down that path,
  or NULL if there is no node in curr's hierarchy that matches
  a prefix of the path. piResult is used to indicate whether the
  node matched is a file or a directory. */
static DTNode FT_traversePathFrom(char* path, DTNode curr, int *piResult) {
   DTNode found;
   size_t i;

   assert(path != NULL);

   if(curr == NULL) {
      return NULL;
   }

   else if(!strcmp(path, DTNode_getPath(curr))) {
      return curr;
   }

   /* Checking if the first n characters match the path of curr, where n is
      length of the path of curr, i.e. checking if the path is currently
      the same. */
   else if(!strncmp(path, DTNode_getPath(curr), strlen(DTNode_getPath(curr)))) {
      /* Recursively traversing the directories that are children of the curr
         DTNode. */
      for(i = 0; i < DTNode_getNumDTChildren(curr); i++) {
         found = FT_traversePathFrom(path, DTNode_getChild(curr, i, FALSE),
                                     piResult);
         if(found != NULL) {
            return found;
         }
      }
      /* If found is NULL, searching for child file instead. */
      for (i = 0; i < DTNode_getNumFileChildren(curr); i++) {
         found = DTNode_getChild(curr, i, TRUE);

         /* If current path matches the path for a given child node. */
         if (strcmp(DTNode_getPath(found), path) == 0) {
            *piResult = PARENT_CHILD_ERROR;
            return found;
         }

      }
      return curr;
   }
   return NULL;
}

/* Returns the farthest node reachable from the root following a given
   path, or NULL if there is no node in the hierarchy that matches a
   prefix of the path. */
static DTNode FT_traversePath(char* path, int* piResult) {
   assert(path != NULL);
   return FT_traversePathFrom(path, root, piResult);
}

/* Given a prospective parent DTNode and child FileNode,
   adds child to parent's children list, if possible.

   If not possible, destroys the childNode and returns PARENT_CHILD_ERROR,
   otherwise, returns SUCCESS. */
static int FT_linkParentToChildFile(DTNode parent, FileNode child) {

   assert(parent != NULL);

   if(DTNode_linkChildFile(parent, child) != SUCCESS) {
      (void) FileNode_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/* Given a prospective parent DTNode and child DTNode,
   adds child to parent's children list, if possible

   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS. */
static int FT_linkParentToChildDirectory(DTNode parent, DTNode child) {

   assert(parent != NULL);

   if(DTNode_linkChildDirectory(parent, child) != SUCCESS) {
      (void) DTNode_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/* Inserts a new path into the tree rooted at parent, or, if
   parent is NULL, as the root of the data structure.

   If a Node representing path already exists, returns ALREADY_IN_TREE.

   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR.

   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR.

   Otherwise, returns SUCCESS. */
static int FT_insertRestOfPath(char* path, DTNode parent, boolean type,
                               void* contents, size_t length) {
   DTNode curr = parent;
   boolean firstNew = TRUE;
   FileNode firstFile = NULL;
   DTNode firstDir = NULL;
   DTNode newDir = NULL;
   FileNode newFile = NULL;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   char* nextToken;
   int result;
   size_t newCount = 0;

   assert(path != NULL);

   if(curr == NULL) {
      if(root != NULL) {
         return CONFLICTING_PATH;
      }
   }
   else {
      /* Comparing path of current node with input path, in case they
         are the same, node is already in the tree. */
      if(!strcmp(path, DTNode_getPath(curr))) {
         return ALREADY_IN_TREE;
      }
      restPath += (strlen(DTNode_getPath(curr)) + 1);
   }

   copyPath = malloc(strlen(restPath)+1);

   /* In case of insufficient memory. */
   if(copyPath == NULL) {
      return MEMORY_ERROR;
   }

   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");


   while(dirToken != NULL) {
      nextToken = strtok(NULL, "/");

      /* If file is being inserted. */
      if ((nextToken == NULL) && (type)) {
         newFile = FileNode_create(dirToken, curr, contents, length);
      }

      /* If directory is being inserted. */
      else {
         newDir = DTNode_create(dirToken, curr);
      }

      newCount++;
      /* If firstNew is TRUE, this is the first node
         being inserted. Else, it is not. */
      if(firstNew == TRUE) {
         firstFile = newFile;
         firstDir = newDir;
         firstNew = FALSE;
      }

      else {
         /* In case of a directory being inserted. */
         if (!type) {
            result = FT_linkParentToChildDirectory(curr, newDir);
            if(result != SUCCESS) {
               /* Destroying temporary directory newDir no longer in
                 use. */
               (void) DTNode_destroy(newDir);

               /* Destroying the path up until this directory. */
               (void) DTNode_destroy(firstDir);
               free(copyPath);
               return result;
            }
         }
         else {
            /* In case of a file being inserted. */
            result = FT_linkParentToChildFile(curr, newFile);
            if(result != SUCCESS) {
               /* Destroying temporary file newFile no longer in
                  use. */
               (void) FileNode_destroy(newFile);

               /* Destroying the path up until this file. */
               (void) DTNode_destroy(firstDir);
               free(copyPath);
               return result;
            }
         }
      }

      /* If end of path has been reached. */
      if (nextToken == NULL) {
         /* In case directory was being inserted and
            insufficient memory was available. */
         if((!type) && (newDir == NULL)) {
            (void) DTNode_destroy(firstDir);
            free(copyPath);
            return MEMORY_ERROR;
         }

         /* In case file was being inserted and
            insufficient memory was available. */
         if((type) && (newFile == NULL)) {
            (void) DTNode_destroy(firstDir);
            free(copyPath);
            return MEMORY_ERROR;
         }
      }
      curr = newDir;
      dirToken = nextToken;
   }

   free(copyPath);

   /* Parent will only be NULL if node is being inserted at the root. */
   if(parent == NULL) {
      root = firstDir;
      fileRoot = firstFile;
      count = newCount;
      return SUCCESS;
   }

   else {
      /* In case file is being inserted. */
      if (firstDir == NULL) {
         result = FT_linkParentToChildFile(parent, firstFile);
      }
      /* In case directory is being inserted. */
      else {
         result = FT_linkParentToChildDirectory(parent, firstDir);
      }

      /* Incrementing number of nodes in tree if successful insertion. */
      if(result == SUCCESS) {
         count += newCount;
      }
      else {
         if (firstDir == NULL) {
            (void) FileNode_destroy(firstFile);
         } else {
            (void) DTNode_destroy(firstDir);
         }

      }

      return result;

   }
}

/* ft.h contains specification. */
int FT_insertDir(char* path) {

   DTNode curr;
   int result = SUCCESS;

   assert(path != NULL);

   /* If trying to insert a directory into an uninitialized file tree. */
   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   /* If root is a file, inserting a directory will never be possible
      and always yield a CONFLICTING_PATH error. */
   if (fileRoot != NULL) {
      return CONFLICTING_PATH;
   }

   curr = FT_traversePath(path, &result);

   /* If traversePath finds a directory node for the input path
      to be inserted into. */
   if (result == SUCCESS) {
      result = FT_insertRestOfPath(path, curr, FALSE, NULL, 0);
   }
   /* If a file is found on the path. */
   else if (result == PARENT_CHILD_ERROR) {
      /* If path being inserted is already in the tree, but as
         the path for a file. */
      if (!strcmp(path, FileNode_getPath((FileNode)curr))) {
         result = ALREADY_IN_TREE;
      }
   }
   return result;
}

/* ft.h contains specification. */
boolean FT_containsDir(char* path) {
   DTNode curr;
   int result = SUCCESS;

   assert(path != NULL);

   /* If trying to check for a directory in an uninitialized file tree. */
   if(!isInitialized) {
      return FALSE;
   }

   /* If root is a file, a directory can never be present in the file tree. */
   if (fileRoot != NULL) {
      return FALSE;
   }

   curr = FT_traversePath(path, &result);

   /* If no node is found whose path  matches the prefix of the path. */
   if(curr == NULL) {
      return FALSE;
   }

   /* In case node with path prefix is found, but path is not identical. */
   else if(strcmp(path, DTNode_getPath(curr))) {
      return FALSE;
   }

   /* In case file with path is found. */
   else if (result == PARENT_CHILD_ERROR) {
      return FALSE;
   }

   /* If DTNode with input path is found in the file tree. */
   else {
      return TRUE;
   }
}

/* ft.h contains specification. */
int FT_insertFile(char* path, void *contents, size_t length) {

   DTNode curr;
   int result = SUCCESS;
   char* checkPath;
   FileNode rootNode;

   assert(path != NULL);

   /* If trying to insert a file into an uninitialized file tree. */
   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   /* In case root is a file. */
   if (fileRoot != NULL) {
      /* If path of root file matches input path, node is
         already in the tree. */
      if ((strcmp(path, FileNode_getPath(fileRoot))) == 0) {
         return ALREADY_IN_TREE;
      }
      /* If trying to insert any other file, conflicting path. */
      else {
         return CONFLICTING_PATH;
      }
   }

   if (root == NULL) {
      checkPath = strstr(path, "/");
      /* If there are no slashes, i.e., if only file is being inserted
         at root. */
      if (checkPath == NULL) {
         rootNode = FileNode_create(path, NULL, contents, length);
         if (rootNode != NULL) {
            fileRoot = rootNode;
         } else {
            return MEMORY_ERROR;
         }
      }
   }

   curr = FT_traversePath(path, &result);

   /* If DTNode with path = prefix of input path is found for the
      new node to be inserted into. */
   if (result == SUCCESS) {
      result = FT_insertRestOfPath(path, curr, TRUE, contents, length);
   }
   /* If file is found. */
   else if (result == PARENT_CHILD_ERROR) {
      if (!strcmp(path, FileNode_getPath((FileNode)curr))) {
         /* If path of file found is same as inserted path. */
         result = ALREADY_IN_TREE;
      }
   }
   return result;
}

/* ft.h contains specification. */
boolean FT_containsFile(char* path) {
   FileNode curr;
   int result = SUCCESS;

   assert(path != NULL);

   /* If trying to check for a file in an uninitialized file tree. */
   if(!isInitialized) {
      return FALSE;
   }

   /* In case root is file, check for it being the
      same file. */
   if (fileRoot != NULL) {
      if (strcmp(path, FileNode_getPath(fileRoot)) == 0) {
         return TRUE;
      } else {
         return FALSE;
      }
   }

   curr = (FileNode) FT_traversePath(path, &result);

   /* If no node is found whose path  matches the prefix of the path. */
   if(curr == NULL) {
      return FALSE;
   }
   else if(strcmp(path, FileNode_getPath(curr))) {
      return FALSE;
   }
   /* If correct file is found. */
   else if (result == PARENT_CHILD_ERROR) {
      return TRUE;
   } else {
      return FALSE;
   }
}

/* Destroys the entire hierarchy of Nodes rooted at curr,
   including curr itself. */
static void FT_removePathFrom(DTNode curr) {
   if(curr != NULL) {
      count -= DTNode_destroy(curr);
   }
}
/* Removes the directory hierarchy rooted at path starting from Node
  curr. If curr is the data structure's root, root becomes NULL.

  Returns NO_SUCH_PATH if curr is not the Node for path,
  and SUCCESS otherwise. */
static int FT_rmPathAt(char* path, DTNode curr) {

   DTNode parent;

   assert(path != NULL);
   assert(curr != NULL);

   parent = DTNode_getParent(curr);

   /* If path of current is the same as input path. */
   if(!strcmp(path, DTNode_getPath(curr))) {
      if(parent == NULL) {
         root = NULL;
      }
      else {
         DTNode_unlinkChildDirectory(parent, curr);
      }

      FT_removePathFrom(curr);

      return SUCCESS;
   }
   else {
      return NO_SUCH_PATH;
   }
}

/* ft.h contains specification. */
int FT_rmDir(char* path) {
   DTNode curr;
   int result;

   assert(path != NULL);

   /* If trying to remove directory from an uninitialized file tree. */
   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   curr = FT_traversePath(path, &result);
   /* If no node is found whose path  matches the prefix of the path. */
   if(curr == NULL) {
      result =  NO_SUCH_PATH;
   }
   /* If a file is found with the input path. */
   else if (result == PARENT_CHILD_ERROR) {
      result = NOT_A_DIRECTORY;
   }
   /* A directory is found with the input path. */
   else {
      result = FT_rmPathAt(path, curr);
   }
   return result;
}

/* ft.h contains specification. */
int FT_rmFile(char* path) {
   FileNode curr;
   int result;

   assert(path != NULL);

   /* If trying to remove file from an uninitialized file tree. */
   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   /* If the root node is a file. */
   if (fileRoot != NULL) {
      /* If the path of the file node is the same as that of the
         file to be removed. */
      if (strcmp(path, FileNode_getPath(fileRoot)) == 0) {
         FileNode_destroy(fileRoot);
         count = 0;
         fileRoot = NULL;
         return SUCCESS;
      } else {
         return NO_SUCH_PATH;
      }
   }


   curr = (FileNode) FT_traversePath(path, &result);
   /* If no node is found whose path  matches the prefix of the path. */
   if(curr == NULL) {
      result =  NO_SUCH_PATH;
   }
   /* If a file is found. */
   else if (result == PARENT_CHILD_ERROR) {
      if (strcmp(path, FileNode_getPath(curr)) != 0) {
         /* If the path of the file found does not match that of the one
            to be removed. */
         result =  NO_SUCH_PATH;
      }
      /* If the correct file is found, destroy the node and decrement
         the number of nodes in the file tree. */
      else {
         FileNode_destroy(curr);
         count--;
         result = SUCCESS;
      }
   }
   /* Node with the same prefix as path to be inserted is found, but is
      a directory. */
   else {
      result = NOT_A_FILE;
   }
   return result;
}

/* ft.h contains specification. */
int FT_init(void) {
   if(isInitialized) {
      return INITIALIZATION_ERROR;
   }
   isInitialized = 1;
   root = NULL;
   fileRoot = NULL;
   count = 0;
   return SUCCESS;
}

/* ft.h contains specification. */
int FT_destroy(void) {
   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }
   if (fileRoot == NULL) {
      FT_removePathFrom(root);
   } else {
      FileNode_destroy(fileRoot);
   }

   isInitialized = 0;
   count = 0;
   root = NULL;
   fileRoot = NULL;
   return SUCCESS;
}

/* ft.h contains specification. */
void *FT_getFileContents(char *path) {
   FileNode curr;
   int result;

   assert(path != NULL);

   /* If trying to retrieve contents of a file from an uninitialized
      file tree. */
   if(!isInitialized) {
      return NULL;
   }

   /* If root node is a file. */
   if (fileRoot != NULL) {
      /* If path of root file is same as path of file whose contents are
         to be retrieved. */
      if ((strcmp(path, FileNode_getPath(fileRoot))) == 0) {
         return FileNode_getContents(fileRoot);
      }
      /* If not, since no other files can exist in the tree, return NULL. */
      else {
         return NULL;
      }
   }

   curr = (FileNode) FT_traversePath(path, &result);

   /* If no node is found whose path  matches the prefix of the path. */
   if(curr == NULL) {
      return NULL;
   }

   /* A file is found. */
   else if (result == PARENT_CHILD_ERROR) {
      /* The path of the file found is not the same as that of the
         one whose contents are to be retrived. */
      if (strcmp(path, FileNode_getPath(curr)) != 0) {
         return NULL;
      }
      /* Path of the file found are the same as that of the one whose
         contents are to be retrieved. */
      else {
         return FileNode_getContents(curr);
      }
   }
   else {
      return NULL;
   }
}

/* ft.h contains specification. */
void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength) {
   FileNode curr;
   int result;

   assert(path != NULL);

   /* If trying to replace contents of a file from an uninitialized
      file tree. */
   if(!isInitialized) {
      return NULL;
   }

   /* If root node is a file. */
   if (fileRoot != NULL) {
      /* If path of root file is same as path of file whose contents are
        to be replaced. */
      if ((strcmp(path, FileNode_getPath(fileRoot))) == 0) {
         return FileNode_replaceContents(fileRoot, newContents,
                                         newLength);
      }
      /* If not, since no other files can exist in the tree, return NULL. */
      else {
         return NULL;
      }
   }

   curr = (FileNode) FT_traversePath(path, &result);

   /* If no node is found whose path  matches the prefix of the path. */
   if(curr == NULL) {
      return NULL;
   }
   /* A file is found. */
   else if (result == PARENT_CHILD_ERROR) {
      /* The path of the file found is not the same as that of the
         one whose contents are to be replaced. */
      if (strcmp(path, FileNode_getPath(curr)) != 0) {
         return NULL;
      }
      /* Path of the file found are the same as that of the one whose
         contents are to be replaced. */
      else {
         return FileNode_replaceContents(curr, newContents, newLength);
      }
   }
   else {
      return NULL;
   }
}

/* ft.h contains specification. */
int FT_stat(char *path, boolean* type, size_t* length) {
   DTNode currDir;
   FileNode currFile;
   int result = SUCCESS;

   assert(path != NULL);

   /* If trying to check status of a path in an uninitialized
      file tree. */
   if(!isInitialized) {
      return FALSE;
   }

   /* If root node is a file. */
   if (fileRoot != NULL) {
      /* If path of root file is same as input path. */
      if ((strcmp(path, FileNode_getPath(fileRoot))) == 0) {
         *type = TRUE;
         *length = FileNode_getLength(fileRoot);
         return SUCCESS;
      } else {
         return NULL;
      }
   }

   currDir = FT_traversePath(path, &result);

   /* Neither file not directory found. */
   if(currDir == NULL) {
      result = NO_SUCH_PATH;
   }
   /* File found. */
   else if (result == PARENT_CHILD_ERROR) {
      currFile = (FileNode)currDir;
      /* Different file found. */
      if (strcmp(path, FileNode_getPath(currFile)) != 0) {
         result =  NO_SUCH_PATH;
      }
      /* Correct file found. */
      else {
         *type = TRUE;
         *length = FileNode_getLength(currFile);
         result = SUCCESS;
      }
   }
   /* Different directory found. */
   else if (strcmp(path, DTNode_getPath(currDir))) {
      result = NO_SUCH_PATH;
   }
   /* Directory found. */
   else {
      *type = FALSE;
      result = SUCCESS;
   }
   return result;
}

/*
  Performs a pre-order traversal of the tree rooted at n,
  inserting each payload to DynArray_T d beginning at index i.
  Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(DTNode n, DynArray_T d, size_t i) {
   size_t c;

   assert(d != NULL);

   if(n != NULL) {
      (void) DynArray_set(d, i, DTNode_getPath(n));
      i++;

      /* Traversing all the file children of a given DTNode. */
      for (c = 0; c < DTNode_getNumFileChildren(n); c++) {
         (void) DynArray_set(d, i,
                             FileNode_getPath((FileNode)DTNode_getChild(n, c, TRUE)));
         i++;
      }

      /* Recursively traversing all the directory children of a given
         DTNode. */
      for(c = 0; c < DTNode_getNumDTChildren(n); c++) {
         i = FT_preOrderTraversal(DTNode_getChild(n, c, FALSE), d, i);
      }
   }
   return i;
}

/*
  Alternate version of strlen that uses pAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  str, and also always adds one more in addition to str's length.
*/
static void FT_strlenAccumulate(char* str, size_t* pAcc) {
   assert(pAcc != NULL);

   if(str != NULL)
      *pAcc += (strlen(str) + 1);
}

/*
  Alternate version of strcat that inverts the typical argument
  order, appending str onto acc, and also always adds a newline at
  the end of the concatenated string.
*/
static void FT_strcatAccumulate(char* str, char* acc) {
   assert(acc != NULL);

   if(str != NULL)
      strcat(acc, str); strcat(acc, "\n");
}

/* ft.h contains specification. */
char *FT_toString() {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   /* If trying to retrieve string representation of an
      uninitialized file tree. */
   if(!isInitialized) {
      return NULL;
   }

   /* If root is file, returning string representation of its path. */
   if (fileRoot != NULL) {
      return (char*)FileNode_getPath(fileRoot);
   }

   /* Else, conducting pre-order traversal to go through all nodes in a
      given tree. */
   nodes = DynArray_new(count);
   (void) FT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      return NULL;
   }

   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);
   return result;
}
