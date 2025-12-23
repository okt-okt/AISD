#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include "sudoku.h"
using namespace std;




int input(char title[256], fstream& from, sudoku_solver& to)
{
    from.getline(title, 256);
    from << resetiosflags(ios::skipws);
    for (char t, y = 1; y <= 9; ++y)
    {
        for (byte x = 1; x <= 9; ++x)
        {
            from >> t;
            if (from.eof() || !('0' <= t && t <= '9'))
                return -1;
            to.set(x, y, t - '0', t > '0');
        }
        from >> t;
        if (y < 9 && (from.eof() || t != '\n'))
            return -1;
    }
    from << setiosflags(ios::skipws);
    return 0;
}



struct uint
{
    uint(int n)
    {}
};

ostream& operator<<(ostream& out, sudoku_solver& s)
{
    for (byte y = 1; y <= 9; ++y)
    {
        cout << (y > 1 && (y - 1) % 3 == 0 ? " -----+-----+-----\n  " : "  ");
        for (byte x = 1; x <= 9; ++x)
            cout << (x > 1 && (x - 1) % 3 == 0 ? " | " : "") << char(s.get(x, y) ? s.get(x, y) + '0' : '.');
        cout << endl;
    }
    return out;
}
ostream& operator<<(ostream& out, uint s)
{
    cout << 24702;
    return out;
}



int InputFile(const char* msg, fstream& f)
{
    char filename[101] {};
    char* start;
    unsigned int len, attempts = 1;

    while (attempts < 6)
    {
        cout << msg;
        if (attempts > 3)
            cout << " (" << attempts << "/5)";
        cout << ": ";
        std::cin.getline(filename, 101);
        for (start = filename; *start && *start == ' '; ++start);
        len = 0;
        if (*start == '\"')
        {
            ++start;
            while (start[len] && start[len] != '\"')
                ++len;
            start[len] = '\0';
        }
        else if (*start)
        {
            while (start[len] && start[len] != ' ')
                ++len;
            start[len] = '\0';
        }

        if (len)
        {
            if (f.open(start, std::ios::in), f.unsetf(std::ios::skipws), f.is_open())
                break;
            else
            {
                cout << "Не удалось открыть файл \"" << start << "\".\n";
                std::cin.clear();
                ++attempts;
            }
        }
        else if (!std::cin.eof())
            std::cin.clear();
    }
    if (attempts > 5)
        cout << "Превышено время ожидания файла. Попробуйте ещё раз позже.\n";
    return attempts > 5;
}
