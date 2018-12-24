#include <stdio.h>
#include <stdlib.h>

#include <iostream>

int main(int argc, char ** argv, char ** env) {
  int i = 0;
  while (env[i] != NULL) {
    std::cout << env[i] << std::endl;
    i++;
  }
  char * a = "aaaaaaaa=dddddd";
  putenv(a);
  i = 0;
  while (env[i] != NULL) {
    std::cout << env[i] << std::endl;
    i++;
  }
  std::cout << "now setenv" << std::endl;
  char * aae = "ssededededed";
  char * value = "sdfsfsdf";
  setenv(aae, value, 0);
  i = 0;
  while (env[i] != NULL) {
    std::cout << env[i] << std::endl;
    i++;
  }
}
