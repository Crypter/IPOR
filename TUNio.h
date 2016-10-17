#ifndef TUNIO_H
#define TUNIO_H

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <string>

using namespace std;

class TUNio
{
private:
int tunSocket;
string tunName;

public:

TUNio()
{
	tunName = "CMeth";
}

int init()
{
	struct ifreq ifr;
	int err;

	if( (tunSocket = open("/dev/net/tun", O_RDWR)) < 0 )
	{
		perror("Opening /dev/net/tun");
		return tunSocket;
	}

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

	if ( !tunName.empty() )
	{
		strncpy(ifr.ifr_name, tunName.c_str(), IFNAMSIZ);
	}

	if( (err = ioctl(tunSocket, TUNSETIFF, (void *)&ifr)) < 0 )
	{
		perror("ioctl(TUNSETIFF)");
		close(tunSocket);
		return err;
	}

	tunName = ifr.ifr_name;

return tunSocket;
}



int receive(char *buf, int n){
  
  int nread;

  if((nread=read(tunSocket, buf, n))<0){
    perror("Reading data");
    exit(1);
  }
  return nread;
}

int send(char *buf, int n){
  
  int nwrite;

  if((nwrite=write(tunSocket, buf, n))<0){
    perror("Writing data");
    exit(1);
  }
  return nwrite;
}





};

#endif