#ifndef __EXECUTE_H__
#define __EXECUTE_H__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "environment.h"
#include "newcommand.h"

/*
execute.h contains functions needed to execute non-buildIn non-pipe function .
*/

/*checkRedict traverses all elements in cmd->argv to decide whether there exists redirection
needed to be handle
*/
bool checkRedict(Command * cmd) {
  char ** argv = cmd->getArgv();
  int argc = cmd->getArgc();
  for (int i = 0; i < argc; i++) {
    if ((!strcmp(argv[i], "<")) || (!strcmp(argv[i], ">")) || (!strcmp(argv[i], "2>"))) {
      return true;
    }
  }
  return false;
}
/*
rediction check cmd->redict to see whether rediction need ,and use the index stored in
cmd->redict to track the redirecion file's name.
*/
void rediction(Command * cmd) {
  int * redict = cmd->getRedict();
  int argc = cmd->getArgc();
  char ** argv = cmd->getArgv();
  int last_valid_index = 200;  //index of the last vaild element after redirection in argv
  for (int i = 0; i < 3; i++) {
    if (redict[i] != 0) {
      if (last_valid_index > redict[i])
        last_valid_index = redict[i];
    }
  }  //get the smallest of invaild index
  last_valid_index -= 1;

  if (redict[0] != 0) {  // redirect stdin
    int descriptor = open(argv[redict[0]], O_RDONLY, S_IRUSR);
    if (dup2(descriptor, 0) < 0) {
      std::cout << "dup2 failed!" << std::endl;
      exit(EXIT_FAILURE);
    }
    close(descriptor);
  }

  for (int i = 1; i < 3; i++) {  // redirect stdout, stderr
    if (redict[i] != 0) {
      int descriptor = open(argv[redict[i]], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      if (dup2(descriptor, i) < 0) {
        std::cout << "dup2 failed!" << std::endl;
        exit(EXIT_FAILURE);
      }
      close(descriptor);
    }
  }

  if (last_valid_index != 200) {
    for (int i = last_valid_index; i < argc; i++) {
      free(argv[i]);
    }
    argv[last_valid_index] = NULL;  //set last element of argv to NULL,execve likes that
  }
  cmd->setArgc(last_valid_index);  //update argc of this command
}
/*
exeCmd is used to execute command without pipe,this function creates child process and 
parent process,it executes command and redirection in child process and spies status in 
parent process.If this command is not executable ,this function would return false.
*/
bool exeCmd(Command * cmd, char ** env, Environ * environ) {
  pid_t pid, wait;
  int status = 0;

  pid = fork();  //create child process
  if (pid == -1) {
    fprintf(stderr, "Fork Failed!");
    exit(EXIT_FAILURE);
  }

  else if (pid == 0) {  //enter chlid process

    char ** argv = cmd->getArgv();
    char ** env_var = environ->getEnv();

    if (checkRedict(cmd)) {
      rediction(cmd);
    }
    //if this command is not executable ,false would be returned.
    if (execve(argv[0], argv, env_var) < 0) {
      printf("ERROR: %s\n", strerror(errno));
      cout << "command unexecutable!" << endl;
      return false;

    }  //execute mission in child process
  }

  else {  //enter parent process

    int ifExited;
    int ifSignaled;

    do {
      wait = waitpid(0, &status, WUNTRACED | WCONTINUED);  //spy child process
      if (wait == -1) {
        printf("Parent wait = -1, error = %s \n", strerror(errno));
      }
      ifExited = WIFEXITED(status);      //return true when exit
      ifSignaled = WIFSIGNALED(status);  //return true when killed by signal
      if (ifExited) {
        printf("Program exited with status %d\n", WEXITSTATUS(status));
      }
      else if (ifSignaled) {
        printf("Program was killed by signal %d\n", WTERMSIG(status));
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return true;
}

#endif
