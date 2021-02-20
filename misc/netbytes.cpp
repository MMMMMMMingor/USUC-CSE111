// $Id: netbytes.cpp,v 1.8 2020-12-06 13:36:36-08 - - $

//
// NAME
//    netbytes - print host and network byte order
//
// SYNOPSIS
//    netbytes  number...
//
// DESCRIPTION
//    Prints the number as input and unit32_t byte swapped.
//

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <utility>
using namespace std;

#include <libgen.h>

union bytes {
   uint32_t value;
   char chars[4];
};

ostream& operator<< (ostream& out, const bytes& arg) {
   out << "0x" << uppercase
       << hex << setfill('0') << setw(8) << arg.value << " "
       << dec << setfill(' ') << setw(10) << arg.value;
   return out;
}

int main (int argc, char** argv) {
   string execname = basename (argv[0]);
   for (char** argp = &argv[1]; argp != &argv[argc]; ++argp) {
      try {
         bytes input, swapped;
         input.value = stol (*argp);
         swapped.value = input.value;
         swap (swapped.chars[0], swapped.chars[3]);
         swap (swapped.chars[1], swapped.chars[2]);
         cout << input << " => " << swapped << endl;
      }catch (invalid_argument& error) {
         cerr << execname << ": " << *argp << ": " << error.what()
              << " invalid argument" << endl;
      }
   }
}
