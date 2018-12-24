#ifndef __BUILDIN_H__
#define __BUILDIN_H__

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "environment.h"
#include "execute.h"
#include "newcommand.h"
/*
bulidIn.h contains all functions to execute buildIn functions.
*/
string getvalue(string input, char ** argv);
bool isValidName(string key);
void set(Command * cmd, Environ * env, string input);
void cd(Command * cmd, Environ * env);
void exportval(Command * cmd, Environ * env, string input);
void inc(Command * cmd, Environ * env, string input);
bool buildInCheck(Command * cmd, Environ * env, string input);

/*
cd takes command object and environ object as inputs to change  current
directory of shell.it would print error when it failed to change the 
current derecory.
*/
void cd(Command * cmd, Environ * env) {
  char ** argv = cmd->getArgv();
  int argc = cmd->getArgc();
  if (argc == 2) {  //if arguments are in "cd xxx"form
    if (chdir(argv[1]) == -1) {
      cout << "cd failed!" << endl;
    }
    return;
  }
  else if (argc == 1) {  //if argument is "cd",goes to home directory.
    char * home;
    home = getenv("HOME");

    if (chdir(home) == -1) {
      cout << "cd failed!" << endl;
    }
    return;
  }
  return;
}

/*
getvalue is used for "set" buildIn function .It cuts input command string 
by the first space and take the remain string after the first space as 
the  variable's value and return it to set function.
*/
string getvalue(string input, char ** argv) {
  size_t value_i = input.find(std::string(argv[1])) + strlen(argv[1]);
  while (input[value_i] == ' ')
    value_i++;  // skip spaces
  //get value for set
  return input.substr(value_i);
}

/*
isValidname takes variable name string input ,then traverses all characters
int the input string to see whether there is invalid characters in variable
name ,then returns true if this string is a vaild name of variable.
*/
bool isValidName(string key) {
  size_t len = key.length();
  for (size_t i = 0; i < len; i++) {
    if (key[i] >= 'a' && key[i] <= 'z') {
      continue;
    }
    else if (key[i] >= 'A' && key[i] <= 'Z') {
      continue;
    }
    else if (key[i] >= '0' && key[i] <= '9') {
      continue;
    }
    else if (key[i] == '_') {
      continue;
    }
    else {
      return false;
    }
  }
  return true;
}

/*
set is the bulidIn function of this shell.It check the argument of command 
first to see if it have proper number of arguments and whether the name of 
variable is valid .If it is valid,inserts the variable into the map we used 
to record variable(no exported to environment yet.)
*/
void set(Command * cmd, Environ * env, string input) {
  char ** argv = cmd->getArgv();
  if (argv[1] == NULL) {  //check number of arguments
    std::cout << "set needs arguement!" << std::endl;
    return;
  }
  if (argv[2] == NULL) {  //insert variable with empty value is allowed
    env->insertMap(argv[1], "");
  }
  string key = argv[1];
  if (isValidName(key)) {  //check if variable name is vaild
    string value = getvalue(input, argv);
    env->insertMap(argv[1], value);
  }
  else {  //print out error message
    cout << "it is not a vaild name !" << endl;
  }
}

/*
exportval is the buildIn function of this shell.It check whether this variable
existed in variable map first.If it  exists,it would make a string by variable 
name and its value ,and add it to environment variable by function addVar().
*/
void exportval(Command * cmd, Environ * env, string input) {
  char ** argv = cmd->getArgv();
  if (argv[1] == NULL) {
    std::cout << "export needs argument !" << std::endl;
    return;
  }
  if (env->mapContain(argv[1])) {
    string value = env->mapGetValue(argv[1]);
    string key = argv[1];
    string pair = key + "=" + value;  //make the pair string if variable exists in
    //variable map

    env->addVar(pair);  //export variable to environment variable
  }
  else {  //print out error message
    std::cout << " export failed to find this key " << std::endl;
  }
  return;
}

/*
inc is the buildIn function of this shell.It checks whether the variable
is existed in variable map first.If it is not existed,inc()would create a 
new variable in this name with a value "0".Then it would increase the value
of variable by calling class function mapIncment(). 
*/
void inc(Command * cmd, Environ * env, string input) {
  char ** argv = cmd->getArgv();
  if (argv[1] == NULL) {
    std::cout << "inc needs argument !" << std::endl;
    return;
  }
  if (!env->mapContain(argv[1])) {  //check if this variable exists
    string a = "0";

    env->insertMap((argv[1]), a);
  }  //If var is not currently set,the initial value is treated as 0

  env->mapIncment(argv[1]);
}

/*
buildInCheck takes command object ,environ object and input string as 
inputs.It checks the first argument of argv(which is always function 
name if it is a buildIn command )and decides whether it is a buildIn
function,return boolean value.
*/
bool buildInCheck(Command * cmd, Environ * env, string input) {
  char ** argv = cmd->getArgv();
  if (!strcmp(argv[0], "cd")) {
    cd(cmd, env);
    return true;
  }
  else if (!strcmp(argv[0], "set")) {
    set(cmd, env, input);
    return true;
  }
  else if (!strcmp(argv[0], "export")) {
    exportval(cmd, env, input);
    return true;
  }
  else if (!strcmp(argv[0], "inc")) {
    inc(cmd, env, input);
    return true;
  }
  else if (!strcmp(argv[0], "exit")) {
    //if it is exit,quits shell and says goodbye to users.
    delete cmd;
    delete env;
    cout << "Goodbye!" << endl;
    exit(0);
  }
  else {
    return false;
  }
}

#endif
