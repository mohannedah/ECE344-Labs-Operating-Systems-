#include <iostream>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
using namespace std;

int convertToNum(unsigned char *buffer)
{
    int cntDigits = 0;

    auto isDigit = [&](char digit) -> int
    {
        int ascii = digit - '0';

        if (ascii >= 0 && ascii <= 9)
            return ascii;

        return -1;
    };

    int num = 0;

    for (int i = 0; i < sizeof(buffer); i++)
    {
        int digit = isDigit(buffer[i]);

        if (digit == -1)
            break;

        num *= 10;
        num += digit;
    }
    return num;
};

int getNumDigits(int num)
{
    int cnt = 0;

    while (num)
    {
        cnt += 1;
        num /= 10;
    }
    return cnt;
}

void convertToString(int num, char *destination)
{
    int digitsCnt = getNumDigits(num);
    int currIdx = digitsCnt - 1;
    while (num)
    {
        int lastDigit = num % 10;
        destination[currIdx] = lastDigit + '0';

        num /= 10;
        currIdx -= 1;
    }

    destination[digitsCnt] = '\0';
}