#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "sudoku.h"
using namespace std;




int input(fstream& from, sudoku_solver& to)
{
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
    return 0;
}



ostream& operator<<(ostream& out, sudoku_solver& s)
{
    for (byte y = 1; y <= 9; ++y)
    {
        cout << (y > 1 && (y - 1) % 3 == 0 ? " -----+-----+-----\n " : "  ");
        for (byte x = 1; x <= 9; ++x)
            cout << (x > 1 && (x - 1) % 3 == 0 ? " | " : "") << char(s.get(x, y) ? s.get(x, y) + '0' : '.');
        cout << "\n";
    }
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




int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    fstream f;
    sudoku_solver s {};

    int hr = InputFile("Введите путь к файлу судоку", f);

    if (!hr && !(hr = input(f, s)))
        cout << "Считанное судоку:\n\n" << s << endl;
    
    if (!hr && !(hr = s.solve()))
        cout << "Решённое судоку:\n\n" << s << endl;
    else
        cout << "Something went wrong and/or no solution was found!\n";

    return 0;
}
