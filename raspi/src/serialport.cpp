#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include "serialport.hpp"

using namespace std;

int openSerialPort(void) {

	int fd;

  struct termios settings;

  fd = open(PORT, O_RDWR | O_NOCTTY);  // Read-write mode

  if (fd == -1) {
    cout << "Error in opening tty" << endl;
    exit(-1);
  }

  // Serial port settings
  tcgetattr(fd, &settings);

  cfsetispeed(&settings,B115200); // Speed for read
  cfsetospeed(&settings,B115200); // Speed for write

  settings.c_cflag &= ~PARENB;
  settings.c_cflag &= ~CSTOPB;
  settings.c_cflag &= ~CSIZE;
  settings.c_cflag |=  CS8;
  settings.c_cflag &= ~CRTSCTS;
  settings.c_cflag |= CREAD | CLOCAL;
  settings.c_iflag &= ~(IXON | IXOFF | IXANY);
  settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  settings.c_oflag &= ~OPOST;

  // Blocking mode
  settings.c_cc[VMIN] = 1;
  settings.c_cc[VTIME] = 0;

  if((tcsetattr(fd,TCSANOW,&settings)) != 0) {
    cout << "Error in setting attributes" << endl;
    exit(-1);
  }

  tcflush(fd, TCIFLUSH);

	return fd;
}

