#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <string>

#define DEF_UDP_PORT 8889
#define DEF_UDP_ADDR "127.0.0.1"

#define _USAGE_TEXT_ "Usage: can-filter [options] \nOptions:\n\
    -a [ip address] \t\t Destination UDP address (8889)\n\
    -p [num] \t\t\t Destination UDP port number (127.0.0.1)"


using namespace std;

auto udp_port=DEF_UDP_PORT;
auto ip_addr=DEF_UDP_ADDR;

volatile sig_atomic_t flag_exit = 0;


void check_signal(int sig)
{ // can be called asynchronously
  flag_exit = 1; // set flag
}

int main(int argc, char* argv[])
{
// Register signals
 // signal(SIGINT, check_signal);
// ===================================
// Обрабатываем командную строку
for(;;)
{
  switch(getopt(argc, argv, ":a:p:h")) //
  {
    case 'p': // Read and check Port number
       udp_port=atoi(optarg);
       if ((udp_port<1000)||(udp_port>20000))
        {
          udp_port=DEF_UDP_PORT;
          cout << "incorrect port number, using default port:"<<DEF_UDP_PORT << endl;
        }
      continue;

    case 'a':
      //printf("parameter 'b' specified with the value %s\n", optarg);
      continue;

    case '?':
    case 'h':
      cout << _USAGE_TEXT_ << endl;
      exit(0);

    default :
    case -1:
       break;


      break;
  } //switch getopt

  break;
} //for getopt
//--------------------------------------

string buffer;

while (getline(cin, buffer))
    cout<<buffer<<endl;


return 0;
}

/*

*/
