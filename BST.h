#pragma once
#include <iostream>
#define max(a,b) ((a)>(b)?(a):(b))




template<typename T>
class BinarySearchTree
{
    struct Node
    {
        Node* left, * right;
        T data;
        Node(T value) : data(value), left(NULL), right(NULL)
        {
        }
    }* root;


    explicit BinarySearchTree(Node* node) : root(node)
    {
    }


    static Node* ptr_to_node(T* ptr)
    {
        return ptr ? reinterpret_cast<Node*>(reinterpret_cast<Node**>(ptr) - 2) : NULL;
    }

    
    static T* node_to_ptr(Node* node)
    {
        return node ? &(node->data) : NULL;
    }


public:

    BinarySearchTree() : root(NULL)
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



    T* insert(T value)
    {
        if (Node* caret = root)
        {
            Node* added {};
            for (;;)
            {
                if (value == caret->data)
                    break;
                if (value < caret->data)
                {
                    if (caret->left)
                        caret = caret->left;
                    else
                        caret->left = added = new Node(value);
                }
                else
                {
                    if (caret->right)
                        caret = caret->right;
                    else
                        caret->right = added = new Node(value);
                }
            }
            return node_to_ptr(added);
        }
        else
            return node_to_ptr(root = new Node(value));
    }



    void remove(T value)
    {
        Node** to_remove = &root, * trash;

    REMOVE:
        while (*to_remove && (*to_remove)->data != value)
            if (value < (*to_remove)->data)
                to_remove = &((*to_remove)->left);
            else
                to_remove = &((*to_remove)->right);
        if (*to_remove)
        {
            trash = (*to_remove);
            if (!((*to_remove)->left) && !((*to_remove)->right))
            {
                *to_remove = NULL;
            }
            else if (!((*to_remove)->left))
            {
                *to_remove = (*to_remove)->right;
            }
            else if (!((*to_remove)->right))
            {
                *to_remove = (*to_remove)->left;
            }
            else
            {
                trash = ptr_to_node(BinarySearchTree((*to_remove)->right).find_min());
                value = (*to_remove)->data = trash->data;
                to_remove = &((*to_remove)->right);
                goto REMOVE;
            }

            delete trash;
        }
    }


    unsigned int height()
    {
        return root ? (max(BinarySearchTree(root->left).height(), BinarySearchTree(root->right).height()) + 1) : 0;
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
            BinarySearchTree(root->left).print_straight();
            BinarySearchTree(root->right).print_straight();
        }
    }



    void print_centre()
    {
        if (root)
        {
            BinarySearchTree(root->left).print_centre();
            std::cout << root->data << ' ';
            BinarySearchTree(root->right).print_centre();
        }
    }



    void print_reverse()
    {
        if (root)
        {
            BinarySearchTree(root->left).print_reverse();
            BinarySearchTree(root->right).print_reverse();
            std::cout << root->data << ' ';
        }
    }

protected:

    int print_width_part(unsigned int lvl)
    {
        if (root)
        {
            if (lvl)
                return BinarySearchTree(root->left).print_width_part(lvl - 1) + BinarySearchTree(root->right).print_width_part(lvl - 1);
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
                BinarySearchTree(root->left).destroy();
            if (root->right)
                BinarySearchTree(root->right).destroy();
            delete root;
            root = NULL;
        }
    }
};
