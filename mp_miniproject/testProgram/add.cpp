#include <stdio.h>
#include <stdlib.h>

#include <iostream>

using namespace std;
int main(int argc, char ** argv, char ** env) {
  int result = 0;
  if (argc != 3) {
    cout << "need 2 adder !" << endl;
  }
  else {
    result = atoi(argv[1]) + atoi(argv[2]);
    cout << result << " = " << argv[1] << " + " << argv[2] << endl;
  }
  return result;
}
