#define _MAIN_FILE_
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "io.h"



class ThreadPool* active_tp;

class ThreadPool
{
private:
    std::vector<std::thread> workers;
    unsigned long long done = 0x3'FFFF'FFFF'FFFF; // 50 bits active

    std::mutex access;
    unsigned int last = 0;

public:

    static void worker()
    {
        unsigned int c;
        for (;;)
        {
            active_tp->access.lock();
            if (active_tp->last == 50)
            {
                active_tp->access.unlock();
                break;
            }
            c = active_tp->last++;
            active_tp->access.unlock();

            active_tp->s[c].solve();

            active_tp->access.lock();
            active_tp->done ^= 1ull << c;
            active_tp->access.unlock();
        }
    }

    sudoku_solver* s;

    ThreadPool(size_t threads, sudoku_solver* tasks) : s(tasks)
    {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back(thread(worker));
        active_tp = this;
    }


    void wait_all()
    {
        for (auto& i : workers)
            i.join();
    }
};



int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    fstream f;
    char title[256];
    unsigned int n = 0, t, result = 0;
    std::chrono::high_resolution_clock::time_point times[2];
    static const char* methods[] = { "перебора", "ограниченного перебора", "Dancing Links", "многопоточного перебора" };

    int hr = InputFile("Введите путь к файлу судоку", f);

    for (; !hr;)
    {
        f.clear();
        f.seekg(0, ios::beg);
        n = result = 0;

        for (t = 5; t > 4; cin >> t)
            cout << "\nВведите метод поиска решения (1-перебор, 2-ограниченный, 3-DLX, 4-многопоточный) или 0 для выхода: ";
        if (t == 0)
            break;

        times[0] = std::chrono::high_resolution_clock::now();
        if (t == 4)
        {
            sudoku_solver s[50];
            for (n = 0; !hr && n < 50; ++n)
                hr = input(title, f, s[n]);
            ThreadPool t(4, s);
            t.wait_all();
            for (n = 0; !hr && n < 50; ++n)
                result += s[n].get(1, 1) * 100 + s[n].get(2, 1) * 10 + s[n].get(3, 1);
        }
        else
            for (sudoku_solver s(sudoku_solver_method(t - 1)); !hr && !f.eof(); s.clear())
            {
                if (!(hr = input(title, f, s)))
                    ++n;
                if (!hr && !(hr = s.solve()))
                    result += s.get(1, 1) * 100 + s.get(2, 1) * 10 + s.get(3, 1);
            }
        times[1] = std::chrono::high_resolution_clock::now();

        if (!hr)
            cout << n << " судоку решено по алгоритму " << methods[t - 1] << " за " << std::chrono::duration_cast<std::chrono::milliseconds>(times[1] - times[0]).count() << " мс.\n"
            << "Сумма чисел из трёх цифр в левом верхнем углу: " << uint(result) << ".\n";
    }

    if (hr)
           cout << "Что-то пошло не так, попробуйте ещё раз позже.\nSomething went wrong and/or no solution was found!\n";
    f.close();
    return hr;
}
