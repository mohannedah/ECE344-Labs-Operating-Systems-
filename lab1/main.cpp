#include <iostream>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
using namespace std;

int extractName(unsigned char *buffer, unsigned char *output)
{
    int idx = 6;
    while (buffer[idx] != '\n' && buffer[idx] != '\n')
    {
        output[idx - 6] = (unsigned char)buffer[idx];
        idx += 1;
    }
    return idx - 6;
};

void print(char *directoryName)
{

    char path[strlen("/proc/") + strlen(directoryName) + strlen("/status")];

    // cout << sizeof(path) << endl;
    strcpy(path, "/proc/");
    strcat(path, directoryName);
    strcat(path, "/status");

    cout << path << endl;
    int fd = open(path, O_RDONLY);

    unsigned char buffer[4096];

    unsigned char *startPos = buffer;
    int totalBytesRead = 0;

    while (read(fd, startPos, 1) == 1)
    {
        startPos += 1;
    }

    unsigned char name[150];
    int len = extractName(buffer, name);

    char nameCopy[len];

    for (int i = 0; i < len; i++)
        nameCopy[i] = name[i];

    cout << directoryName << "     ";
    for (int i = 0; i < len; i++)
    {
        putchar(nameCopy[i]);
    }
    close(fd);
};

int main()
{

    DIR *dir = opendir("/proc");

    dirent *startPtr;

    cout << "PID" << "     " << "Name" << endl;

    while (startPtr = readdir(dir))
    {
        char *name = startPtr->d_name;

        bool isPid = 1;
        for (int i = 0; i < strlen(name); i++)
        {
            int asciVal = (int)(name[i] - '0');
            if (asciVal > 9 || asciVal < 0)
            {
                isPid = 0;
                break;
            }
        };

        if (isPid)
        {
            print(name);
            cout << "\n";
        }
    }

    closedir(dir);
}