#include <stdio.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char * argv[]) {
  string name;
  while (getline(cin, name)) {
    if (name == "exit")
      break;
    cout << name << endl;
  }
  return 0;
}
