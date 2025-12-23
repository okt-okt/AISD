#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>
using namespace std;



using byte = unsigned char;


enum sudoku_solver_method : byte
{
    iteration,
    limited,
    DLX,
};


class sudoku_solver
{
    struct
    {
        unsigned int val : 4;
        unsigned int locked : 1;
        unsigned int count_impossible : 4;
        unsigned int impossible : 2*9;
    } field[9][9] {};

public:

    const sudoku_solver_method method;

    sudoku_solver(sudoku_solver_method method = iteration) : method(method)
    {
    }



    inline int is_locked(byte x, byte y)
    {
        return field[x - 1][y - 1].locked;
    }



    byte get(byte x, byte y)
    {
        return field[x - 1][y - 1].val;
    }

private:

    byte add_impossible(byte x_1, byte y_1, byte val)
    {
        if (get_impossible(x_1, y_1, val) != 3 && (((field[x_1][y_1].impossible += 1 << (2 * val - 2)) >> (2 * val - 2)) & 3) == 1)
            ++(field[x_1][y_1].count_impossible);
        return field[x_1][y_1].count_impossible;
    }



    byte remove_impossible(byte x_1, byte y_1, byte val)
    {
        if (get_impossible(x_1, y_1, val) && (((field[x_1][y_1].impossible -= 1 << (2 * val - 2)) >> (2 * val - 2)) & 3) == 0)
            --(field[x_1][y_1].count_impossible);
        return field[x_1][y_1].count_impossible;
    }



    byte get_impossible(byte x_1, byte y_1, byte val)
    {
        return (field[x_1][y_1].impossible >> (2 * val - 2)) & 3;
    }



    byte get_last_possible(byte x_1, byte y_1)
    {
        byte result = 1;
        while (get_impossible(x_1, y_1, result))
            ++result;
        return result;
    }

public:

    // returns:
    // //0-9 - (x;y) was not locked and held that value before func called, (x;y) was set to val
    // -2 - (x;y) was locked and was not changed
    // -1 - invalid args
    // -2 - [for method=limited] set value creates impossible combination
    int set(byte x, byte y, byte val, byte lock = 0)
    {
        --x, --y;
        //if (x > 8 || y > 8 || val > 9)
        //    return -1;
        if (field[x][y].locked)
            return -2;
        field[x][y].locked = (lock ? 1 : 0);
        lock = field[x][y].val;
        field[x][y].val = val;

        if (method == limited)
        {
            byte a, b, q, r;
            if (!method && lock)
            {
                for (a = 0, b = y; a < 9; ++a)
                    if (!(field[a][b].locked) && a != x)
                    {
                        q = field[a][b].count_impossible;
                        if (remove_impossible(a, b, lock) == 7 && q == 8)
                            set(a + 1, b + 1, 0, field[x][y].locked);
                    }
                for (a = x, b = 0; b < 9; ++b)
                    if (!(field[a][b].locked) && b != y)
                    {
                        q = field[a][b].count_impossible;
                        if (remove_impossible(a, b, lock) == 7 && q == 8)
                            set(a + 1, b + 1, 0, field[x][y].locked);
                    }
                for (a = x / 3 * 3; a < (x / 3 + 1) * 3; ++a)
                    for (b = y / 3 * 3; b < (y / 3 + 1) * 3; ++b)
                        if (!(field[a][b].locked) && !(a == x && b == y))
                        {
                            q = field[a][b].count_impossible;
                            if (remove_impossible(a, b, lock) == 7 && q == 8)
                                set(a + 1, b + 1, 0, field[x][y].locked);
                        }
            }
            if (val)
            {
                for (a = 0, b = y; a < 9; ++a)
                    if (!(field[a][b].locked) && a != x)
                    {
                        q = field[a][b].count_impossible;
                        r = add_impossible(a, b, val);
                        if (r == 8 && q == 7)
                            set(a + 1, b + 1, get_last_possible(a, b), field[x][y].locked);
                        else if (r == 9 && q == 8)
                            return -2;
                    }
                for (a = x, b = 0; b < 9; ++b)
                    if (!(field[a][b].locked) && b != y)
                    {
                        q = field[a][b].count_impossible;
                        r = add_impossible(a, b, val);
                        if (r == 8 && q == 7)
                            set(a + 1, b + 1, get_last_possible(a, b), field[x][y].locked);
                        else if (r == 9 && q == 8)
                            return -2;
                    }
                for (a = x / 3 * 3; a < (x / 3 + 1) * 3; ++a)
                    for (b = y / 3 * 3; b < (y / 3 + 1) * 3; ++b)
                        if (!(field[a][b].locked) && !(a == x && b == y))
                        {
                            q = field[a][b].count_impossible;
                            r = add_impossible(a, b, val);
                            if (r == 8 && q == 7)
                                set(a + 1, b + 1, get_last_possible(a, b), field[x][y].locked);
                            else if (r == 9 && q == 8)
                                return -2;
                        }
            }
        }
        return val;
    }



    // returns:
    // 1 - (x;y) can hold val
    // 0 - (x;y) can not hold val (repetetions)
    // -1 - invalid args
    int check(byte x, byte y, byte val)
    {
        --x, --y;
        byte a, b;
        for (a = 0, b = y; a < 9; ++a)
            if (a != x && field[a][b].val == val)
                return 0;
        for (a = x, b = 0; b < 9; ++b)
            if (b != y && field[a][b].val == val)
                return 0;
        for (a = x / 3 * 3; a < (x / 3 + 1) * 3; ++a)
            for (b = y / 3 * 3; b < (y / 3 + 1) * 3; ++b)
                if (!(a == x && b == y) && field[a][b].val == val)
                    return 0;
        return 1;
    }



    // returns:
    // 1-9 - (x;y) was changed to that value
    // 0 - (x;y) cycled through all possible values for it and was set to 0
    // -2 - (x;y) was locked and was not changed
    // -1 - invalid args
    int set_next(byte x, byte y)
    {
        --x, --y;
        //if (x > 8 || y > 8)
        //    return -1;
        if (field[x][y].locked)
            return -2;
        if (!method && method == limited)
        {
            byte val = field[x][y].val;
            while ((val = ++(val) % 10) && get_impossible(x, y, val));
            return (val = set(x + 1, y + 1, val)) == -2 ? 0 : val;
        }
        while ((field[x][y].val = ++(field[x][y].val) % 10) && !check(x + 1, y + 1, field[x][y].val));
        return field[x][y].val;
    }



    // returns:
    // 0 - success
    // -1 - no solutions
    int solve()
    {
        if (method == DLX)
        {
            DLXconstructMatrix();
            return 0;
        }

        byte x = 1, y = 1;
        for (;;)
        {
            if (set_next(x, y) == 0)
            {
                do
                    if (x == 1)
                    {
                        if (y == 1)
                            return -1;
                        x = 9, --y;
                    }
                    else
                        --x;
                while (is_locked(x, y));
            }
            else
            {
                if (x == 9)
                {
                    if (y == 9)
                        break;
                    x = 1, ++y;
                }
                else
                    ++x;
            }
        }

        return 0;
    }

private:

    struct DLXNode
    {
        DLXNode* left, * right, * up, * down, * col;
        int name, size;
    }* soluts[500] {};


    void DLXcover(DLXNode* col)
    {
        col->right->left = col->left;
        col->left->right = col->right;
        for (DLXNode* i = col->down; i != col; i = i->down)
            for (DLXNode* j = i->right; j != i; j = j->right)
            {
                j->down->up = j->up;
                j->up->down = j->down;
                --j->col->size;
            }
    }


    void DLXuncover(DLXNode* col)
    {
        for (DLXNode* i = col->up; i != col; i = i->up)
            for (DLXNode* j = i->left; j != i; j = j->left)
            {
                ++j->col->size;
                j->down->up = j;
                j->up->down = j;
            }
        col->right->left = col;
        col->left->right = col;
    }

    DLXNode* DLXfindBest(DLXNode* head)
    {
        DLXNode* j, * c = NULL;
        int s = 500000;
        for (j = head->right; j != head; j = j->right)
            if (j->size < s)
            {
                s = j->size;
                c = j;
            }
        return c;
    }

    void DLXsearch(int k, DLXNode* head)
    {
        DLXNode* r, * j;
        if (head->right == head)
            return;
        DLXNode* c = DLXfindBest(head);
        DLXcover(c);
        for (r = c->down; r != c; r = r->down)
        {
            soluts[k] = r;
            for (j = r->right; j != r; j = j->right)
                DLXcover(j->col);
            DLXsearch(k + 1, head);
            r = soluts[k];
            c = r->col;
            for (j = r->left; j != r; j = j->left)
                DLXuncover(j->col);
        }
        DLXuncover(c);
    }

    void DLXinitCols(int n, DLXNode* head)
    {
        for (int i = 0; i < n; ++i)
        {
            head[i].name = i - 1;
            head[i].up = head + i;
            head[i].down = head + i;
            head[i].size = 0;
            head[i].col = NULL;
            if (i == 0)
                head[i].left = head + (n - 1);
            else
                head[i].left = head + (i - 1);
            if (i == n - 1)
                head[i].right = head;
            else
                head[i].right = head + (i + 1);
        }
    }

    void DLXinitRowLinks(DLXNode* row, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            if (i == 0)
                row[i].left = row + (n - 1);
            else
                row[i].left = row + (i - 1);
            if (i == n - 1)
                row[i].right = row;
            else
                row[i].right = row + (i + 1);
        }
    }

    void DLXplaceRow(DLXNode* row, int n, int pos[], DLXNode* head)
    {
        for (int col, i = 0; i < n; ++i)
        {
            col = pos[i] + 1;
            row[i].up = head[col].up;
            row[i].down = head + col;
            row[i].col = head + col;
            row[i].up->down = row + i;
            head[col].up = row + i;
            ++head[col].size;
        }
    }

    int sudokuRowCol(int row, int col)
    {
        return row * 9 + col;
    }

    int sudokuRowDig(int row, int val)
    {
        return 80 + row * 9 + val;
    }

    int sudokuColDig(int col, int val)
    {
        return 161 + col * 9 + val;
    }

    int sudokuBoxDig(int row, int col, int val)
    {
        int box = (col / 3) + (row / 3) * 3;
        return 242 + box * 9 + val;
    }

    void DLXplaceSolution()
    {
        int i, row, col, val;
        DLXNode* n;
        for (i = 0; i < 81; ++i)
        {
            n = soluts[i];
            while (n->col->name > 80)
                n = n->right;
            row = n->col->name / 9;
            col = n->col->name % 9;
            n = n->right;
            val = (n->col->name - 81) % 9 + 1;
            field[row][col].val = val;
        }
    }

    void DLXconstructMatrix()
    {
        int row, col, matRowCount = 0, placement[4], i;
        DLXNode matCols[325], matRows[729][4];
        DLXinitCols(325, matCols);
        for (row = 0; row < 9; ++row)
            for (col = 0; col < 9; ++col)
                if (field[row][col].val)
                {
                    DLXinitRowLinks(matRows[matRowCount], 4);
                    placement[0] = sudokuRowCol(row, col);
                    placement[1] = sudokuRowDig(row, field[row][col].val);
                    placement[2] = sudokuColDig(col, field[row][col].val);
                    placement[3] = sudokuBoxDig(row, col, field[row][col].val);
                    DLXplaceRow(matRows[matRowCount], 4, placement, matCols);
                    ++matRowCount;
                }
                else
                    for (i = 1; i < 10; ++i)
                    {
                        DLXinitRowLinks(matRows[matRowCount], 4);
                        placement[0] = sudokuRowCol(row, col);
                        placement[1] = sudokuRowDig(row, i);
                        placement[2] = sudokuColDig(col, i);
                        placement[3] = sudokuBoxDig(row, col, i);
                        DLXplaceRow(matRows[matRowCount], 4, placement, matCols);
                        ++matRowCount;
                    }
        DLXsearch(0, matCols);
        DLXplaceSolution();
    }

public:

    void clear()
    {
        *this = { method };
    }
};
