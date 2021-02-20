.so Tmac.mm-etc
.if t .Newcentury-fonts
.INITR* \n[.F]
.SIZE 12 14
.TITLE CSE-111 Winter\~2021 Program\~4 "Client/Server and Sockets"
.RCS "$Id: asg4-client-server.mm,v 1.29 2021-02-17 18:17:57-08 - - $"
.PWD
.URL
.EQ
delim $$
.EN
.hw ac-cu-mu-la-ted
.de COMMAND
.   LI "\f[CB]\\$[1]\0\f[I]\\$[2]\f[R]" \\$[3]
..  
.de MESSAGE
.   LI "\f[CB]\\$[1]\0\f[R]\\$[2]" \\$[3]
..
.H 1 "Overview"
This project will implement a client/server application using
sockets.
A daemon 
.=V ( cxid )
listens on a socket for client connection requests.
Each connection will cause the server to fork a child process 
to serve the client.
The daemon runs in an infinite loop listening.
The server exits when the client disconnects.
A client
.=V ( cxi )
connects to a server and can send files, receive files,
and get a summary listing of all files present.
.P
A socket is a two-way means of communication between processes,
not necessarily running on the same host.
An IPv4 host is know by a 4-octet sequence such as
128.114.108.152,
and a port is an unsigned 16-bit number (0 to 65535).
Communication will be done via TCP/IP over IPv4 sockets.
.H 1 "Programs"
In this project,
two main programs are to be written
for the three parts of the project\(::
the daemon and server,
and the client.
There will also be several library files written to be used
by the programs.
The general function is similar to
.V= sftp (1).
.MANPAGE=BVL
.V=LI cxid
Usage\(::
\f[CB]cxid\f[P] \|\f[I]port\f[P]
.P
Creates a server socket and goes into an infinite loop\(::
When it accepts a client socket,
it uses
.V= fork (2)
to create a child process, 
which functions as the server to communicate with the client.
The daemon listens for connections on the given port.
.P
The server is forked
with an open socket communicating with the client.
Its loop repeatedly reads commands and information from the
client and acts on those commands,
returning information back to the client.
Its loop will be a receive followed by a send,
responding to client requests.
It exits when the client closes the socket.
It does no terminal I/O except for debugging purposes.
.P
.V=LI cxi
Usage\(::
\f[CB]cxi\f[P] \|\f[I]host\f[P] \|\f[I]port\f[P]
.P
The client interacts with the user.
Commands are read from the terminal (or redirect),
each of which is executed one at a time by communicating
with the server.
Results are then displayed at the terminal.
The client connects to the given host and port.
.LE
.H 1 "Interactive Commands"
The 
.V= cxi
client responds to commands read from the standard output
and writes output to the standard output and error and accesses files.
In the syntax below,
.V= "Courier Bold"
are literal characters actually typed in, while
.IR "Roman Italic"
stands for appropriate substitutions.
.MANPAGE=BVL
.COMMAND exit
Quit the program.
An end of file marker or Control/D is equivalent.
.COMMAND get filename
Copy the file named
.IR filename
on the remote server and create or overwrite a file of the 
same name in the current directory.
.COMMAND help
A summary of available commands is printed.
.COMMAND ls
Causes the remote server to execute the command
.V= "ls -l"
and prints the output to the user's terminal.
.COMMAND put filename
Copies a local file into the socket and causes the remote server to
create that file in its directory.
.COMMAND rm filename
Causes the remote server to remove the file.
.LE
.H 1 "Protocol used by the \f[CB]cxi*\f[P] programs"
For the client and server to communicate,
a protocol needs to be established.
Each message must be framed in terms of a header
and a payload.
The header always consists of a
.V= struct
of size 64 bytes.
All messages between client and server consist of these 64 bytes,
possibly followed by a payload.
For alignment purposes, the
.V= nbytes
field is first.
Before filling in the fields, use
.V= memset (3)
to clear the struct.
.DS
.in +\n[Pi]n
.ft CB
enum class cxi_command : uint8_t {
   ERROR = 0, EXIT, GET, HELP, LS, PUT, RM, FILEOUT, LSOUT, ACK, NAK
};
size_t constexpr FILENAME_SIZE = 59;
struct cxi_header {
   uint32_t nbytes {0};
   cxi_command command {cxi_command::ERROR};
   char filename[FILENAME_SIZE] {};
};
.DE
The purposes of the fields are as follows\(::
.MANPAGE=BVL
.V=LI "uint32_t nbytes;"
The number of bytes in the payload if there is any payload.
Otherwise it must be zero (MBZ).
This field is sent in network byte order and so must use the
functions 
.V= ntohl (3)
and
.V= htonl (3)
when loading and storing data.
.V=LI "cxi_command command;"
A single byte containing one of the
.V= cxi_command
constants.
Note that the
.V= enum
is specifically a
.V= uint8_t
single byte type.
.V=LI "char filename[59];"
The name of the file being transferred or removed.
The filename may not have any slash
.=V ( '/' )
characters in it and must be null-terminated (with
.V= '\[rs]0' ).
All bytes following the null must also be null.
Pathnames with slashes and filenames longer than 58 characters
are prohibited.
.LE
.P
Following are the meanings of each of the
.V= cxi_command
values.
.ds CTS (C\[->]S)
.ds STC (S\[->]C)
Each is either client to server \*[CTS] or
server to client \*[STC], but never both.
.MANPAGE=BVL
.MESSAGE cxi_command::ERROR
An error flag to indicate an invalid header.
Used internally.
.MESSAGE cxi_command::EXIT
Internal to
.V= cxi ,
not used in communication.
.MESSAGE cxi_command::GET \*[CTS]
Request a file from the server.
The filename is used both remotely and locally.
The payload length is 0.
.MESSAGE cxi_command::HELP
Internal to
.V= cxi ,
not used in communication.
.MESSAGE cxi_command::LS \*[CTS]
Request file
.=V ( ls )
information.
The payload length and filename are zeroed.
.MESSAGE cxi_command::PUT \*[CTS]
The length of the payload is the number of bytes in the file.
The contents of the file immediately follow the header.
The bytes of the payload are unstructured and may contain null bytes.
Binary files are acceptable.
.MESSAGE cxi_command::RM \*[CTS]
Request to remove a file.
The payload length is 0.
.MESSAGE cxi_command::FILEOUT \*[STC]
Response to a
.V= cxi_command::GET .
The filename is the same as in the request and the payload length
reflects the number of bytes in the file.
The payload consists of the bytes of the file.
.MESSAGE cxi_command::LSOUT \*[STC]
Response to a
.V= cxi_command::LS .
The filename is zeroed and the payload length is the number of bytes
sent in the payload.
The payload is the output of the command
.V= "ls -l" .
.MESSAGE cxi_command::ACK \*[STC]
Response to either a
.V= cxi_command::PUT
or a
.V= cxi_command::RM 
indicating that the request was successfully completed.
.MESSAGE cxi_command::NAK \*[STC]
Response to any request that fails.
There is no payload.
The filename field is the same as was in the original request.
The nbytes field is set to the value of
.V= errno
in the server's attempt to preform a task.
.LE
.H 1 "Procedures"
Each of the above commands requires procedures for accessing files,
including reading files from disk and writing files to disk,
as well as accessing directories.
When any of the system calls fails in the server,
the server immediately terminates the operation and sends
the value of
.V= errno
back to the client in a 
.V= cxi_command::NAK
message.
.ALX a ()
.LI
For the client or server to send a file it must first be read into
a buffer.
Binary files must be properly handled, 
so protocols which assume text files won't work.
To load a file from disk, use 
.V= ifstream::read() ,
collecting characters into a buffer.
Read the entire file into a buffer then close it.
After that,
it may be sent down the socket.
.LI
Alternatively,
.V= stat (2)
the file to see how large it is,
and send the file down the socket piecemeal.
In conjunction with
.V= stat (2),
it is also possible to may the file into memory using
.V= mmap (2),
provided that the memory thus acquired is released with
.V= munmap (2)
when the file operation is complete.
.LI
When receiving a file from the socket,
Receive the header and determine the size of the file.
Create an
.V= ofstream
and use
.V= ofstream::write()
to write the parts of the file as they are received from the socket.
A C++ stream is closed
when the variable goes out of scope,
or you can call
.V= close .
.LI
To delete a file for the
.V= cxi_command::RM
command, use
.V= unlink (2)\(::
.VTCODE* 1 "rc = unlink (filename);"
.LI
To execute the
.V= cxi_command::LS
command use
.V= popen (2)
and
.V= pclose (2)
to create a pipe stream from the
.V= ls (1)
command
.VTCODE* 1 "FILE* pipe = popen (\[Dq]ls -l\[Dq], \[Dq]r\[Dq]);"
Then read the characters from the pipe in the easiest way,
probably by using 
.V= fgets (3).
Finally,
.V= "pclose(pipe)" .
Then send the output back the client in a 
.V= cxi_command::LSOUT
message.
.LE
.H 1 "Modules"
There will need to be several modules in this suite of programs.
Each of the programs, of course, will have its own source file
with a main function in it.
.P
The
.V= sockets
module will be a useful inclusion into the program
as its own module.
.P
There should also be a
.V= cxi_protocol
module to implement the protocols and contain code for accessing
files and sockets,
since these will be used by both the client and the server.
.H 1 "Use of sockets"
The sysadmin operating
.V= unix.ucsc.edu
has blocked all ports except for the ssh port for security reasons,
so you will not be able to run the server on one of the
Linux servers and your client on your own machine.
When testing your program,
open two terminals on the
.E= same
host.
You may log into any host to run your server, 
then start it up.
Then, in another window, log into the same host to run the client.
The client and the server must be run in different directories.
Use the
.V= hostname (1)
command to discover which host you are logged into.
Example\(::
.TVCODE* 1 "-bash-71\[Do] " "hostname"
.TVCODE* 1 unix2.lt.ucsc.edu
The name
.V= unix.ucsc.edu
is not actually a real host.
It is just an alias for one of the real hosts,
which have name patterns like the one shown here.
.H 1 "Use of ports"
If your daemon listens on a port that has been bound by another
process, you will get the message ``Address already in use''
because only one process at any given time is allowed to listen
on a particular process.
To avoid this, choose a port number not being used by anyone
else on the same server.
.P
Port numbers are divided into three ranges\(::
.ALX a ()
.LI
Ports 0 through 1023 (0x3FF) are well known.
They are associated with services in a static manner
and are reserved to privileged processes.
A list of these services may be found in the file
.V= /etc/services .
.LI
Port numbers from 1024 (0x400) through 49151 (0xBFFF) are registered.
They are used for multiple purposes.
Choose any of these ports for your server if not already in use.
.LI
Dynamic and private ports are those
from 49152 (0xC000) through 65535 (0xFFFF)
and services should not be associated with them.
.LE
.H 1 "Runaway Processes"
Be careful in using
.V= fork (2)
so that you don't accidentally create a fork-bomb.
The command
.V= pkill (1)
can be used to kill all processes matching a particular pattern.
So the command
.VTCODE* 1 "pkill cxi"
will kill all of your processes whose executables contain the string
.=V `` cxi ''.
A really quick way to log out is to use
.V= kill (1)\(::
.VTCODE* 1 "kill -9 -1"
.VTCODE* 1 "kill -s KILL -1"
will send
.V= SIGKILL
to all of your processes,
thus logging you out.
.H 1 "What to Submit"
Submit
.V= Makefile
which builds both programs,
all necessary C++ header and implementation files.
And if doing pair programming, the
.V= PARTNER
file.
When the grader uses the command
.V= make
in the submit directory,
the both binaries should be built.
