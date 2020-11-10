/*--------------------------------------------------------------------*/
/* DTNode.h                                                           */
/* Author: Eesha Agarwal                                              */
/*--------------------------------------------------------------------*/

#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "FTNode.h"

/*
   a DTNode is an object that contains a path payload and references to
   the DTNode's parent (if it exists) and children (if they exist).
*/

/*
   Given a parent Node and a directory string dir, returns a new
   Node structure or NULL if any allocation error occurs in creating
   the node or its fields.

   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory string parameter,
   separated by a slash. It is also initialized with its parent link
   as the parent parameter value (but the parent itself is not changed
   to link to the new Node.  The children links are initialized but
   do not point to any children.
*/

DTNode DTNode_create(const char* dir, DTNode parent);

/*
  Destroys the entire hierarchy of Nodes rooted at n,
  including n itself.

  Returns the number of Nodes destroyed.
*/
size_t DTNode_destroy(DTNode n);

/*
  Compares node1 and node2 based on their paths.
  Returns <0, 0, or >0 if node1 is less than,
  equal to, or greater than node2, respectively.
*/
int DTNode_compare(DTNode node1, DTNode node2);

/*
   Returns Node n's path.
*/
const char* DTNode_getPath(DTNode n);

/*
  Returns the number of child directories n has.
*/
size_t DTNode_getNumDTChildren(DTNode n);

/*
  Returns the number of child files n has.
*/
size_t DTNode_getNumFileChildren(DTNode n);
/*
   Returns 1 if n has a child directory with path,
   0 if it does not have such a child, and -1 if
   there is an allocation error during search.

   If n does have such a child, and childID is not NULL, store the
   child's identifier in *childID. If n does not have such a child,
   store the identifier that such a child would have in *childID.
*/
int DTNode_hasChild(DTNode n, const char* path, size_t* childID);

/*
   Returns the child Node of n with identifier childID, if one exists,
   otherwise returns NULL; type is used to indicate whether file node
   (type = TRUE), or directory node (type = FALSE) is being searched for.
*/
DTNode DTNode_getChild(DTNode n, size_t childID, boolean type);

/*
   Returns the parent Node of n, if it exists, otherwise returns NULL
*/
DTNode DTNode_getParent(DTNode n);

/*
  Makes child directory a child of parent, if possible, and returns SUCCESS.
  This is not possible in the following cases:
  * child's path is not parent's path + / + directory,
    in which case returns PARENT_CHILD_ERROR
    * parent already has a child with child's path,
    in which case returns ALREADY_IN_TREE
    * parent is unable to allocate memory to store new child link,
    in which case returns MEMORY_ERROR
 */
int DTNode_linkChildDirectory(DTNode parent, DTNode child);

/*
  Makes child file a child of parent, if possible, and returns SUCCESS.
  This is not possible in the following cases:
  * child's path is not parent's path + / + directory,
    in which case returns PARENT_CHILD_ERROR
    * parent already has a child with child's path,
    in which case returns ALREADY_IN_TREE
    * parent is unable to allocate memory to store new child link,
    in which case returns MEMORY_ERROR
*/
int DTNode_linkChildFile(DTNode parent, FileNode child);

/*
  Unlinks Node parent from its child directory Node child, leaving the
  child Node unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
*/
int DTNode_unlinkChildDirectory(DTNode parent, DTNode child);

/*
  Unlinks Node parent from its child file Node child, leaving the
  child Node unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
*/
int  DTNode_unlinkChildFile(DTNode parent, FileNode child);

/*
  Creates a new Node such that the new Node's path is dir appended to
  n's path, separated by a slash, and that the new Node has no
  children of its own. The new node's parent is n, and the new node is
  added as a child of n.

  (Reiterating for clarity: unlike with Node_create, parent *is*
  changed so that the link is bidirectional.)

  Returns SUCCESS upon completion, or:
  MEMORY_ERROR if the new Node cannot be created,
  ALREADY_IN_TREE if parent already has a child with that path
*/
int DTNode_addChildDir(DTNode parent, const char* dir);

/*
  Creates a new Node such that the new Node's path is dir appended to
  n's path, separated by a slash, and that the new Node has no
  children of its own. The new node's parent is n, and the new file node is
  added as a child of n, with file contents = contents, and length =
  parameter length.

  (Reiterating for clarity: unlike with Node_create, parent *is*
  changed so that the link is bidirectional.)

  Returns SUCCESS upon completion, or:
  MEMORY_ERROR if the new Node cannot be created,
  ALREADY_IN_TREE if parent already has a child with that path
*/
int DTNode_addChildFile(DTNode parent, const char* file,
                        void *contents, size_t length);

/*
  Returns a string representation n, or NULL if there is an allocation
  error.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char* DTNode_toString(DTNode n);

#endif
