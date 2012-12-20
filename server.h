#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include <sys/epoll.h>

#ifndef SERVER_H
#define	SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif


#define CONNMAX 1000
#define BYTES 1024
#define MESSAGE_LENGHT 99999

char *ROOT;
//Default Values PATH = ~/ and PORT=10000
char PORT[6];
struct addrinfo hints;
int efd;
struct epoll_event event;
struct epoll_event *events;

int listenfd, clients[CONNMAX];
void Error(char *);
void StartServer(char *);
void Respond(int);
void Init();
void Connection();

#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_H */

