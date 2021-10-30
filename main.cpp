#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <string.h>
#include <ctype.h>
#include <regex>

#define DEF_UDP_PORT 8889
#define DEF_UDP_ADDR "127.0.0.1"
#define DEF_FILTER_VALUE "01"
#define DEF_MIN_ID 0x01
#define DEF_MAX_ID 0x7f
#define DEF_MIN_UDP_PORT 1000
#define DEF_MAX_UDP_PORT 50000

#define DEF_CAN_TEMPL R"( [(]\w{4}[-]\w{2}[-]\w{2} \w{2}[:]\w{2}[:]\w{2}[.]\w{6}[)])"

#define _USAGE_TEXT_ "Usage: can-filter [options] \nOptions:\n\
    -a [ip address]\t\t Destination UDP address (127.0.0.1)\n\
    -p [num]\t\t\t Destination UDP port number (from 1000 to 20000) default:8889\n\
    -f [hexnum]\t\t Filter COB ID (from 0x01 to 0x7F)\n\n\
    Example: can-filter -a 192.168.1.1 -p 8889 -f 0x05 < dump.txt"

using namespace std;

auto udp_port=DEF_UDP_PORT;
auto ip_addr=DEF_UDP_ADDR;

volatile sig_atomic_t flag_exit = 0;

void check_signal(int sig)
{ // can be called asynchronously
  flag_exit = 1; // set flag
}


template<typename T = unsigned int>
T Hex2Int(const char* const Hexstr, bool* Overflow)
{
    if (!Hexstr)
        return false;
    if (Overflow)
        *Overflow = false;

    auto between = [](char val, char c1, char c2) { return val >= c1 && val <= c2; };
    size_t len = strlen(Hexstr);
    T result = 0;

    for (size_t i = 0, offset = sizeof(T) << 3; i < len && (int)offset > 0; i++)
    {
        if (between(Hexstr[i], '0', '9'))
            result = result << 4 ^ Hexstr[i] - '0';
        else if (between(tolower(Hexstr[i]), 'a', 'f'))
            result = result << 4 ^ tolower(Hexstr[i]) - ('a' - 10); // Remove the decimal part;
        offset -= 4;
    }
    if (((len + ((len % 2) != 0)) << 2) > (sizeof(T) << 3) && Overflow)
        *Overflow = true;
    return result;
}


void deleteSpaces(std::string& string)
{
    size_t strBegin=string.find_first_not_of(' ');
    size_t strEnd=string.find_last_not_of(' ');
    string.erase(strEnd+1, string.size() - strEnd);
    string.erase(0, strBegin);
}


int main(int argc, char* argv[])
{
  string buffer,addr_str;
  // рег выражение шаблона правильной строки
//  std::regex regex_e (R"( [(]\w{4}[-]\w{2}[-]\w{2} \w{2}[:]\w{2}[:]\w{2}[.]\w{6}[)])");
  std::regex regex_e (DEF_CAN_TEMPL);
  std::smatch m;  // for result regex

  auto filter_value_calc=DEF_MIN_ID;
  auto cur_value_calc=0;

 //Register signals
 //signal(SIGINT, check_signal);

try
   {
// ===================================
// Обрабатываем параметры командной строки
for(;;)
{
  switch(getopt(argc, argv, ":a:p:f:h")) //
  {
    case 'p': // Read and check Port number
       udp_port=atoi(optarg);
       if ((udp_port<DEF_MIN_UDP_PORT)||(udp_port>DEF_MAX_UDP_PORT))
        {
          udp_port=DEF_UDP_PORT;
          cout << "incorrect port number, using default port:"<<DEF_UDP_PORT << endl;
        }
      continue;

    case 'a': // IP адрес сервера
      //printf("parameter 'b' specified with the value %s\n", optarg);
      continue;

    case 'f': // Значение фильтра
       //преобразуем в число, проверяем диапазон
       filter_value_calc=Hex2Int(optarg, NULL);
       if ((filter_value_calc<DEF_MIN_ID)||(filter_value_calc>DEF_MAX_ID)) throw std::invalid_argument( "Invalid filter argument value");
      continue;

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


//std::regex regex_(R"([(]\w{4}[-]\w{2}[-]\w{2} \w{2}[:]\w{2}[:]\w{2}[.]\w{2}[.]\w{6}[)]/g)");

   while (getline(cin, buffer))
    {
       //проверяем строку на соответсвие шаблону
       if (std::regex_search (buffer,m,regex_e))
        {
            addr_str=string(buffer,37,8);
            deleteSpaces(addr_str);
            if (string(addr_str,0,2)=="1E")addr_str=string(addr_str,4,2);
             else
              {
                addr_str=string(addr_str,1,2);
              }
            //addr_str=addr_str.substr(0,addr_str.find(" "));
            //Преобразуем в десятичное и берем только 11 бит по маске
            cur_value_calc=Hex2Int(addr_str.c_str(), NULL)&DEF_MAX_ID;

            if (filter_value_calc==cur_value_calc) { cout<<buffer<<endl;}
            //cout<<cur_value_calc<<endl;

        }
    }
   }
   catch ( exception &e )
   {
      cerr << "Error: " << e.what( ) << endl;
      //cerr << "Type " << typeid( e ).name( ) << endl;
   };
return 0;
}

