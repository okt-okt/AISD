#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "bintre.h"
#include <sstream>
#include <fstream>
#include <random>
#include <unordered_set>
using std::cout;



int main()
{
    SetConsoleOutputCP(1251);

#define p(ptr) ((ptr) ? (std::stringstream() << *(ptr)).str() : std::string("NULL"))
#define test_print(tree) cout << "Дерево (высота=" << tree.height() << ", макс.=" << p(tree.find_max()) << ", мин.=" << p(tree.find_min()) << "):\n"; tree.print(); cout << std::endl;
#define test_remove(tree, value) tree.remove(value); cout << #value " удалено:\n"; test_print(tree)
#define test_search(tree, value) cout << value << (tree.find(value) ? " нашлось\n" : " не нашлось\n");
    
    cout << "= BST ==========================================================\n\n";
    {
        BinaryTree<unsigned int> TreeBST(BST);
        const unsigned int vv[] = { 45, 78, 11, 11, 12, 56, 49, 89, 10, 52, 42, 88, 13, 100 };
        for (auto v : vv)
            TreeBST.insert(v);
        test_print(TreeBST);
        test_search(TreeBST, 10);
        test_search(TreeBST, 1);
        test_remove(TreeBST, 11);
        test_remove(TreeBST, 45);
        test_remove(TreeBST, 13);
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
    
    cout << "= AVL ==========================================================\n\n";
    {
        BinaryTree<unsigned int> TreeAVL(AVL);
        const unsigned int vv[] = { 45, 78, 11, 11, 12, 56, 49, 89, 10, 52, 42, 88, 13, 100 };
        for (auto v : vv)
            TreeAVL.insert(v);
        test_print(TreeAVL);
        test_search(TreeAVL, 10);
        test_search(TreeAVL, 1);
        test_remove(TreeAVL, 11);
        test_remove(TreeAVL, 45);
        test_remove(TreeAVL, 42);
        TreeAVL.destroy();
    }
    cout << "\n\n";
    
    cout << "= RnB ==========================================================\n\n";
    {
        BinaryTree<unsigned int> RNBTree(RNB);
        const unsigned int vv[] = { 45, 78, 11, 11, 12, 56, 49, 89, 10, 52, 42, 88, 13, 100 };
        for (auto v : vv)
            RNBTree.insert(v);
        test_print(RNBTree);
        test_search(RNBTree, 10);
        test_search(RNBTree, 1);
        test_remove(RNBTree, 11);
        test_remove(RNBTree, 45);
        test_remove(RNBTree, 42);
        test_remove(RNBTree, 78);
        test_remove(RNBTree, 52);
        test_remove(RNBTree, 56);
        test_remove(RNBTree, 88);
        test_remove(RNBTree, 100);
        test_remove(RNBTree, 10);
        test_remove(RNBTree, 12);
        test_remove(RNBTree, 49);
        test_remove(RNBTree, 13);
        test_remove(RNBTree, 89);
        RNBTree.destroy();
    }
    cout << "\n\n";

    cout << "= СТАТИСТИКА ДЛЯ ЭКСПЕРИМЕНТАЛЬНЫХ ДАННЫХ ====================\n";
    for (unsigned int t;;)
    {
        cout << "\nВведите тип дерева для формирования статистики (1-BST, 2-AVL, 3-RNB) или 0 для выхода: ";
        do
            std::cin >> t;
        while (!(t <= 3));
        if (t == 0)
            break;
        BinaryTreeType types[] = { BST, AVL, RNB };
        const char* filenames[] = { "stat_BST.txt", "stat_AVL.txt", "stat_RNB.txt", "stat_BST_monoton.txt", "stat_AVL_monoton.txt", "stat_RNB_monoton.txt",
            "stat_BST_short.txt", "stat_AVL_short.txt", "stat_RNB_short.txt", "stat_BST_monoton_short.txt", "stat_AVL_monoton_short.txt", "stat_RNB_monoton_short.txt", };
        std::ofstream out(filenames[t - 1]), out_short(filenames[t - 1 + 6]);
        if (!out.is_open() || !out_short.is_open())
        {
        FAIL_FOPEN:
            cout << "Не удалось открыть файл!\n";
            break;
        }
        BinaryTree<unsigned int> Tree(types[t - 1]);
        std::mt19937 generate(std::random_device {}());
        std::uniform_int_distribution<unsigned int> dist(0, 50'000'000);
        std::unordered_set<unsigned int> used;
        used.reserve(100'000);
        unsigned int n = 0, k, last = -1;
        while (n < 56'000)
        {
            k = dist(generate);
            if (used.insert(k).second)
            {
                Tree.insert(k);
                ++n;
                out << n << " " << Tree.height() << "\n";
                if (Tree.height() != last)
                    out_short << n << " " << (last = Tree.height()) << "\n";
                if (n % 100 == 0)
                    cout << "  Всего вставлено " << n << " значений...\n";
            }
        }
        out.close();
        out.open(filenames[t - 1 + 3]);
        out_short.close();
        out_short.open(filenames[t - 1 + 9]);
        if (!out.is_open() || !out_short.is_open())
            goto FAIL_FOPEN;
        Tree.destroy();
        last = -1;
        for (n = 1; n <= 56'000; ++n)
        {
            Tree.insert(n);
            out << n << " " << Tree.height() << "\n";
            if (Tree.height() != last)
                out_short << n << " " << (last = Tree.height()) << "\n";
            if (n % 100 == 0)
                cout << "  Монотонно вставлено " << n << " значений...\n";
        }
        out.close();
        out_short.close();
        Tree.destroy();
    }

    return 0;
}