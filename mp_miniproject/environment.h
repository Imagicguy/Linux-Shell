#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

using namespace std;

/*
 environment.h contains Envrion class,this class contains private field:
 envPath:contains "PATH" in environment variables(which is used to search command ).
 newEnv :contains environment variables.
 num_path: record how many paths in envPath.
 num_env : record how many environment variables in newEnv.
*/
typedef pair<string, int> pair_val;
class Environ
{
 private:
  char ** envPath;
  char ** newEnv;
  int num_path;
  int num_env;
  map<string, pair_val> val_map;

 public:
  Environ(char ** env) : envPath(NULL), newEnv(NULL), num_path(0), num_env(0) {
    //this constructor takes environment variables as input and splits it by ":"
    //to constrcut envPath and num_path.Then records environment variables in
    //the new Environ object/
    envPath = NULL;
    char * pPath;
    pPath = getenv("PATH");
    char * pPath_copy = (char *)malloc((strlen(pPath) + 1) * sizeof(char));
    strcpy(pPath_copy, pPath);
    char * pch;
    int count = 0;
    if (pPath_copy != NULL) {  //splits "PATH" by ":"
      pch = strtok(pPath_copy, ":");

      while (pch != NULL) {
        envPath = (char **)realloc(envPath, (count + 1) * sizeof(char *));
        envPath[count] = (char *)malloc((strlen(pch) + 1) * sizeof(char));
        strcpy(envPath[count], pch);

        count++;
        pch = strtok(NULL, ":");
      }
    }
    free(pPath_copy);
    num_path = count;

    //copy environment variables
    count = 0;
    while (env[count] != NULL) {
      newEnv = (char **)realloc(newEnv, (count + 1) * sizeof(char *));
      newEnv[count] = (char *)malloc((strlen(env[count]) + 1) * sizeof(char));

      strcpy(newEnv[count], env[count]);
      count++;
    }
    newEnv = (char **)realloc(newEnv, (count + 1) * sizeof(char *));
    newEnv[count] = NULL;  //set end of newEnv to NULL,which helps us to find the end of
    //environment variables
    num_env = count;
  }
  //run of three:
  ~Environ() { freeHelper(); }

  Environ & operator=(const Environ & rhs) {
    if (this != &rhs) {
      freeHelper();
      copyHelper(rhs);
    }

    return *this;
  }

  Environ(const Environ & rhs) { copyHelper(rhs); }

  void freeHelper() {
    for (int i = 0; i < num_path; i++) {
      free(envPath[i]);
    }
    free(envPath);
    for (int i = 0; i < num_env; i++) {
      free(newEnv[i]);
    }
    free(newEnv);
  }
  void copyHelper(const Environ & rhs) {
    char ** rhs_envPath = rhs.getPath();
    int rhs_num_path = rhs.getNum();
    envPath = (char **)malloc((rhs_num_path) * sizeof(char *));
    for (int i = 0; i < rhs_num_path; i++) {
      envPath[i] = (char *)malloc(strlen(rhs_envPath[i] + 1) * sizeof(char));
      strcpy(envPath[i], rhs_envPath[i]);
    }

    num_path = rhs_num_path;
  }

  char ** getPath() const { return envPath; }

  int getNum() const { return num_path; }

  char ** getEnv() const { return newEnv; }

  /*
    addVar() is used to add pair string to environment variables 
   */
  void addVar(string pair) {
    newEnv[num_env] = (char *)malloc((strlen(pair.c_str()) + 1) * sizeof(char));
    strcpy(newEnv[num_env], pair.c_str());
    newEnv = (char **)realloc(newEnv, (num_env + 2) * sizeof(char *));
    num_env++;
    newEnv[num_env] = NULL;
  }

  /*
   dirSearch() is used to search the input command in envPath and change the filename with
   proper path ,if this command exist in envPath's directories.
   */
  string dirSearch(char ** argv) {
    DIR * d;
    struct dirent * dir;
    for (int i = 0; i < num_path; i++) {
      d = opendir(envPath[i]);  //use for loop to open all directories in envPath
      if (d) {
        while ((dir = readdir(d)) != NULL) {
          //check each file under directories to see whether it exists
          if (strcmp(dir->d_name, argv[0]) == 0) {
            closedir(d);
            string newname = string(envPath[i]) + "/" + string(argv[0]);
            return newname;  //if it is ,return the newname
          }
        }
        closedir(d);
      }
    }
    return "";
  }

  /*
    mapContain() is used to check whether input key is contained in current map,
    return true if it is .
   */
  bool mapContain(string key) {
    std::map<std::string, pair_val>::iterator it;
    it = val_map.find(key);
    return (it != val_map.end());
  }  //return whether key is existed in map

  /*
    mapGetvalue() return the value of input  name variable if it is existed in 
    map,else it return "".
   */
  string mapGetValue(string key) {
    std::map<std::string, pair_val>::iterator it;
    it = val_map.find(key);
    if (it != val_map.end()) {
      pair_val res_pair = it->second;
      return res_pair.first;
    }
    return "";
  }  //return value of var

  /*
    insertMap() is used to insert variables in map.
    it would erase the old variables if it has same name with the new variable.
   */
  void insertMap(string key, string value) {
    if (mapContain(key))
      val_map.erase(key);
    pair_val newpair = make_pair(value, 0);
    val_map.insert(pair<string, pair_val>(key, newpair));
  }

  /*
    mapIncment() is used to increase variables' values.it works only if this variable
    is already existed in the map.
   */
  void mapIncment(string key) {
    std::map<std::string, pair_val>::iterator it;
    it = val_map.find(key);
    if (it != val_map.end()) {
      string value = it->second.first;
      int number = atoi(value.c_str());
      //if value not represent a numberin base10, then atoi would change  value to  0
      number += 1;  //increment 1
      ostringstream newvalue;
      newvalue << number;
      string strvalue = newvalue.str();
      pair_val newpair = make_pair(strvalue, 0);
      val_map.erase(key);
      val_map.insert(pair<string, pair_val>(key, newpair));
    }
    else {
      std::cout << "inc failed to find this key !" << std::endl;
    }
  }

  /*
    mapPrint() helps user to check the current situation of their environ map.
   */
  void mapPrint() {
    std::map<std::string, pair_val>::iterator it;
    for (it = val_map.begin(); it != val_map.end(); ++it)
      std::cout << it->first << "\n => " << it->second.first << "  " << it->second.second
                << std::endl;
  }
};

#endif
