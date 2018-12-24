#ifndef __NEWCOMMAND_H__
#define __NEWCOMMAND_H__
#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

using namespace std;
/*
 Command class contains all data needed to execute command,it has private fields:
 argv : stores all argument is this char **.
 argc : records the number of arguments in this command.
 redict[3] : used to decide whether this command contains redirection and redirecting signal index.
*/
class Command
{
 private:
  char ** argv;
  int argc;
  int redict[3];

 public:
  Command() : argv(NULL), argc(0) {}

  //rule of three
  ~Command() { freeHelper(); }

  Command & operator=(const Command & rhs) {
    if (this != &rhs) {
      freeHelper();
      copyHelper(rhs);
    }

    return *this;
  }

  Command(const Command & rhs) { copyHelper(rhs); }

  void copyHelper(const Command & rhs) {
    char ** rhs_argv = rhs.getArgv();
    int rhs_argc = rhs.getArgc();
    argv = (char **)malloc((rhs_argc + 1) *
                           sizeof(char *));  //one more place for argv[rhs_argc] = NULL
    for (int i = 0; i < rhs_argc; i++) {
      argv[i] = (char *)malloc(strlen(rhs_argv[i]));
      strcpy(argv[i], rhs_argv[i]);
    }
    argv[rhs_argc] = NULL;
    argc = rhs_argc;
  }
  void freeHelper() {
    for (int i = 0; i <= argc; i++) {
      free(argv[i]);
    }
    free(argv);
  }
  char ** getArgv() const { return argv; }

  int getArgc() const { return argc; }

  int * getRedict() const { return (int *)redict; }

  /*
    RedictCheck() is used to check whether this command has rediretion in it.If it is
    ,it record the redirected filename's index in the redict array.
   */
  void RedictCheck() {
    for (int j = 0; j < 3; j++) {
      redict[j] = 0;
    }  //initialize redict with zero

    for (int i = 0; i < argc; i++) {
      if (!strcmp(argv[i], "<") && i < argc - 1) {
        redict[0] = i + 1;
      }  //record output filename by it's index in argv
      else if (!strcmp(argv[i], ">") && i < argc - 1) {
        redict[1] = i + 1;
      }  //record input filename by it's index in argv
      else if (!strcmp(argv[i], "2>") && i < argc - 1) {
        redict[2] = i + 1;
      }  //record error-out filename by it's index in argv
    }
  }
  /*
    readIn() takes input string from getline.it would parse "\ " to " " and 
    "\\" to "\" and stores it in argument array char** argv.
   */
  string readIn(string input_ori) {
    argv = NULL;
    argc = 0;
    string input = input_ori;
    int length = input.length();
    string cur_str = "";
    if (input == "") {
      std::cout << "empty input !" << std::endl;
    }
    if (input[0] != ' ' || input[0] != '\\') {
      cur_str = cur_str + input[0];
    }
    for (int i = 1; i < length; i++) {
      if (input[i] == ' ') {
        if (input[i - 1] == '\\') {
          cur_str = cur_str + ' ';
          input[i] = '#';
        }
        else if (input[i - 1] != ' ') {
          argv = (char **)realloc(argv, (argc + 1) * sizeof(char *));
          argv[argc] = (char *)malloc(cur_str.size() + 1);
          strcpy(argv[argc], cur_str.c_str());
          argc++;
          cur_str = "";
        }
      }
      else if (input[i] == '\\') {
        if (input[i - 1] == '\\') {
          cur_str = cur_str + '\\';

          input[i] = '#';
        }
      }
      else {
        cur_str = cur_str + input[i];
      }
    }
    //need to check if it has remain argument after for loop.
    if (cur_str != "") {
      argv = (char **)realloc(argv, (argc + 1) * sizeof(char *));
      argv[argc] = (char *)malloc((cur_str.size() + 1) * sizeof(char));
      strcpy(argv[argc], cur_str.c_str());
      argc++;
      //cur_str = "";
    }

    argv = (char **)realloc(argv, (argc + 1) * sizeof(char *));
    argv[argc] = NULL;  //set end of argv to NULL,execve likes that.
    return input_ori;
  }

  /*
    setArgv() is used to set new name if it needs path to execute.
   */
  void setArgv(string newname) {
    int len = newname.length();
    argv[0] = (char *)realloc(argv[0], (len + 1) * sizeof(char));
    strcpy(argv[0], newname.c_str());
  }
  void setArgc(int newargc) { argc = newargc; }
};

#endif
