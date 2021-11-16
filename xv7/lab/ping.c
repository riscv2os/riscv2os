//
// send a UDP packet to port 3000 on the host (outside of qemu),
// and receive a response.
//

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/net.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char obuf[4] = { 1, 2, 3, 4 };
  uint32 dst;

  // 10.0.2.2, which qemu remaps to the external host,
  // i.e. the machine you're running qemu on.
  dst = (10 << 24) | (0 << 16) | (2 << 8) | (2 << 0);

  // you can send a UDP packet to any Internet address
  // by using a different dst.
  
  if(send(dst, 3000, 2000, obuf, sizeof(obuf)) < 0){
    fprintf(2, "ping: send() failed\n");
    exit(1);
  }

  uint32 src;
  uint16 sport;
  uint16 dport;
  char ibuf[128];
  int cc = recv(&src, &sport, &dport, ibuf, sizeof(ibuf));
  if(cc < 0){
    fprintf(2, "ping: recv() failed\n");
    exit(1);
  }
  printf("recv: n=%d src=%x sport=%d dport=%d\n", cc, src, sport, dport);

  exit(0);
}
