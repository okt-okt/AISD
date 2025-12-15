#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "bintre.h"
#include <sstream>
using std::cout;



int main()
{
    SetConsoleOutputCP(1251);

#define p(ptr) ((ptr) ? (std::stringstream() << *(ptr)).str() : std::string("NULL"))
    
    cout << "= BST ======================================\n\n";
    {
        BinaryTree<unsigned int> TreeBST(BST);
        const unsigned int vv[] = { 45, 78, 11, 11, 12, 56, 49, 89, 10, 52, 42, 88, 13, 100 };
        for (auto v : vv)
            TreeBST.insert(v);
        cout << "Дерево (высота=" << TreeBST.height() << ", макс.=" << p(TreeBST.find_max()) << ", мин.=" << p(TreeBST.find_min()) << "):\n";
        TreeBST.print();
        cout << '\n' << 10 << (TreeBST.find(10) ? " нашлось\n" : " не нашлось\n");
        cout << 1 << (TreeBST.find(1) ? " нашлось\n" : " не нашлось\n");
        TreeBST.remove(11);
        cout << "\n11 удалено:\n";
        cout << "Дерево (высота=" << TreeBST.height() << ", макс.=" << p(TreeBST.find_max()) << ", мин.=" << p(TreeBST.find_min()) << "):\n";
        TreeBST.print();
        TreeBST.remove(45);
        cout << "\n45 удалено:\n";
        cout << "Дерево (высота=" << TreeBST.height() << ", макс.=" << p(TreeBST.find_max()) << ", мин.=" << p(TreeBST.find_min()) << "):\n";
        TreeBST.print();
        TreeBST.remove(13);
        cout << "\n13 удалено:\n";
        cout << "Дерево (высота=" << TreeBST.height() << ", макс.=" << p(TreeBST.find_max()) << ", мин.=" << p(TreeBST.find_min()) << "):\n";
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
        cout << "Дерево (высота=" << TreeAVL.height() << ", макс.=" << p(TreeAVL.find_max()) << ", мин.=" << p(TreeAVL.find_min()) << "):\n";
        TreeAVL.print();
        cout << '\n' << 10 << (TreeAVL.find(10) ? " нашлось\n" : " не нашлось\n");
        cout << 1 << (TreeAVL.find(1) ? " нашлось\n" : " не нашлось\n");
        TreeAVL.remove(11);
        cout << "\n11 удалено:\n";
        cout << "Дерево (высота=" << TreeAVL.height() << ", макс.=" << p(TreeAVL.find_max()) << ", мин.=" << p(TreeAVL.find_min()) << "):\n";
        TreeAVL.print();
        TreeAVL.remove(45);
        cout << "\n45 удалено:\n";
        cout << "Дерево (высота=" << TreeAVL.height() << ", макс.=" << p(TreeAVL.find_max()) << ", мин.=" << p(TreeAVL.find_min()) << "):\n";
        TreeAVL.print();
        TreeAVL.remove(42);
        cout << "\n42 удалено:\n";
        cout << "Дерево (высота=" << TreeAVL.height() << ", макс.=" << p(TreeAVL.find_max()) << ", мин.=" << p(TreeAVL.find_min()) << "):\n";
        TreeAVL.print();
        TreeAVL.destroy();
    }
    cout << "\n\n";
    
    cout << "= RnB ======================================\n\n";
    {
        BinaryTree<unsigned int> RNBTree(RNB);
        const unsigned int vv[] = { 45, 78, 11, 11, 12, 56, 49, 89, 10, 52, 42, 88, 13, 100 };
        for (auto v : vv)
            RNBTree.insert(v);
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        cout << '\n' << 10 << (RNBTree.find(10) ? " нашлось\n" : " не нашлось\n");
        cout << 1 << (RNBTree.find(1) ? " нашлось\n" : " не нашлось\n");
        RNBTree.remove(11);
        cout << "\n11 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(45);
        cout << "\n45 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(42);
        cout << "\n42 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(78);
        cout << "\n78 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(52);
        cout << "\n52 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(56);
        cout << "\n56 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(88);
        cout << "\n88 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(100);
        cout << "\n100 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(10);
        cout << "\n10 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(12);
        cout << "\n12 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(49);
        cout << "\n49 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(13);
        cout << "\n13 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.remove(89);
        cout << "\n89 удалено:\n";
        cout << "Дерево (высота=" << RNBTree.height() << ", макс.=" << p(RNBTree.find_max()) << ", мин.=" << p(RNBTree.find_min()) << "):\n";
        RNBTree.print();
        RNBTree.destroy();
    }

    return 0;
}