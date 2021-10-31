#include <iostream>
#include <unistd.h>
#include <cstdlib> // для system
using namespace std;

int main()
{
  for (;;)
  {
    cout << " (1970-01-01 05:36:06.268545)  can0       701   [1]  05                        '.'" << endl;
    usleep(100);
    cout << " (1970-01-01 05:36:06.179242)  can0       703   [1]  05                        '.'" << endl;
    usleep(100);
    cout << "ERROR FRAME controller-problem{rx-error-warining}}" << endl;
    usleep(100);
    cout << " (1970-01-01 05:36:06.179242)  can0       703   [1]  05                        '.'" << endl;
    usleep(100);
    cout << " (1970-01-01 05:36:06.180698)  can0       705   [1]  05                        '.'" << endl;
    usleep(100);
    cout << " (1970-01-01 05:37:06.110698)  can0       706   [1]  05                        '.'" << endl;
    usleep(100);
    cout << " (1970-01-01 05:36:06.136358)  can0  1E0007F0   [8]  64 64 00 15 15 15 15 15   'dd......'" << endl;
    usleep(100);
  }

    return 0;
}
