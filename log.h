#pragma once

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdarg.h"
#include "unistd.h"
#include "socket.h"

#define MAXLEN (2048)
#define MAXFILEPATH (512)
#define MAXFILENAME (50)
typedef enum{
    ERROR_1=-1,
    ERROR_2=-2,
    ERROR_3=-3
}ERROR0;


typedef enum{
    NONE=0,
    INFO=1,
    DEBUG=2,
    WARN=3,
    ERROR=4,
    ALL=255
}LOGLEVEL;

typedef struct log{
    char logtime[20];
    char filepath[MAXFILEPATH];
    FILE *logfile;
}LOG;

typedef struct logseting{
    char filepath[MAXFILEPATH];
    unsigned int maxfilelen;
    unsigned char loglevel;
}LOGSET;

int LogWrite(unsigned char loglevel,char *fromat,...);