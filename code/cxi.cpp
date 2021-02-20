// $Id: cxi.cpp,v 1.1 2020-11-22 16:51:43-08 - - $

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"
#include "util.hpp"

logstream outlog (cout);
struct cxi_exit: public exception {};

unordered_map<string,cxi_command> command_map {
   {"exit", cxi_command::EXIT},
   {"help", cxi_command::HELP},
   {"ls"  , cxi_command::LS  },
   {"get" , cxi_command::GET },
   {"put" , cxi_command::PUT },
   {"rm"  , cxi_command::RM  },
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cxi_help() {
   cout << help;
}

void cxi_ls (client_socket& server) {
   cxi_header header;
   header.command = cxi_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cxi_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      size_t host_nbytes = ntohl (header.nbytes);
      auto buffer = make_unique<char[]> (host_nbytes + 1);
      recv_packet (server, buffer.get(), host_nbytes);
      outlog << "received " << host_nbytes << " bytes" << endl;
      buffer[host_nbytes] = '\0';
      cout << buffer.get();
   }
}

void cxi_get(client_socket& server, const string& file_name){
   if(file_name == ""){
      outlog << "sent GET, file name can not be empty" << endl;
      return;
   }

   cxi_header header;
   header.command = cxi_command::GET;
   memset(header.filename, 0, FILENAME_SIZE);
   memcpy(header.filename, file_name.c_str(), file_name.size());
   outlog << "sending header" << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   if (header.command != cxi_command::FILEOUT){
      outlog << "sent GET, server did not return FILEOUT" << endl;
      outlog << "server returned " << header << endl;

      return;
   }

   size_t host_nbytes = ntohl(header.nbytes);
   auto buffer = make_unique<char[]>(host_nbytes + 1);
   recv_packet(server, buffer.get(), host_nbytes);
   outlog << "received " << host_nbytes << " bytes" << endl;
   buffer[host_nbytes] = '\0';

   ofstream ofs(file_name, std::ofstream::out);
   ofs << buffer.get();
   ofs.close();
}

void cxi_put(client_socket& server, const string& file_name){
   if(file_name == ""){
      outlog << "sent PUT, file name can not be empty" << endl;
      return;
   }

   ifstream ifs(file_name, std::ifstream::in);

   std::string content((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
   ifs.close();

   cxi_header header;
   header.command = cxi_command::PUT;
   header.nbytes = htonl (content.size());
   memset(header.filename, 0, FILENAME_SIZE);
   memcpy(header.filename, file_name.c_str(), file_name.size());
   outlog << "sending header" << header << endl;
   send_packet (server, &header, sizeof header);
   send_packet (server, content.c_str(), content.size());
   outlog << "sent " << content.size() << " bytes" << endl;

   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   if (header.command != cxi_command::ACK){
      outlog << "sent PUT, server did not return ACK" << endl;
      outlog << "server returned " << header << endl;

      return;
   }

   outlog << "put " << file_name << " success" << endl;
}

void cxi_rm(client_socket& server, const string& file_name){
   if(file_name == ""){
      outlog << "sent RM, file name can not be empty" << endl;
      return;
   }

   cxi_header header;
   header.command = cxi_command::RM;
   memset(header.filename, 0, FILENAME_SIZE);
   memcpy(header.filename, file_name.c_str(), file_name.size());
   outlog << "sending header" << header << endl;
   send_packet (server, &header, sizeof header);

   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   if (header.command != cxi_command::ACK){
      outlog << "sent RM, server did not return ACK" << endl;
      outlog << "server returned " << header << endl;

      outlog << "rm " << file_name << " fail" << endl;
      return;
   }

   outlog << "rm " << file_name << " success" << endl;
}

void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cxi_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cxi_server_host (args, 0);
   in_port_t port = get_cxi_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cxi_exit();
         outlog << "command " << line << endl;

         vector<string> commands = util::split(line);

         const auto& itor = command_map.find (commands[0]);
         cxi_command cmd = itor == command_map.end()
                         ? cxi_command::ERROR : itor->second;

         string file_name = "";
         if(commands.size() > 1)
            file_name = commands[1];

         switch (cmd) {
            case cxi_command::EXIT:
               throw cxi_exit();
               break;
            case cxi_command::HELP:
               cxi_help();
               break;
            case cxi_command::LS:
               cxi_ls (server);
               break;
            case cxi_command::GET:
               cxi_get (server, file_name);
               break;
            case cxi_command::PUT:
               cxi_put(server, file_name);
               break;
            case cxi_command::RM:
               cxi_rm(server, file_name);
               break;
            default:
               outlog << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cxi_exit& error) {
      outlog << "caught cxi_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}

