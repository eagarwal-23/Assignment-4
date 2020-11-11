/*--------------------------------------------------------------------*/
/* checker.c                                                          */
/* Author: Eesha Agarwal                                              */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checker.h"


/* see checker.h for specification */
boolean Checker_Node_isValid(Node n) {
   Node parent, child, sibling;
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;
   size_t numChildren;

   /* Sample check: a NULL pointer is not a valid Node */
   if(n == NULL) {
      fprintf(stderr, "Node is a NULL pointer.\n");
      return FALSE;
   }

/* The path for a node should never be NULL. */
   npath = Node_getPath(n);
   if (npath == NULL) {
      fprintf(stderr, "Path of a node in the tree is NULL.\n");
      return FALSE;
   }

   parent = Node_getParent(n);
   if(parent != NULL) {
      npath = Node_getPath(n);

      /* Sample check that parent's path must be prefix of n's path */
      ppath = Node_getPath(parent);
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path.\n");
         return FALSE;
      }
      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path.\n");
         return FALSE;
      }

      /* Checking if parent and child are mutually linked. */
      numChildren = Node_getNumChildren(parent);
      for (i = 0; i < numChildren; i++) {
         child = Node_getChild(parent, i);
         if (Node_getParent(child) != parent) {
            fprintf(stderr, "Parent and child not mutually linked.\n");
            return FALSE;
         }
      }

      /* Checking if children are lexicographically-ordered. */
      for (i = 0; i < numChildren - 1; i++) {
         child = Node_getChild(parent, i);
         sibling = Node_getChild(parent, i + 1);
         if (Node_compare(child, sibling) > 0) {
            fprintf(stderr, "Children not lexicographically-ordered.\n");
            return FALSE;
         }

         if (Node_compare(child, sibling) == 0) {
            fprintf(stderr, "Non unique children.\n");
            return FALSE;
         }
      }
   }

   /* If none of the checks are failed, return TRUE. */
   return TRUE;

}

/*
  Performs a pre-order traversal of the tree rooted at n.
  Returns FALSE if a broken invariant is found and
  returns TRUE otherwise. Counts the number of nodes
  and updates the variable pointed to by numNodes while
  performing pre-order traversal.
*/
static boolean Checker_treeCheck(Node n, size_t *numNodes) {
   size_t c;

   assert(numNodes != NULL);

   if(n != NULL) {

      /* Sample check on each non-root Node: Node must be valid */
      /* If not, pass that failure back up immediately */
      if(!Checker_Node_isValid(n))
         return FALSE;

      /* Counting number of valid nodes. */
      (*numNodes)++;

      for(c = 0; c < Node_getNumChildren(n); c++)
      {
         Node child = Node_getChild(n, c);

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!Checker_treeCheck(child, numNodes))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checker.h for specification */
boolean Checker_DT_isValid(boolean isInit, Node root, size_t count) {
   size_t numNodes = 0;
   boolean treeCheck = TRUE;

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!isInit) {
      if(count != 0) {
         fprintf(stderr, "Not initialized, but count is not 0.\n");
         return FALSE;
      }

      /* If the DT is not initialized, its root should be NULL. */
      if (root != NULL) {
         fprintf(stderr, "Not initialized, but root is not NULL\n");
         return FALSE;
      }
   }
   else {
      /* Checking if root is NULL but count is not 0. */
      if ((root == NULL) && (count != 0)) {
         fprintf(stderr, "Root is NULL but count is not 0.\n");
         return FALSE;
      }

      /* Checking if count is 0 but root is not NULL. */
      if ((count == 0) && (root != NULL)) {
         fprintf(stderr, "Count is 0 but root is not NULL.\n");
         return FALSE;
      }

      /* Checking that parent of the root is NULL as expected. */
      if (root != NULL) {
         if(Node_getParent(root) != NULL) {
            fprintf(stderr, "Parent of root is not NULL.\n");
            return FALSE;
         }
      }

      /* Now checks invariants recursively at each Node from the root. */
      treeCheck = Checker_treeCheck(root, &numNodes);
      if (treeCheck) {

         /* Checking that the number of nodes and count are consistent
            i.e. that the number of nodes is equal to count. */
         if (count != numNodes) {
            fprintf(stderr, "Incorrect number of nodes.\n");
            return FALSE;
         }
      }
   }

   return treeCheck;
}
