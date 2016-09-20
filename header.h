#ifndef HEADER
#define HEADER

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<string.h>
#include<sys/utsname.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/stat.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/resource.h>

#define BUFFER_SIZE 1024

char home_dir[BUFFER_SIZE];
int and_flag;

char *command_list[] = {"echo",
    "pwd",
    "cd",
    "exit",
    "pinfo",
    "jobs",
    "kjob",
    "overkill",
    "quit",
    "fg",
    '\0'};

#endif
