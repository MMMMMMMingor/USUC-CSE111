// $Id: lsfile.cpp,v 1.15 2020-11-30 11:46:08-08 - - $

// NAME
//    lsfile - print output similar to ls(1) command
// SYNOPSIS
//    lsfile filename...
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
#include <pwd.h>
#include <sys/stat.h>

class failure: public exception {};

using num_base = ios_base& (*) (ios_base&);

void lsfile (const char* filename) {
   struct stat stat_buf;
   if (stat (filename, &stat_buf) != 0) throw failure();
   struct tm time_tm;
   localtime_r (&stat_buf.st_mtim.tv_sec, &time_tm);
   char time_str[64];
   strftime (time_str, sizeof time_str, "%Y-%m-%d %T", &time_tm);
   auto passwd_buf = getpwuid (stat_buf.st_uid);
   if (passwd_buf == nullptr) throw failure();
   cout << oct << setfill('0') << setw(6) << stat_buf.st_mode
        << dec << setfill(' ') << setw(2) << stat_buf.st_nlink
        << setw(10) << passwd_buf->pw_name
        << setw(12) << stat_buf.st_size
        << " " << time_str
        << " " << filename << endl;
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
            lsfile (*argp);
         }catch (failure&) {
            cerr << exec_name << ": " << *argp << ": "
                 << strerror (errno) << endl;
         }
      }
   }
   return exit_status;
}

//
// The  following mask values are defined for the file type of the
// st_mode field:
//
//     S_IFMT     0170000   bit mask for the file type bit field
//     S_IFSOCK   0140000   socket
//     S_IFLNK    0120000   symbolic link
//     S_IFREG    0100000   regular file
//     S_IFBLK    0060000   block device
//     S_IFDIR    0040000   directory
//     S_IFCHR    0020000   character device
//     S_IFIFO    0010000   FIFO
//
// The  following mask values are defined for the file mode compo-
// nent of the st_mode field:
//
//     S_ISUID     04000   set-user-ID bit
//     S_ISGID     02000   set-group-ID bit (see below)
//     S_ISVTX     01000   sticky bit (see below)
//
//     S_IRUSR     00400   owner has read permission
//     S_IWUSR     00200   owner has write permission
//     S_IXUSR     00100   owner has execute permission
//
//     S_IRGRP     00040   group has read permission
//     S_IWGRP     00020   group has write permission
//     S_IXGRP     00010   group has execute permission
//
//     S_IROTH     00004   others have read permission
//     S_IWOTH     00002   others have write permission
//     S_IXOTH     00001   others have execute permission
//
// The set-group-ID bit (S_ISGID) has several special uses.  For a
// directory, it indicates that BSD semantics is to  be  used  for
// that directory: files created there inherit their group ID from
// the directory, not from the effective group ID of the  creating
// process,  and  directories  created  there  will  also  get the
// S_ISGID bit set.  For a file that does not have the group  exe-
// cution bit (S_IXGRP) set, the set-group-ID bit indicates manda-
// tory file/record locking.
//
// The sticky bit (S_ISVTX) on a directory means that  a  file  in
// that  directory  can be renamed or deleted only by the owner of
// the file, by the owner of the directory, and  by  a  privileged
// process.
//
