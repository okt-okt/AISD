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
        unsigned int height;
        T data;
        Node(T value, Node* parent, unsigned int height = 0) : data(value), left(NULL), right(NULL), height(height), parent(parent)
        {
        }
    }* root;


    BinaryTreeType type;


    BinaryTree(Node* node, BinaryTreeType type) : root(node), type(type)
    {
    }


    static Node* ptr_to_node(T* ptr)
    {
        return ptr ? reinterpret_cast<Node*>(reinterpret_cast<Node**>(reinterpret_cast<unsigned int*>(ptr) - 1) - 3) : NULL;
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
        node->parent = node_next;
        update_height(node);
        update_height(node_next);
        return node_next;
    }



    Node* rotate_right(Node* node)
    {
        Node* node_next = node->left, * node_last = node_next->right;
        node_next->right = node;
        if(node->left = node_last)
            node_last->parent = node;
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
                node->left = rotate_left(node->left);
            return rotate_right(node);
        }
        else if (bf > 1)
        {
            if (balance_factor(node->right) < 0)
                node->right = rotate_right(node->right);
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
            Node* added {}, * parent;
            for (;;)
            {
                if (value == caret->data)
                    break;
                if (value < caret->data)
                {
                    if (caret->left)
                        caret = caret->left;
                    else
                        caret->left = added = new Node(value, caret, 1);
                }
                else
                {
                    if (caret->right)
                        caret = caret->right;
                    else
                        caret->right = added = new Node(value, caret, 1);
                }
            }
            for (caret = added; caret; caret = parent)
            {
                update_height(caret);
                parent = caret->parent;
                if (type == AVL)
                {
                    caret = (caret->parent ? (caret->data < caret->parent->data ? caret->parent->left : caret->parent->right) : root) = balance(caret);
                    caret->parent = parent;
                }
            }
            return node_to_ptr(added);
        }
        else
            return node_to_ptr(root = new Node(value, NULL, 1));
    }



    void remove(T value)
    {
        Node** to_remove = &root, * parent, * caret, * trash;

    REMOVE:
        while ((*to_remove)->data != value)
            if (value < (*to_remove)->data)
                to_remove = &((*to_remove)->left);
            else
                to_remove = &((*to_remove)->right);
        if (*to_remove)
        {
            parent = (*to_remove)->parent;
            trash = (*to_remove);
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

            if (type == AVL)
                for (caret = *to_remove; caret; caret = parent)
                {
                    update_height(caret);
                    parent = caret->parent;
                    if (type == AVL)
                    {
                        caret = (caret->parent ? (caret->data < caret->parent->data ? caret->parent->left : caret->parent->right) : root) = balance(caret);
                        caret->parent = parent;
                    }
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
        return root ? root->height : 0;
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
            std::cout << "+- " << node->data << '\n' << children_space;
            print_part(node->right, children_space + "|  ");
            std::cout << children_space;
            print_part(node->left, children_space + "   ");
        }
        else
            std::cout << "+- NULL\n";
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
        {
            if (root->left)
                subtree(root->left).destroy();
            if (root->right)
                subtree(root->right).destroy();
            delete root;
            root = NULL;
        }
    }
};
