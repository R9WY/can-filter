#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <string>
#include <string.h>
#include <ctype.h>
#include <regex>
#include <sys/select.h>


//* структуры для асинхронного чтения
static constexpr int STD_INPUT = 0;
static constexpr __suseconds_t WAIT_BETWEEN_SELECT_US = 250000L;

//* UDP адрес по умолчанию
#define DEF_UDP_ADDR "127.0.0.1"
//* UDP порт по умолчанию
#define DEF_UDP_PORT 8889
//* Диапазон допустимых UDP портов
#define DEF_MIN_UDP_PORT 1000
#define DEF_MAX_UDP_PORT 50000

//* Значение фильтра по умолчанию
#define DEF_FILTER_VALUE "01"

//* Мин. и макс значение CANid
#define DEF_MIN_ID 0x01
#define DEF_MAX_ID 0x7f

//* Шаблон корректного сообщения
#define DEF_CAN_TEMPL R"( [(]\w{4}[-]\w{2}[-]\w{2} \w{2}[:]\w{2}[:]\w{2}[.]\w{6}[)])"

//* мин. количество символов в разбираемой строке
#define DEF_MIN_PROT_STR_LEN    60

//* позиция адреса в логе
#define DEF_PROT_ADDR_POS       37
//* позиция в логе в пвкете NMT
#define DEF_PROT_NMT_ADDR_POS   51

//* Подсказка
#define _USAGE_TEXT_ "Usage: can-filter [options] \nOptions:\n\
    -a [ip address]\t\t Destination UDP address (127.0.0.1)\n\
    -p [num]\t\t\t Destination UDP port number (from 1000 to 20000) default:8889\n\
    -f [hexnum]\t\t\t Filter CAN ID (from 0x01 to 0x7F)\n\n\
    Example: can-filter -a 192.168.1.1 -p 8889 -f 0x05 < dump.txt"

using namespace std;

//* флаг выхода
volatile sig_atomic_t flag_exit = 0;
/********************************************************
* Функция вызова при получении SIGINT и SIGTERM
*/
void check_signal(int sig)
{ // can be called asynchronously
  flag_exit = 1; // set flag

  cout << "Catch terminate signal" << endl;
}

/********************************************************
* Функция преобразования из HEX в DEC
* \param[in]        Hexstr    hex-число в текстовом виде
* \param[in\out]    Overflow  возвращает флаг переполнения
* \return           Десятичное число
*/
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



/********************************************************
* Процедура выборки первого токена из строки с пробелами
* \param[in\out]  string   преобразуемая строка
*/
void getFirstTok(std::string& string)
{
    size_t strBegin=string.find_first_not_of(' ');
    if (strBegin!=std::string::npos)string.erase(0, strBegin);
    size_t strEnd=string.find_first_of(' ');
    if (strEnd!=std::string::npos)string.erase(strEnd, string.size() - strEnd);
}

/********************************************************
* MAIN
*/
int main(int argc, char* argv[])
{
  // переменные для передачи на сервер
  int sockfd;
  struct sockaddr_in servaddr;
  auto udp_port=DEF_UDP_PORT;
  char ip_addr[12]=DEF_UDP_ADDR;

  string buffer,addr_str;

  // определяем рег. выражение
  std::regex regex_e (DEF_CAN_TEMPL);
  std::smatch m;

  //значение выбираемого CanId
  auto filter_can_id=DEF_MIN_ID;
  auto cur_can_id=0;
  int c,len;

 //Регистрируем сигналы
 signal(SIGINT, check_signal);
 signal(SIGTERM, check_signal);

try
   {
/*----------------------------------------
* Обрабатываем параметры командной строки
*/
while ((c = getopt(argc, argv, ":a:p:f:h")) != -1)
{
  switch(c) //
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
        strncpy(ip_addr,optarg,11);
      continue;

    case 'f': // Значение фильтра
       //преобразуем в число, проверяем диапазон
       filter_can_id=Hex2Int(optarg, NULL);
       if ((filter_can_id<DEF_MIN_ID)||(filter_can_id>DEF_MAX_ID)) throw std::invalid_argument( "Invalid filter argument value");
      continue;

    case 'h':
      cout << _USAGE_TEXT_ << endl;
      exit(0);

    default :
    case -1:
       break;
  } //switch getopt

  break;
} //for getopt

/*----------------------------------------
* Создаем соединение с UDP-сервером
*/
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        throw std::runtime_error( "Socket creation failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    in_addr_t ip_to_num;

    cout << "Connecting to address " <<ip_addr << " port "<<udp_port<<endl;

    if (inet_pton(AF_INET, ip_addr, &ip_to_num) <= 0) { throw std::invalid_argument( "Error in IP translation to special numeric format"); }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(udp_port);
    servaddr.sin_addr.s_addr = ip_to_num;

/*--------------------------------------------------
* Считываем строки, проверяем и обрабатываем текст
*/
  struct timeval tv = { 0L, WAIT_BETWEEN_SELECT_US };
  fd_set fds;

// можно заменить на синхронное чтение
//while (getline(cin, buffer)){

   while (!flag_exit)
    {
        //асинхронное чтение cin, чтобы можно было отследить сигналы завершения программы
        //завершаем программу только после завершения отправки пакета на UDP-сервер
        while (!flag_exit)
        {
            //int ready = select(STD_INPUT + 1, &fds, NULL, NULL, &tv);
            FD_ZERO(&fds);
            FD_SET(STD_INPUT, &fds);

            if (select(STD_INPUT + 1, &fds, NULL, NULL, &tv) > 0)
            {
            // читаем строку, если поток закрылся, то выходим
            if (!std::getline(std::cin, buffer))flag_exit=1;
            break;
            }
            usleep(10);
        }

        // короткие строки сразу игнорируем
       if (buffer.length()<DEF_MIN_PROT_STR_LEN) continue;

       //проверяем строку на соответствие шаблону
       // if (string(addr_str,0,2)==" (")
       if (std::regex_search (buffer,m,regex_e))
        {
            //получаем блок строки, содержащий адрес
            addr_str=string(buffer,DEF_PROT_ADDR_POS,8);
            getFirstTok(addr_str);
            len=addr_str.length();
            cur_can_id=0;
            // отбрасываем пакеты не содержащие NodeId, извлекаем номер из соответсвующих
            switch (len)
            {
               case 3:
                    // если NMT пакет, тогда NodeId - второй байт из блока данных
                    if (addr_str=="000")
                        {
                         cur_can_id=Hex2Int(string(buffer,DEF_PROT_NMT_ADDR_POS,2).c_str(), NULL);
                         break;
                        }
                    // если обычный пакет, то 2 и 3 байт содержат NodeId
                    // 181-57F, 581-5FF,601-67F, 701-77F
                    // Преобразуем в десятичное и берем по маске DEF_MAX_ID.
                    cur_can_id=Hex2Int(string(addr_str,1,2).c_str(), NULL)&DEF_MAX_ID;
                    break;
               case 8:
                //если расширенный пакет, используем 5,6 байт и берем по маске DEF_MAX_ID
                if ((string(addr_str,0,3)=="1E0"))cur_can_id=Hex2Int(string(addr_str,4,2).c_str(), NULL)&DEF_MAX_ID;
                break;

               default:
                break;
            } // switch len

            //Если совпадает, отправляем на сервер
            if (filter_can_id==cur_can_id)
            {
              sendto(sockfd, (const char *)buffer.c_str(), buffer.length(), 0, (const struct sockaddr *) &servaddr,sizeof(servaddr));
              //cout<<buffer<<endl;
            }

        } //if (std::regex_search

    } // while (getline
} // try
   catch ( exception &e )
   {
      cerr << "Error: " << e.what( ) << endl;
      //cerr << "Type " << typeid( e ).name( ) << endl;
   };

close(sockfd);
return 0;
} // main
