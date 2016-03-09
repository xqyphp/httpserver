#include "k_rbtree.h"


static void
left_rotate(k_rbtree_t* t,k_rbnode_t* x)
{
        k_rbnode_t* y = x->right;
        x->right = y->left;
        if(y->left != t->nil_node)
        {
                y->left->parent = x;
        }

        y->parent = x->parent;
        if(x->parent == t->nil_node)
        {
                t->root = y;
        }else if(x->parent->left == x)
        {
                x->parent->left = y;
        }else if(x->parent->right == x){
                x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
}

static void
right_rotate(k_rbtree_t* t,k_rbnode_t* y)
{
        k_rbnode_t* x = y->left;

        y->left = x->right;//put x's right node to y's left node
        if(x->right != t->nil_node)
        {
                x->right->parent = y;
        }

        x->parent = y->parent;//link y->parent to x
        if(x->parent == t->nil_node)
        {
                t->root = x;
        }else if(y->parent->left == y){
                y->parent->left = x;
        }else if(y->parent->right == y){
                y->parent->right = x;
        }

        x->right = y;//put y to x's right
        y->parent = x;
}

void
k_rbtree_init(k_rbtree_t* rbtree,k_compare_t compare,
	      k_getkey_t getkey)
{
        rbtree->compare = compare;
        rbtree->getkey  = getkey;
        rbtree->nil_node= K_NULL;
        rbtree->root = rbtree->nil_node;
        rbtree->left = rbtree->nil_node;
        rbtree->right= rbtree->nil_node;
}

static void
k_rbtree_insert_fixup(k_rbtree_t* t,k_rbnode_t* z)
{
        k_rbnode_t* y = K_NULL;
        while(z->parent->color == k_color_red)
        {
                if(z->parent == z->parent->parent->left)
                {
                        y = z->parent->parent->right;
                        if(y->color == k_color_red)//case 1
                        {
                                z->parent->color = k_color_black;
                                y->color = k_color_black;
                                z->parent->parent->color = k_color_red;
                                z = z->parent->parent;
                        }else{
                                if(z == z->parent->right){//case 2
                                        z = z->parent;
                                        left_rotate(t,z);
                                }
                                //case 2 will turn to case 3
                                z->parent->color = k_color_black;//case 3
                                z->parent->parent->color = k_color_red;
                                right_rotate(t,z->parent->parent);

                        }

                }else{
                        y = z->parent->parent->left;
                        if(y->color == k_color_red)//case 1
                        {
                                z->parent->color = k_color_black;
                                y->color = k_color_black;
                                z->parent->parent->color = k_color_red;
                                z = z->parent->parent;
                        }else{
                                if(z == z->parent->left){//case 2
                                        z = z->parent;
                                        right_rotate(t,z);
                                }

                                z->parent->color = k_color_black;//case 3
                                z->parent->parent->color = k_color_red;
                                left_rotate(t,z->parent->parent);
                        }

                }
        }
        t->root->color = k_color_black;
}

void
k_rbtree_insert(k_rbtree_t* t,k_rbnode_t* z)
{
        k_rbnode_t* y = t->nil_node;
        k_rbnode_t* x = t->root;

        while(x != t->nil_node)
        {
                y = x;
                if(t->compare(t->getkey(z),t->getkey(x)) < 0)
                {
                        x = x->left;
                }else{
                        x = x->right;
                }
        }
        z->parent = y;
        if(y == t->nil_node)
        {
                t->root = z;
        }else if(t->compare(t->getkey(z),t->getkey(y)) < 0){
                y->left = z;
        }else{
                y->right = z;
        }

        z->left = t->nil_node;
        z->right = t->nil_node;
        z->color = k_color_red;
        k_rbtree_insert_fixup(t,z);
}

static void
rb_transplant(k_rbtree_t* t,k_rbnode_t* u,k_rbnode_t* v)
{
        if(u->parent == t->nil_node){
                t->root = v;
        }else if(u == u->parent->left){
                u->parent->left = v;
        }else{
                u->parent->right = v;
        }
        u->parent = u->parent;

}

static k_rbnode_t*
rb_minimum(k_rbtree_t* t,k_rbnode_t* z)
{
        k_rbnode_t* y = t->nil_node;

        while(z != t->nil_node){
                y = z;
                z = z->left;
        }
        return y;

}

static void
k_rbtree_delete_fixup(k_rbtree_t* t,k_rbnode_t* x)
{
        k_rbnode_t* w;// w is x's brother
        while( (x != t->root)&& (x->color == k_color_black) ){

                if(x == x->parent->left){
                        w = x->parent->right;
                        if(w->color == k_color_red){
                                w->color = k_color_black;
                                x->parent->color = k_color_red;
                                left_rotate(t, x->parent);
                                w = x->parent->left;
                        }
                        if(w->left->color == k_color_black &&
                           w->right->color == k_color_black){
                                w->color = k_color_red;
                                x = x->parent;
                        }else if (w->right->color == k_color_black){
                                w->left->color = k_color_black;
                                w->color = k_color_red;
                                right_rotate(t, w);
                                w = x->parent->right;
                        }
                        w->color = x->parent->color;
                        w->parent->color = k_color_black;
                        left_rotate(t, x->parent);
                        x = t->root;

                }else{//x == x->parent->right;
                        w = x->parent->left;
                        if(w->color == k_color_red){
                                w->color = k_color_black;
                                x->parent->color = k_color_red;
                                right_rotate(t,x->parent);
                                w = x->parent->right;
                        }
                        if(w->right->color == k_color_black &&
                           w->left->color == k_color_black){
                                w->color = k_color_red;
                                x = x->parent;
                        }else if ( w->left->color = k_color_black){
                                w->right->color = k_color_black;
                                w->color = k_color_red;
                                left_rotate(t,w);
                                w = w->parent->left;
                        }
                        w->color = x->parent->color;
                        w->parent->color = k_color_black;
                        right_rotate(t, x->parent);
                        x = t->root;

                }

        }
        x->color = k_color_black;
}

void
k_rbtree_delete(k_rbtree_t* t,k_rbnode_t* z)
{
        /*  y's postion is the really to be delete,
         *   move y to z's position  and move x to y's postion
         */
        k_rbnode_t* y = z;
        k_rbnode_t* x = t->nil_node;

        enum k_color_type_t y_old_color = y->color;

        if(z->left == t->nil_node){//if one of z's child is nil move the other child to z;
                x = z->right;
                rb_transplant(t,z,z->right);
        }else if(z->right == t->nil_node){
                x = z->left;
                rb_transplant(t,z,z->left);
        }else{// y to be z's successor
                y = rb_minimum(t,z->right);
                y_old_color = y->color;
                x = y->right;

                //since y has no left child then move y's right to be y
                if(y->parent == z){//move x to y's postion
                        x->parent = z;
                }else{
                        rb_transplant(t,y,x);
                        y->right = z->right;
                        y->right->parent = y;
                }
                // move y to z's postion
                rb_transplant(t,z,y);
                y->left = z->left;
                y->left->parent = y;
                y->color = z->color;

        }

        if(y_old_color == k_color_black){
                k_rbtree_delete_fixup(t,x);
        }

}
