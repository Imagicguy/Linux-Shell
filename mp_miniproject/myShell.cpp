#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "buildIn.h"
#include "environment.h"
#include "execute.h"
#include "newcommand.h"
#include "pipe.h"
using namespace std;

/*
  init_shell() is used to clear current screen and  show the welcome image.
*/
void init_shell() {
  printf("\033[H\033[J");
  printf("\n\n\n\n******************"
         "************************");
  printf("\n\n\n\t****MY SHELL****");
  printf("\n\n\t-AUTHOR: Haili Wu-");
  printf("\n\n\n\n*******************"
         "***********************");

  printf("\n");
  sleep(1);
}

/*
prompt_type() is used to should the current directory of the shell.
*/
void prompt_type() {
  char * dir_path = get_current_dir_name();
  cout << "myShell:" << dir_path << " $";
  free(dir_path);
}

/*
commandExecve()  is used to execute non-pipe command .It would check if it is 
buildIn function,then it would check if it needs a more proper path and execute 
this command .
*/
void commandExecve(Environ * environ, Command * cmd, char ** env, string input) {
  char ** argv = cmd->getArgv();
  if (buildInCheck(cmd, environ, input)) {
    return;
  }  //if it is buildIn function ,it would execute in buildInCheck function and return
  //true to this function and return to main().
  else if (strchr(argv[0], '/') == NULL) {
    //check whether this command needs a more proper path in it's name.
    string newname = environ->dirSearch(argv);
    if (newname == "") {
      //prints error message if this command is not found in dirSearch();
      cout << "Command " << argv[0] << " not found " << endl;
      return;
    }
    else {
      cmd->setArgv(newname);  //set new name for the command
    }
  }

  if (!exeCmd(cmd, env, environ)) {  //whether cmd executable
    //free all space created and exit if this command is not executable.
    delete cmd;
    delete environ;
    exit(0);
  }
}

/*
  replacevar() is used to check whether the input string has $+variable_name in it
  and repalce the $+variable_name with this variable's value.
*/
string replaceVar(string input, Environ * environ) {
  size_t len = input.length();
  string var = "";        //var is used to be the variable name to check in env->mapContain().
  string new_input = "";  //new_input is used to generate new input string
  bool in_var = false;
  for (size_t i = 0; i < len; i++) {
    if (input[i] == '$') {
      in_var = true;
      continue;
    }
    if (in_var) {
      var += input[i];
      if (environ->mapContain(var)) {
        new_input += environ->mapGetValue(var);
        var = "";
        in_var = false;
      }
    }
    else {
      new_input += input[i];
    }
  }
  return new_input;
}

int main(int argc, char ** argv, char ** env) {
  init_shell();
  Environ * environ = new Environ(env);
  std::string input;
  prompt_type();
  //getline(cin,input)
  while (getline(std::cin, input)) {
    input = replaceVar(input, environ);  //parse input string with variable
    if (PipeCheck(input)) {              //check if pipe existed
      PipeExev(input, environ);
      prompt_type();
    }
    else {
      Command * cmd = new Command();
      input = cmd->readIn(input);
      cmd->RedictCheck();                       //check if redirection existed
      commandExecve(environ, cmd, env, input);  //1.check buildin function first 2. execute
      delete cmd;
      prompt_type();
    }
  }

  delete environ;

  return EXIT_SUCCESS;
}
