// $Id: statfile.cpp,v 1.1 2019-11-21 13:22:37-08 - - $

// NAME
//    statfile - stat(2) files given in argv
// SYNOPSIS
//    statfile filename...
// DESCRIPTION
//    Calls the stat(2) system call and prints out struct stat
//    information about the file.

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace std;

#include <libgen.h>
#include <sys/stat.h>

class failure: public exception {};

using num_base = ios_base& (*) (ios_base&);

template <typename number>
void show (number value, const string& label,
           num_base base = std::dec) {
   cout << setw(12) << showbase << base << value << " " << label
        << endl << dec;
}

void showtime (const struct timespec spec, const string& label) {
   struct tm time_buf;
   localtime_r (&spec.tv_sec, &time_buf);
   char buffer[64];
   strftime (buffer, sizeof buffer, "%Y-%m-%d %T %Z", &time_buf);
   cout << buffer << " " << label << endl;
}

void statfile (const char* filename) {
   struct stat stat_buf;
   if (stat (filename, &stat_buf) != 0) throw failure();
   cout << filename << ":" << endl;
   show (stat_buf.st_dev,     "ID of device containing file"   );
   show (stat_buf.st_ino,     "inode number"                   );
   show (stat_buf.st_mode,    "file type and mode", oct        );
   show (stat_buf.st_nlink,   "number of hard links"           );
   show (stat_buf.st_uid,     "user ID of owner"               );
   show (stat_buf.st_gid,     "group ID of owner"              );
   show (stat_buf.st_rdev,    "device ID (if special file)"    );
   show (stat_buf.st_size,    "total size, in bytes"           );
   show (stat_buf.st_blksize, "blocksize for filesystem I/O"   );
   show (stat_buf.st_blocks,  "number of 512B blocks allocated");
   showtime (stat_buf.st_atim, "time of last access"        );
   showtime (stat_buf.st_mtim, "time of last modification"  );
   showtime (stat_buf.st_ctim, "time of last status change" );
}

int main (int argc, char** argv) {
   string exec_name = basename (argv[0]);
   int exit_status = EXIT_SUCCESS;
   if (argc < 2) {
      cerr << "Usage: " << exec_name << " filename..." << endl;
      exit_status = EXIT_FAILURE;
   }else {
      for (auto argp = &argv[1]; argp != &argv[argc]; ++argp) {
         try {
            statfile (*argp);
         }catch (failure&) {
            cerr << exec_name << ": " << *argp << ": "
                 << strerror (errno) << endl;
         }
      }
   }
   return exit_status;
}

