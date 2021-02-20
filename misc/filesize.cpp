// $Id: filesize.cpp,v 1.3 2019-11-21 14:03:52-08 - - $

// Print size of files given as command line arguments.

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

#include <libgen.h>
#include <sys/stat.h>

int main (int argc, char** argv) {
   string exec_name = basename (argv[0]);
   int exit_status = EXIT_SUCCESS;
   for (auto argp = &argv[1]; argp != &argv[argc]; ++argp) {
      const char* filename = *argp;
      struct stat stat_buf;
      int status = stat (filename, &stat_buf);
      if (status != 0) {
         cerr << exec_name << ": " << filename << ": "
              << strerror (errno) << endl;
         exit_status = EXIT_FAILURE;
      }else {
         cout << setw(12) << stat_buf.st_size << " "
              << filename << endl;
      }
   }
   return exit_status;
}
