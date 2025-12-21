#pragma once



using byte = unsigned char;



class sudoku_solver
{
    struct
    {
        byte val : 4;
        byte locked : 1;
    } field[9][9] {};


public:

    int is_locked(byte x, byte y)
    {
        --x, --y;
        return (x > 8 || y > 8) ? -1 : field[x][y].locked;
    }



    byte get(byte x, byte y)
    {
        --x, --y;
        return (x > 8 || y > 8) ? -1 : field[x][y].val;
    }



    // returns:
    // 1 - (x;y) was locked before func called but still was set to val
    // 0 - (x;y) was not locked before func called and was set to val
    // -1 - invalid args
    int set_forced(byte x, byte y, byte val, byte lock = 1)
    {
        --x, --y;
        if (x > 8 || y > 8 || val > 9)
            return -1;
        field[x][y].val = val;
        val = field[x][y].locked;
        field[x][y].locked = (lock ? 1 : 0);
        return val;
    }



    // returns:
    // 0-9 - (x;y) was not locked and held that value before func called, (x;y) was set to val
    // -2 - (x;y) was locked and was not changed
    // -1 - invalid args
    int set(byte x, byte y, byte val, byte lock = 0)
    {
        --x, --y;
        if (x > 8 || y > 8 || val > 9)
            return -1;
        if (field[x][y].locked)
            return -2;
        field[x][y].locked = (lock ? 1 : 0);
        lock = field[x][y].val;
        field[x][y].val = val;
        return lock;
    }



    // returns:
    // 1 - (x;y) can hold val
    // 0 - (x;y) can not hold val (repetetions)
    // -1 - invalid args
    int check(byte x, byte y, byte val)
    {
        --x, --y;
        if (x > 8 || y > 8 || !val || val > 9)
            return -1;
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
        if (x > 8 || y > 8)
            return -1;
        if (field[x][y].locked)
            return -2;
        while ((field[x][y].val = ++(field[x][y].val) % 10) && !check(x + 1, y + 1, field[x][y].val));
        return field[x][y].val;
    }



    // returns:
    // 0 - success
    // -1 - no solutions
    int solve()
    {
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



    void clear()
    {
        for (byte x = 1; x <= 9; ++x)
            for (byte y = 1; y <= 9; ++y)
                set_forced(x, y, 0, 0);
    }
};
