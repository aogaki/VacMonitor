#include <TApplication.h>
#include <TSystem.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <thread>

#include "TVacMon.hpp"

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF) {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

int main(int argc, char **argv)
{
  auto timeInterval = 60;
  auto portName = std::string("/dev/ttyUSB0");
  for (auto i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "-t") {
      timeInterval = atoi(argv[++i]);
    }
    if (std::string(argv[i]) == "-p") {
      portName = std::string((argv[++i]));
    }
  }

  TApplication app("testApp", &argc, argv);

  std::unique_ptr<TVacMon> monitor(new TVacMon());
  monitor->SetPortName(portName);
  monitor->SetTimeInterval(timeInterval);
  monitor->InitPort();

  std::thread SendCommand(&TVacMon::Write, monitor.get());
  std::thread GetResults(&TVacMon::Read, monitor.get());

  while (true) {
    gSystem->ProcessEvents();  // This should be called at main thread

    if (kbhit()) {
      monitor->Terminate();
      SendCommand.join();
      GetResults.join();
      break;
    }

    usleep(1000);
  }

  return 0;
}
