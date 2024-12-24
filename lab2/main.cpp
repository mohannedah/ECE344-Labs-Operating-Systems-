#include <iostream>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include "utils.cpp"
using namespace std;

int count_processes = 0, max_process_count = 0;

struct ProcessInfo
{
    int process_id;
    char *process_name;
    int process_status;

    ProcessInfo(int process_id, char *process_name, int process_status)
    {
        this->process_id = process_id;
        this->process_name = process_name;
        this->process_status = process_status;
    };
};

ProcessInfo **arr;

void init_child_process(int pid, int id, int status, char *process_name)
{
    ProcessInfo *process_info = (ProcessInfo *)malloc(sizeof(ProcessInfo));
    process_info->process_id = pid;
    process_info->process_name = (char *)malloc(sizeof(process_name));
    strcpy(process_info->process_name, process_name);
    process_info->process_status = status;
    arr[id] = process_info;
};

void handle_signal(int seg)
{

    if (seg == SIGCHLD)
    {
        // This indicates that a child process has been terminated and sent its parent a signal for it to notice about its terminating factor.
        // Now I need to know which process of the spawned child processes by the parent that has been terminated to wait upon it and update its status accordingly.
        int status, pid;
        while (pid = wait(&status) != -1)
        {
            bool found;
            for (int i = 0; i < count_processes; i++)
            {
                if (pid == arr[i]->process_id)
                {
                    arr[i]->process_status = status;
                    found = true;
                    break;
                }
            }

            if (!found) // This must have been an orphan process that has been reparented to the root process
            {
                init_child_process(pid, count_processes++, status, "unknown");
            }
        };
    }
    else
    {
        exit(0);
    }
}

void ssp_init()
{
    prctl(PR_SET_CHILD_SUBREAPER, 1);
    signal(SIGTERM, handle_signal);
    signal(SIGCHLD, handle_signal);

    int fd = open("/proc/sys/kernel/pid_max", O_RDONLY);

    if (fd == -1)
    {
        cerr << strerror(errno) << endl;
        return;
    }
    unsigned char buffer[16];

    int one = read(fd, buffer, sizeof(buffer));

    max_process_count = convertToNum(buffer);

    arr = (ProcessInfo **)malloc(sizeof(ProcessInfo *) * max_process_count);
    close(fd);
};

void exec_child(char *const *argv, int fd0, int fd1, int fd2)
{
    int file_descriptors[] = {fd0, fd1, fd2};
    for (int i = 0; i < sizeof(file_descriptors); i++)
        dup2(file_descriptors[i], i);

    int p_id = getpid();
    char folderPath[strlen("/proc/") + 10 + strlen("/fd")];

    char stringifiedNum[11];

    convertToString(p_id, stringifiedNum);

    strcpy(folderPath, "/proc/");
    strcat(folderPath, stringifiedNum);
    strcat(folderPath, "/fd");

    DIR *dir = opendir(folderPath);

    dirent *startPtr;

    while (startPtr = readdir(dir))
    {
        if (startPtr->d_type == DT_LNK)
        {
            if (atoi(startPtr->d_name) >= 3 && atoi(startPtr->d_name) != dirfd(dir))
            {
                close(atoi(startPtr->d_name));
            }
        }
    };

    closedir(dir);
    execvp(argv[0], argv);
}

int ssp_create(char *const *argv, int fd0, int fd1, int fd2)
{
    count_processes += 1;
    int p_id = fork();

    if (p_id == 0)
    {
        exec_child(argv, fd0, fd1, fd2);
        return -1;
    }
    else if (p_id > 0)
    {
        ProcessInfo *processInfoPtr = (ProcessInfo *)malloc(sizeof(ProcessInfo));
        processInfoPtr->process_name = argv[0];
        processInfoPtr->process_id = p_id;
        processInfoPtr->process_status = -1;
        arr[count_processes - 1] = processInfoPtr;
        return count_processes - 1;
    }
    else
    {
        cerr << strerror(errno) << endl;
        exit(-1);
    }
    return count_processes - 1;
};

void ssp_print()
{
    cout << endl;
    cout << "PID" << "    " << "Process Name" << "    " << "Process Status" << endl;

    for (int i = 0; i < count_processes; i++)
    {
        cout << arr[i]->process_id << "    " << arr[i]->process_name << "    " << arr[i]->process_status << endl;
    }
}

int ssp_get_status(int p_id)
{
    return arr[p_id]->process_status;
}

void ssp_wait()
{
    for (int i = 0; i < count_processes; i++)
    {
        int newStatus;

        int err = waitpid(arr[i]->process_id, &newStatus, WNOHANG);

        if (err == -1)
        {
            cerr << strerror(errno) << endl;
            exit(-1);
        };
        arr[i]->process_status = newStatus;
    }
}
