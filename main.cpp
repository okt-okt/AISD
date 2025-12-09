#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "bintre.h"
using std::cout;



int main()
{
    SetConsoleOutputCP(1251);
    
    cout << "= BST ======================================\n\n";
    {
        BinaryTree<unsigned int> TreeBST(BST);
        const unsigned int vv[] = { 45, 78, 11, 11, 12, 56, 49, 89, 10, 52, 42, 88, 13, 100 };
        for (auto v : vv)
            TreeBST.insert(v);
        cout << "Дерево (высота=" << TreeBST.height() << ", макс.=" << *TreeBST.find_max() << ", мин.=" << *TreeBST.find_min() << "):\n";
        TreeBST.print();
        cout << '\n' << 10 << (TreeBST.find(10) ? " нашлось\n" : " не нашлось\n");
        cout << 1 << (TreeBST.find(1) ? " нашлось\n" : " не нашлось\n");
        TreeBST.remove(11);
        cout << "\n11 удалено:\n";
        TreeBST.print();
        TreeBST.remove(45);
        cout << "\n45 удалено:\n";
        TreeBST.print();
        cout << "\nПрямой обход: ";
        TreeBST.print_straight();
        cout << "\nЦентрированный обход: ";
        TreeBST.print_centre();
        cout << "\nОбратный обход: ";
        TreeBST.print_reverse();
        cout << "\nОбход в ширину: ";
        TreeBST.print_width();
        TreeBST.destroy();
    }
    cout << "\n\n";
    
    cout << "= AVL ======================================\n\n";
    {
        BinaryTree<unsigned int> TreeAVL(AVL);
        const unsigned int vv[] = { 45, 78, 11, 11, 12, 56, 49, 89, 10, 52, 42, 88, 13, 100 };
        for (auto v : vv)
            TreeAVL.insert(v);
        cout << "Дерево (высота=" << TreeAVL.height() << ", макс.=" << *TreeAVL.find_max() << ", мин.=" << *TreeAVL.find_min() << "):\n";
        TreeAVL.print();
        cout << '\n' << 10 << (TreeAVL.find(10) ? " нашлось\n" : " не нашлось\n");
        cout << 1 << (TreeAVL.find(1) ? " нашлось\n" : " не нашлось\n");
        TreeAVL.remove(11);
        cout << "\n11 удалено:\n";
        TreeAVL.print();
        TreeAVL.remove(45);
        cout << "\n45 удалено:\n";
        TreeAVL.print();
        TreeAVL.destroy();
    }
    cout << "\n\n";
    
    cout << "= RnB ======================================\n\n";
    {
       // BinarySearchTree<unsigned int> TreeBST;
    }

    return 0;
}