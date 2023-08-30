#include "systemcalls.h"
#include <stdlib.h> //system
#include <unistd.h> //fork
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> // O_WRONLY|O_TRUNC|O_CREAT
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    // int status = system(cmd);
    // int a;
    // // maybe to check the termination status
    // int childPID = wait(&a);
    // if(status == childPID){
    //     return true;
    // }
    // else{
    //     return false;
    // }
    return system(cmd) == 0;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    pid_t childPID = fork();
    // int status;
    // bool returnVal;

    if(childPID == -1){
        return false;
    }
    else if(childPID == 0){
        if (execv(command[0], command) == -1){
            exit(1);
        }
    }
    else{
        pid_t pidResult;
        int status;
        
        pidResult = waitpid(childPID, &status, 0);

        if(pidResult == -1){
            return false;
        }
        // WIFEXITED: This macro evaluates to a nonzero (true) value 
        // if the child process ended normally (that is, if it returned 
        // from main(), or else called the exit() or _exit() function).

        // When WIFEXITED() is nonzero (child process ended normally), 
        // WEXITSTATUS() evaluates to the low-order 
        // 8 bits of the status argument that the child passed to the exit() or 
        // _exit() function, or the value the child process returned from main().
        // here judge if there is any error (WEITSTATUS(status) == EXIT_FAILURE)
        if(WIFEXITED(status) && WEXITSTATUS(status)){
            return false;
        }
    }
    

    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 744);
    // bool returnVal = true;
    
    if (fd < 0) { 
        return false;
        // returnVal = false;
    }
    pid_t childPID = fork();

    if(childPID == -1){
        return false;
    }

    else if(childPID == 0){
        if (dup2(fd, 1) < 0) { 
            // int dup2(int newfd, int oldfd);
            // if newfd is already opened, close if first
            // if not, just oldfd = newfd
            // so dup2(fd, 1) means the original fd point to 1 will
            // be redirect to fd. Therefore, the string originally
            // printed to terminal will be printed out into fd.
            return false;
        }
        close(fd);

        if (execv(command[0], command) == -1){
            exit(1);
        }
    }
    else{
        pid_t pidResult;
        int status;
        pidResult = waitpid(childPID, &status, 0);

        if(pidResult == -1){
            return false;
        }

        if(WIFEXITED(status) && WEXITSTATUS(status)){
            return false;
        }        
    }
    

    va_end(args);

    return true;
}
