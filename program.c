#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/event.h>
#include <sys/socket.h>

void ok(char *msg) { printf("OK: %s\n", msg); }
void die(char *msg) { perror(msg); exit(-1); }
void usage(char *name) { printf("usage: %s port\n", name); exit(-1); }
void unless_die(int val, char *msg) { if (val < 0) die(msg); }
void unless_perror(int val, char *msg) { if (val < 0) perror(msg); }

void set_nonblocking(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void server_setting(int sockfd) {
  int result;
  int one = 1;
  result  = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&one, sizeof(int));
  unless_die(result, "setsockopt()");
}

int  server_socket(int port);
void server_loop(int sockfd);
/* void server_accept(int kq, int sockfd); */
/* void server_recv(int kq, struct kevent *evp); */
/* void server_send(int kq, struct kevent *evp); */

int (* vlog)(const char *, ...) = &printf;

int main(int argc, char *argv[])
{
  int port, sockfd;

  if (argc != 2) usage(argv[0]);
  port = atoi(argv[1]);
  if (port <= 0) usage(argv[0]);

  sockfd = server_socket(port);
  server_loop(sockfd);
  return 0;
}

/* create and return a server socket */
int server_socket(int port)
{
  int                result;
  int                sockfd;
  struct sockaddr_in sin;

  /* socket() */
  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  (sockfd < 0) ? die("socket") : ok("socket()");

  /* server_setting() */
  server_setting(sockfd);

  /* bind() */
  memset(&sin, '\0', sizeof(sin));
  sin.sin_family      = AF_INET;
  sin.sin_port        = htons(port);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  result              = bind(sockfd, (struct sockaddr *)&sin, sizeof(sin));
  (result < 0) ? die("bind()") : ok("bind()");

  /* listen() */
  result = listen(sockfd, 128);
  (result < 0) ? die("listen()") : ok("listen()");

  return sockfd;
}

char http_200[] = "HTTP/1.1 200 OK\n\r\n";
char http_400[] = "HTTP/1.1 400 Bad Request\n\r\n";

void server_accept(int kq, int sockfd) {
  int                new_fd;
  int                result;
  socklen_t          sin_size;
  struct sockaddr_in sin;
  struct kevent      chlist[1];

  while (1) {
    new_fd = accept(sockfd, (struct sockaddr *)&sin, &sin_size);
    if (new_fd < 0) { break; }

    EV_SET(&chlist[0], new_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    result = kevent(kq, chlist, 1, NULL, 0, NULL);
    unless_perror(result, "kevent()");
  }
  if (errno != EAGAIN) perror("accept()");
}

void server_recv(int kq, struct kevent *evp) {
  char          recv_buf[4096];
  char          send_buf[4096];
  int           result;
  int           sockfd = evp->ident;
  struct kevent chlist[1];

  memset(send_buf, 0, sizeof(send_buf));
  strcpy(send_buf, http_200);
  result = read(sockfd, recv_buf, sizeof(recv_buf));

  if (result <= 0) { close(sockfd); return; }

  strncat(send_buf, recv_buf, 4096 - strlen(http_200) - 1);
  EV_SET(&chlist[0], sockfd, EVFILT_WRITE, EV_ADD, 0, 0, send_buf);
  result = kevent(kq, chlist, 1, NULL, 0, NULL);
  unless_perror(result, "kevent()");
}

void server_send(int kq, struct kevent *evp) {
  int   result;
  int   sockfd = evp->ident;
  char *udata  = evp->udata;
  result       = write(sockfd, udata, strlen(udata));
  if (result <= 0) {
    perror("write()");
    vlog("sockfd: %d, result: %d, errno: %d\n", sockfd, result, errno);
  }
  unless_perror(result, "write()");
  close(sockfd);
}

void server_loop(int sockfd)
{
  int           kq, nev, i, result;
  struct kevent kev;
  struct kevent evlist[1024];

  kq = kqueue();
  if (kq < 0) die("kqueue()");

  /* register listen event */
  set_nonblocking(sockfd);
  EV_SET(&kev, sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  result = kevent(kq, &kev, 1, NULL, 0, NULL);
  if (result < 0) die("kevent()");

  while (1) {
    /* vlog("==================== waiting ====================\n"); */
    nev = kevent(kq, NULL, 0, evlist, 1024 * 10, NULL);

    /* event loop */
    for (i = 0; i < nev; i++) {
      kev = evlist[i];

      /* vlog("ident: %hd, filter: %hd, flags: %d, data: %d\n", */
      /*      kev.ident, kev.filter, kev.flags, kev.data); */
      /* if (kev.udata) vlog("========== udata ==========\n%s\n", kev.udata); */

      if (kev.ident == sockfd) {                /* accpet */
        server_accept(kq, sockfd);
      } else if (kev.filter == EVFILT_READ) {   /* read */
        server_recv(kq, &kev);
      } else if (kev.filter == EVFILT_WRITE) {  /* write */
        server_send(kq, &kev);
      }
    }
  }
}
