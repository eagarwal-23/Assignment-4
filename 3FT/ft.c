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

/* A File Tree is an AO with 3 state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* a pointer to a root DTNode in the hierarchy */
static DTNode root;
/* a pointer to a root FileNode in the hierarchy */
static FileNode fileRoot;
/* a counter of the number of Nodes in the hierarchy */
static size_t count;

/*
  Starting at the parameter curr, traverses as far down
  the hierarchy as possible while still matching the path
  parameter.

  Returns a pointer to the farthest matching Node down that path,
  or NULL if there is no node in curr's hierarchy that matches
  a prefix of the path
*/
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

   else if(!strncmp(path, DTNode_getPath(curr), strlen(DTNode_getPath(curr)))) {
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

         if (strcmp(DTNode_getPath(found), path) == 0) {
            *piResult = PARENT_CHILD_ERROR;
            return found;
         }

      }
      return curr;
   }
   return NULL;
}

static DTNode FT_traversePath(char* path, int* piResult) {
   assert(path != NULL);
   return FT_traversePathFrom(path, root, piResult);
}

static int FT_linkParentToChildFile(DTNode parent, FileNode child) {

   assert(parent != NULL);

   if(DTNode_linkChildFile(parent, child) != SUCCESS) {
      (void) FileNode_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

static int FT_linkParentToChildDirectory(DTNode parent, DTNode child) {

   assert(parent != NULL);

   if(DTNode_linkChildDirectory(parent, child) != SUCCESS) {
      (void) DTNode_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}


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
      if(!strcmp(path, DTNode_getPath(curr))) {
         return ALREADY_IN_TREE;
      }
      restPath += (strlen(DTNode_getPath(curr)) + 1);
   }

   copyPath = malloc(strlen(restPath)+1);
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
         if (!type) {
            result = FT_linkParentToChildDirectory(curr, newDir);
            if(result != SUCCESS) {
               (void) DTNode_destroy(newDir);

               /* To destroy the path that led us to this dir. */
               (void) DTNode_destroy(firstDir);
               free(copyPath);
               return result;
            }
         }
         else {
            result = FT_linkParentToChildFile(curr, newFile);
            if(result != SUCCESS) {
               (void) FileNode_destroy(newFile);

               /* To destroy the path that led us to this file. */
               (void) DTNode_destroy(firstDir);
               free(copyPath);
               return result;
            }
         }
      }

      if (nextToken == NULL) {
         if((!type) && (newDir == NULL)) {
            (void) DTNode_destroy(firstDir);
            free(copyPath);
            return MEMORY_ERROR;
         }

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

   if(parent == NULL) {
      root = firstDir;
      fileRoot = firstFile;
      count = newCount;
      return SUCCESS;
   }

   else {
      if (firstDir == NULL) {
         result = FT_linkParentToChildFile(parent, firstFile);
      }
      else {
         result = FT_linkParentToChildDirectory(parent, firstDir);
      }

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

int FT_insertDir(char* path) {

   DTNode curr;
   int result = SUCCESS;

   assert(path != NULL);

   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   if (fileRoot != NULL) {
      return CONFLICTING_PATH;
   }

   curr = FT_traversePath(path, &result);

   if (result == SUCCESS) {
      result = FT_insertRestOfPath(path, curr, FALSE, NULL, 0);
   } else if (result == PARENT_CHILD_ERROR) {
      if (!strcmp(path, FileNode_getPath((FileNode)curr))) {
         result = ALREADY_IN_TREE;
      }
   }

   return result;
}

boolean FT_containsDir(char* path) {
   DTNode curr;
   int result = SUCCESS;

   assert(path != NULL);

   if(!isInitialized) {
      return FALSE;
   }

   if (fileRoot != NULL) {
      return FALSE;
   }

   curr = FT_traversePath(path, &result);

   if(curr == NULL) {
      return FALSE;
   }
   else if(strcmp(path, DTNode_getPath(curr))) {
      return FALSE;
   }
   else if (result == PARENT_CHILD_ERROR) {
      return FALSE;
   } else {
      return TRUE;
   }
}

int FT_insertFile(char* path, void *contents, size_t length) {

   DTNode curr;
   int result = SUCCESS;
   char* checkPath;
   FileNode rootNode;

   assert(path != NULL);

   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   if (fileRoot != NULL) {
      if ((strcmp(path, FileNode_getPath(fileRoot))) == 0) {
         return ALREADY_IN_TREE;
      } else {
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

   if (result == SUCCESS) {
      result = FT_insertRestOfPath(path, curr, TRUE, contents, length);
   } else if (result == PARENT_CHILD_ERROR) {
      if (!strcmp(path, FileNode_getPath((FileNode)curr))) {
         result = ALREADY_IN_TREE;
      }
   }

   return result;
}

boolean FT_containsFile(char* path) {
   FileNode curr;
   int result = SUCCESS;

   assert(path != NULL);

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

   if(curr == NULL) {
      return FALSE;
   }
   /* In case file is found, but different file. */
   else if(strcmp(path, FileNode_getPath(curr))) {
      return FALSE;
   } else if (result == PARENT_CHILD_ERROR) {
      return TRUE;
   } else {
      return FALSE;
   }
}

/*
  Destroys the entire hierarchy of Nodes rooted at curr,
  including curr itself.
*/
static void FT_removePathFrom(DTNode curr) {
   if(curr != NULL) {
      count -= DTNode_destroy(curr);
   }
}
/*
  Removes the directory hierarchy rooted at path starting from Node
  curr. If curr is the data structure's root, root becomes NULL.

  Returns NO_SUCH_PATH if curr is not the Node for path,
  and SUCCESS otherwise. Update for type
*/
static int FT_rmPathAt(char* path, DTNode curr) {

   DTNode parent;

   assert(path != NULL);
   assert(curr != NULL);

   parent = DTNode_getParent(curr);

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

int FT_rmDir(char* path) {
   DTNode curr;
   int result;

   /* assert(Checker_DT_isValid(isInitialized,root,count)); */
   assert(path != NULL);

   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   curr = FT_traversePath(path, &result);
   if(curr == NULL) {
      result =  NO_SUCH_PATH;
   }
   else if (result == PARENT_CHILD_ERROR) {
      result = NOT_A_DIRECTORY;
   }
   else {
      result = FT_rmPathAt(path, curr);
   }

   return result;
}

int FT_rmFile(char* path) {
   FileNode curr;
   int result;

   assert(path != NULL);

   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   if (fileRoot != NULL) {
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
   if(curr == NULL) {
      result =  NO_SUCH_PATH;
   }
   else if (result == PARENT_CHILD_ERROR) {
      if (strcmp(path, FileNode_getPath(curr)) != 0) {
         result =  NO_SUCH_PATH;
      }
      else {
         FileNode_destroy(curr);
         count--;
         result = SUCCESS;
      }
   }
   else {
      result = NOT_A_FILE;
   }
   return result;
}

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

void *FT_getFileContents(char *path) {
   FileNode curr;
   int result;

   assert(path != NULL);

   if(!isInitialized) {
      return NULL;
   }

   if (fileRoot != NULL) {
      if ((strcmp(path, FileNode_getPath(fileRoot))) == 0) {
         return FileNode_getContents(fileRoot);
      } else {
         return NULL;
      }
   }

   curr = (FileNode) FT_traversePath(path, &result);
   if(curr == NULL) {
      return NULL;
   }
   else if (result == PARENT_CHILD_ERROR) {
      if (strcmp(path, FileNode_getPath(curr)) != 0) {
         return NULL;
      }
      else {
         return FileNode_getContents(curr);
      }
   }
   else {
      return NULL;
   }
}

void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength) {
   FileNode curr;
   int result;

   assert(path != NULL);

   if(!isInitialized) {
      return NULL;
   }

   if (fileRoot != NULL) {
      if ((strcmp(path, FileNode_getPath(fileRoot))) == 0) {
         return FileNode_replaceContents(fileRoot, newContents,
                                         newLength);
      } else {
         return NULL;
      }
   }

   curr = (FileNode) FT_traversePath(path, &result);
   if(curr == NULL) {
      return NULL;
   }
   else if (result == PARENT_CHILD_ERROR) {
      if (strcmp(path, FileNode_getPath(curr)) != 0) {
         return NULL;
      }
      else {
         return FileNode_replaceContents(curr, newContents, newLength);
      }
   }
   else {
      return NULL;
   }
}

int FT_stat(char *path, boolean* type, size_t* length) {
   DTNode currDir;
   FileNode currFile;
   int result = SUCCESS;

   assert(path != NULL);

   if(!isInitialized) {
      return FALSE;
   }

   if (fileRoot != NULL) {
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

      for (c = 0; c < DTNode_getNumFileChildren(n); c++) {
         (void) DynArray_set(d, i,
                             FileNode_getPath((FileNode)DTNode_getChild(n, c, TRUE)));
         i++;
      }

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

char *FT_toString() {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   if(!isInitialized) {
      return NULL;
   }

   if (fileRoot != NULL) {
      return FileNode_getPath(fileRoot);
   }

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
