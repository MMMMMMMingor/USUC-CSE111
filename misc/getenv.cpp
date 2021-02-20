// $Id: getenv.cpp,v 1.24 2017-08-02 15:58:04-07 - - $

// NAME
//    showgetenv - show the operation of getenv
//
// SYNOPSIS
//    showgetenv envvar...
//
// DESCRIPTION
//    Prints the value of environment variables given on the
//    command line.
//

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <string>
#include <vector>
using namespace std;

int main (int argc, char**argv) {
   int status = EXIT_SUCCESS;
   string execname = basename (argv[0]);
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() == 0) {
      extern char** environ;
      for (auto env = environ; *env != nullptr; ++env) {
         cout << *env << endl;
      }
   }else {
      for (string varname: args) {
         const char* value = getenv (varname.c_str());
         if (value == nullptr) {
            cerr << execname << ": " << varname << ": no such variable"
                 << endl;
            status = EXIT_FAILURE;
         }else {
            cout << varname << "=" << value << endl;
         }
      }
   }
   return status;
}

//TEST// mkpspdf getenv.cpp.ps getenv.cpp
