#ifndef k_rbtree_h
#define k_rbtree_h

#include "k_types.h"

#define DEF_RBNODE_TYPE(node_type) \
  enum k_color_type_t color;\
  node_type* parent;   \
  node_type* left;     \
  node_type* right

#define DEF_RBTREE_TYPE(node_type) \
  k_compare_t  compare;\
  k_getkey_t   getkey;\
  node_type*   nil_node;\
  node_type*   root;\
  node_type*   left;\
  node_type*   right

enum k_color_type_t{
  k_color_red,
  k_color_black,
};

typedef struct  k_rbnode_s{
  DEF_RBNODE_TYPE(struct k_rbnode_s);
}k_rbnode_t;

typedef struct  k_rbtree_s{
  DEF_RBTREE_TYPE(struct k_rbnode_s);
}k_rbtree_t;

void
k_rbtree_init(k_rbtree_t* rbtree,k_compare_t compare,k_getkey_t getkey);

void
k_rbtree_insert(k_rbtree_t* rbtree,k_rbnode_t* rbnode);

void 
k_rbtree_remove(k_rbtree_t* rbtree,k_rbnode_t* rbnode);

k_rbnode_t*
k_rbtree_find(k_rbtree_t* rbtree,void* key);

#endif /* rbtree_h */
