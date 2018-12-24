#ifndef __PIPE_H__
#define __PIPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "environment.h"
#include "execute.h"
#include "newcommand.h"

using namespace std;

/*
pipe.h contains function needed to execute multi-pipe command 
*/

bool PipeCheck(string input);
std::string * ParsePipe(std::string input_ori, int & pipeNum);
void PipeExev(std::string input, Environ * environ);
void PipeConnect(Command ** array_cmd, int pipeNum, Environ * environ);
void PipeProcess(Command * cmd, Environ * environ);
void PipeCleaner(pid_t * pids, int ** pipe_array, int pipeNum, Command ** cmd_array);

/*
ParsePipe takes input string from the getline function of myShell.cpp
and the initlized pipeNum (which is 0).It cut input string by "|" and 
return the result in a string array (which is in string* form).

*/
std::string * ParsePipe(std::string input_ori, int & pipeNum) {
  std::string input = input_ori;
  std::string * PipeArr = new std::string[20];
  size_t pos;
  while ((pos = input.find("|")) != std::string::npos) {
    PipeArr[pipeNum] = input.substr(0, pos - 1);  //erase the space befor '|'
    pipeNum++;
    input.erase(0, pos + 2);  //erase the space after '|'
  }                           //cut input string by "|" if existed
  PipeArr[pipeNum] = input;
  pipeNum++;
  return PipeArr;
}

/*
PipeCheck traverses all characters in input string to see whether there exists
pipe signal '|'.Return true if pipe exists.
*/
bool PipeCheck(std::string input) {
  for (size_t i = 0; i < input.size(); i++) {
    if (input[i] == '|') {
      return true;
    }
  }
  return false;
}

/*
PipeProcess takes command object and environ object as inputs.In this function
it checks redirection first and then executes this command .
*/
void PipeProcess(Command * cmd, Environ * environ) {
  char ** argv = cmd->getArgv();

  cmd->RedictCheck();  //initlize the redict array in command

  char ** env_var = environ->getEnv();
  if (checkRedict(cmd)) {
    rediction(cmd);
  }

  if (execve(argv[0], argv, env_var) < 0) {
    printf("ERROR: %s\n", strerror(errno));
    cout << "command unexecutable!" << endl;
    return;
  }
}

/*
PipeConnect takes command array ,pipe number , environment variables as inputs.
it links each command's input and output with pipe() and dup2() .Each time we create a 
child process for a new command ,and only open the needed descriptor when we handle the 
command .After build up the pipe we pass in arguments to PipeProcess to execute the command.
*/
void PipeConnect(Command ** array_cmd, int pipeNum, Environ * environ) {
  int status = 0;
  int * pipefds = new int[2 * pipeNum];
  for (int i = 0; i < pipeNum; i++) {
    if (pipe(pipefds + 2 * i) < 0) {  //print error message when pipe failed
      std::cout << "Pipe failed at " << i << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  int count = 0;
  while (count < pipeNum) {
    pid_t pid = fork();
    if (pid == 0) {
      if (count != 0) {
        if (dup2(pipefds[(count - 1) * 2], 0) < 0) {  //connect current command's input to previous
                                                      //command's output
          std::cout << "dup2 failed when getting input at :" << count << std::endl;
          exit(EXIT_FAILURE);
        }
      }

      if (count != pipeNum - 1) {
        if (dup2(pipefds[count * 2 + 1], 1) < 0) {  //connect current command's output to next
                                                    //command's input
          std::cout << "dup2 failed when output at :" << count << std::endl;
          exit(EXIT_FAILURE);
        }
      }

      for (int j = 0; j < pipeNum * 2; j++) {
        close(pipefds[j]);
      }  //close all descriptor for parent process
      PipeProcess(array_cmd[count], environ);
    }
    else if (pid < 0) {
      std::cout << "fork failed!" << std::endl;
    }  //print out error message when fork failed
    count++;
  }
  for (int k = 0; k < pipeNum * 2; k++) {
    close(pipefds[k]);
  }

  for (int k = 0; k < pipeNum; k++) {
    wait(&status);
  }  //wait all children processes to finish
  for (int t = 0; t < pipeNum; t++) {
    delete array_cmd[t];
  }
  delete[] array_cmd;
  delete[] pipefds;  //free all space created
}

/*
PipeExev takes input string in which pipe was detected and 
environment variable as inputs.It parses input string to string
array by '|' first,then create command objects for each command .
Then it check if those command need to add absolute path ,after 
find the command in directories,PipeExev pass the command array to 
PipeConnect to execute command .
*/
void PipeExev(std::string input, Environ * environ) {
  int pipeNum = 0;
  std::string * pipeArr = ParsePipe(input, pipeNum);
  //parse input string to string array by '|'

  Command ** array_cmd = new Command *[pipeNum]();
  for (int i = 0; i < pipeNum; i++) {
    array_cmd[i] = new Command();
    array_cmd[i]->readIn(pipeArr[i]);
  }  //initialize commands in command array
  delete[] pipeArr;
  for (int i = 0; i < pipeNum; i++) {
    //check each command whether it needs an absolute path to execute
    char ** argv = array_cmd[i]->getArgv();
    if (strchr(argv[0], '/') == NULL) {
      string newname = environ->dirSearch(argv);
      if (newname == "") {
        cout << "Command " << argv[0] << " not found " << endl;
        delete array_cmd[i];
        return;
      }
      else {
        array_cmd[i]->setArgv(newname);  //set new name for command
      }
    }
  }
  PipeConnect(array_cmd, pipeNum, environ);  //execute commands in PipeConnect
}

#endif
