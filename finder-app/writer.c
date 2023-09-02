// 1. Accepts the following arguments: the first argument is a full path to a file (including filename) 
//    on the filesystem, referred to below as "writefile"; 
//    the second argument is a text string which will be written within this file, referred to below as "writestr"
//
// 2. Exits with value 1 error and print statements if any of the arguments above were not specified
//
// 3. Creates a new file with name and path writefile with content writestr, 
//    overwriting any existing file. Exits with value 1 and error print statement 
//    if the file could not be created.
//
// 4. Setup syslog logging for your utility using the LOG_USER facility.
//
// 5. Use the syslog capability to write a message 鈥淲riting <string> to <file>鈥?
//    where <string> is the text string written to file (second argument) and 
//    <file> is the file created by the script.  
//    This should be written with LOG_DEBUG level.
//
// 6. Use the syslog capability to log any unexpected errors with LOG_ERR level.

#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// char* id = "wuzy";


int main(int argc, char* argv[]){ //argv[0] is writefile, argv[1] is writestr

    openlog("wuzy", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    if(argc < 3){
        syslog(LOG_ERR, "There should be two params");
        exit(1);
    }
    char* writerstr = argv[2];
    char* writerfile = argv[1];

    FILE *fp;
    fp = fopen(writerfile, "w+"); //overwriting any existing file

    if(fp == NULL){
        syslog(LOG_ERR, "File %s is not created", writerfile);
        exit(1);
    }

    fprintf(fp, "%s", writerstr);

    syslog(LOG_DEBUG, "Writing %s to %s", writerstr, writerfile);

    closelog();

    return 0;
}

