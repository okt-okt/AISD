#pragma once
#include <iostream>
#define max(a, b) ((a)>(b) ? (a) : (b))




enum BinaryTreeType : int
{
    BST,
    AVL,
    RNB
};



template<typename T>
class BinaryTree
{
    struct Node
    {
        Node* left, * right, * parent;
        unsigned int height, color;
        T data;
        Node(T value, Node* parent, unsigned int color = '0', unsigned int height = 1) : data(value), left(NULL), right(NULL), height(height), parent(parent), color(color)
        {
        }
    }* root;


    BinaryTreeType type;


    BinaryTree(Node* node, BinaryTreeType type) : root(node), type(type)
    {
    }


    static Node* ptr_to_node(T* ptr)
    {
        return ptr ? reinterpret_cast<Node*>(reinterpret_cast<Node**>(reinterpret_cast<unsigned int*>(ptr) - 2) - 3) : NULL;
    }

    
    static T* node_to_ptr(Node* node)
    {
        return node ? &(node->data) : NULL;
    }


    BinaryTree<T> subtree(Node* node)
    {
        return { node, type };
    }

public:

    BinaryTree(BinaryTreeType type) : root(NULL), type(type)
    {
    }



    T* find(T value)
    {
        Node* caret = root;
        while (caret && caret->data != value)
            if (value < caret->data)
                caret = caret->left;
            else
                caret = caret->right;
        return node_to_ptr(caret);
    }

protected:

    Node* rotate_left(Node* node)
    {
        Node* node_next = node->right, * node_last = node_next->left;
        node_next->left = node;
        if (node->right = node_last)
            node_last->parent = node;
        (node->parent ? (node->parent->left == node ? node->parent->left : node->parent->right) : root) = node_next;
        node_next->parent = node->parent;
        node->parent = node_next;
        update_height(node);
        update_height(node_next);
        return node_next;
    }



    Node* rotate_right(Node* node)
    {
        Node* node_next = node->left, * node_last = node_next->right;
        node_next->right = node;
        if (node->left = node_last)
            node_last->parent = node;
        (node->parent ? (node->parent->left == node ? node->parent->left : node->parent->right) : root) = node_next;
        node_next->parent = node->parent;
        node->parent = node_next;
        update_height(node);
        update_height(node_next);
        return node_next;
    }


    int balance_factor(Node* node)
    {
        return node ? (node->right ? node->right->height : 0) - (node->left ? node->left->height : 0) : 0;
    }



    Node* balance(Node* node)
    {
        int bf = balance_factor(node);
        if (bf < -1)
        {
            if (balance_factor(node->left) > 0)
                rotate_left(node->left);
            return rotate_right(node);
        }
        else if (bf > 1)
        {
            if (balance_factor(node->right) < 0)
                rotate_right(node->right);
            return rotate_left(node);
        }
        else
            return node;
    }


public:

    T* insert(T value)
    {
        if (Node* caret = root)
        {
            Node* added {}, * parent, * grandpa, * uncle;
            for (;;)
            {
                if (value == caret->data)
                    break;
                if (value < caret->data)
                {
                    if (caret->left)
                        caret = caret->left;
                    else
                        caret->left = added = new Node(value, caret, (type == RNB ? 'R' : '0'));
                }
                else
                {
                    if (caret->right)
                        caret = caret->right;
                    else
                        caret->right = added = new Node(value, caret, (type == RNB ? 'R' : '0'));
                }
            }
            for (caret = added; caret; caret = parent)
            {
                update_height(caret);
                parent = caret->parent;
                if (type == AVL)
                    balance(caret);
            }
            if (type == RNB && (caret = added))
            {
                while (caret != root && caret->parent->color == 'R')
                {
                    parent = caret->parent;
                    grandpa = parent->parent;

                    if (parent == grandpa->left)
                    {
                        uncle = grandpa->right;

                        if (uncle && uncle->color == 'R')
                        {
                            parent->color = uncle->color = 'B';
                            grandpa->color = 'R';
                            caret = grandpa;
                        }
                        else
                        {
                            if (caret == parent->right)
                            {
                                rotate_left(caret = parent);
                                parent = caret->parent;
                                grandpa = parent->parent;
                            }

                            parent->color = 'B';
                            grandpa->color = 'R';
                            rotate_right(grandpa);
                        }
                    }
                    else
                    {
                        uncle = grandpa->left;

                        if (uncle && uncle->color == 'R')
                        {
                            parent->color = uncle->color = 'B';
                            grandpa->color = 'R';
                            caret = grandpa;
                        }
                        else
                        {
                            if (caret == parent->left)
                            {
                                rotate_right(caret = parent);
                                parent = caret->parent;
                                grandpa = parent->parent;
                            }

                            parent->color = 'B';
                            grandpa->color = 'R';
                            rotate_left(grandpa);
                        }
                    }
                }
                root->color = 'B';
            }
            
            return node_to_ptr(added);
        }
        else
            return node_to_ptr(root = new Node(value, NULL, (type == RNB ? 'B' : '0')));
    }



    void remove(T value)
    {
        Node** to_remove = &root, * parent {}, * caret, * trash, * sibling;
        unsigned int col;

    REMOVE:
        while ((*to_remove) && (*to_remove)->data != value)
            if (value < (*to_remove)->data)
                to_remove = &((*to_remove)->left);
            else
                to_remove = &((*to_remove)->right);
        if (*to_remove)
        {
            col = (trash = (*to_remove))->color;
            parent = (*to_remove == root ? root : (*to_remove)->parent);
            if (!((*to_remove)->left) && !((*to_remove)->right))
            {
                *to_remove = NULL;
            }
            else if (!((*to_remove)->left))
            {
                if (*to_remove = (*to_remove)->right)
                    (*to_remove)->parent = trash->parent;
            }
            else if (!((*to_remove)->right))
            {
                if (*to_remove = (*to_remove)->left)
                    (*to_remove)->parent = trash->parent;
            }
            else
            {
                trash = ptr_to_node(subtree((*to_remove)->right).find_min());
                value = (*to_remove)->data = trash->data;
                to_remove = &((*to_remove)->right);
                goto REMOVE;
            }

            if (type == RNB && col == 'B')
            {
                for (caret = *to_remove; caret != root && (!caret || caret->color == 'B');)
                {
                    if (caret == parent->left)
                    {
                        sibling = parent->right;
                        if (sibling && sibling->color == 'R')
                        {
                            sibling->color = 'B';
                            parent->color = 'R';
                            rotate_left(parent);
                            sibling = parent->right;
                        }
                        if (sibling && (!(sibling->left) || sibling->left->color == 'B') && (!(sibling->right) || sibling->right->color == 'B'))
                        {
                            sibling->color = 'R';
                            if ((caret = parent) != root)
                                parent = caret->parent;
                        }
                        else
                        {
                            if (sibling && (!(sibling->right) || sibling->right->color == 'B'))
                            {
                                if (sibling->left)
                                    sibling->left->color = 'B';
                                sibling->color = 'R';
                                rotate_right(sibling);
                                sibling = parent->right;
                            }
                            if (sibling)
                                sibling->color = parent->color;
                            parent->color = 'B';
                            if (sibling && sibling->right)
                                sibling->right->color = 'B';
                            rotate_left(parent);
                            caret = root;
                        }
                    }
                    else
                    {
                        sibling = parent->left;
                        if (sibling && sibling->color == 'R')
                        {
                            sibling->color = 'B';
                            parent->color = 'R';
                            rotate_right(parent);
                            sibling = parent->left;
                        }
                        if (sibling && (!(sibling->left) || sibling->left->color == 'B') && (!(sibling->right) || sibling->right->color == 'B'))
                        {
                            sibling->color = 'R';
                            if ((caret = parent) != root)
                                parent = parent;
                        }
                        else
                        {
                            if (sibling && (!(sibling->left) || sibling->left->color == 'B'))
                            {
                                if (sibling->right)
                                    sibling->right->color = 'B';
                                sibling->color = 'R';
                                rotate_left(sibling);
                                sibling = parent->left;
                            }
                            if (sibling)
                                sibling->color = parent->color;
                            parent->color = 'B';
                            if (sibling && sibling->left)
                                sibling->left->color = 'B';
                            rotate_right(parent);
                            caret = root;
                        }
                    }
                    if (caret)
                        update_height(caret);
                    if (sibling)
                        update_height(sibling);
                }
                if (caret)
                    caret->color = 'B';
            }

            for (caret = (*to_remove ? *to_remove : parent); caret; caret = parent)
            {
                update_height(caret);
                parent = caret->parent;
                if (type == AVL)
                    balance(caret);
            }

            delete trash;
        }
    }

protected:

    unsigned int update_height(Node* node)
    {
        return node->height = 1 + max((node->right ? node->right->height : 0), (node->left ? node->left->height : 0));
    }

public:

    unsigned int height()
    {
        return (root ? root->height : 0) + (type == RNB);
    }



    T* find_max()
    {
        Node* caret = root;
        while (caret && caret->right)
            caret = caret->right;
        return node_to_ptr(caret);
    }



    T* find_min()
    {
        Node* caret = root;
        while (caret && caret->left)
            caret = caret->left;
        return node_to_ptr(caret);
    }



    void print()
    {
        std::cout << "root\n";
        print_part(root, "   ");
    }

protected:

    void print_part(Node* node, std::string children_space)
    {
        if (node)
        {
            std::cout << "+- " << node->data << (type == RNB ? (node->color == 'R' ? " (red)\n" : " (black)\n") : "\n") << children_space;
            print_part(node->right, children_space + "|  ");
            std::cout << children_space;
            print_part(node->left, children_space + "   ");
        }
        else
            std::cout << "+- NULL" << (type == RNB ? " (black)\n" : "\n");
    }

public:

    void print_straight()
    {
        if (root)
        {
            std::cout << root->data << ' ';
            subtree(root->left).print_straight();
            subtree(root->right).print_straight();
        }
    }



    void print_centre()
    {
        if (root)
        {
            subtree(root->left).print_centre();
            std::cout << root->data << ' ';
            subtree(root->right).print_centre();
        }
    }



    void print_reverse()
    {
        if (root)
        {
            subtree(root->left).print_reverse();
            subtree(root->right).print_reverse();
            std::cout << root->data << ' ';
        }
    }

protected:

    int print_width_part(unsigned int lvl)
    {
        if (root)
        {
            if (lvl)
                return subtree(root->left).print_width_part(lvl - 1) + subtree(root->right).print_width_part(lvl - 1);
            else
            {
                std::cout << root->data << ' ';
                return root->left || root->right;
            }
        }
        else
            return false;
    }

public:

    void print_width()
    {
        for (unsigned int lvl = 0, end = true; end; ++lvl)
            end = this->print_width_part(lvl);
    }



    void destroy()
    {
        if (root)
            for (Node** to_remove = &root, ** next, * trash; to_remove;)
                if ((*to_remove)->left)
                    to_remove = &((*to_remove)->left);
                else if ((*to_remove)->right)
                    to_remove = &((*to_remove)->right);
                else
                {
                    trash = *to_remove;
                    next = (to_remove == &root ? NULL : (trash->parent->parent ? &(trash->parent == trash->parent->parent->left ? trash->parent->parent->left : trash->parent->parent->right) : &root));
                    *to_remove = NULL;
                    to_remove = next;
                    delete trash;
                }
    }
};
