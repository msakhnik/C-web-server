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

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
//Default Values PATH = ~/ and PORT=10000
char PORT[6];
int listenfd, clients[CONNMAX];
void error(char *);
void startServer(char *);
void respond(int);

void ParsingCommandLine(int argc, char* argv[])
{
    char c;
    //Parsing the command line arguments
    while ((c = getopt(argc, argv, "p:r:")) != -1)
        switch (c)
        {
        case 'r':
            ROOT = malloc(strlen(optarg));
            strcpy(ROOT, optarg);
            break;
        case 'p':
            strcpy(PORT, optarg);
            break;
        case '?':
            fprintf(stderr, "Wrong arguments given!!!\n");
            exit(1);
        default:
            exit(1);
        }
}

int main(int argc, char* argv[])
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;

    ROOT = getenv("PWD");
    strcpy(PORT, "10000");

    int slot = 0;
    ParsingCommandLine(argc, argv);
    printf("Server started at port no. %s with root directory as %s \n", PORT, ROOT);
    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i = 0; i < CONNMAX; i++)
        clients[i] = -1;
    startServer(PORT);

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof (clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot] < 0)
            error("accept() error");
        else
        {
            if (fork() == 0)
            {
                respond(slot);
                exit(0);
            }
        }
        while (clients[slot] != -1) slot = (slot + 1) % CONNMAX;
    }
    return 0;
}

//start server

void startServer(char *port)
{
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p != NULL; p = p->ai_next)
    {
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p == NULL)
    {
        perror("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(listenfd, 1000000) != 0)
    {
        perror("listen() error");
        exit(1);
    }
}

//client connection

void respond(int n)
{
    char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999], php[99999] = "php ";
    int rcvd, fd, bytes_read;
    char buffer[BUFSIZ + 1];

    memset((void*) mesg, (int) '\0', 99999);

    rcvd = recv(clients[n], mesg, 99999, 0);

    if (rcvd < 0) // receive error
        fprintf(stderr, ("recv() error\n"));
    else if (rcvd == 0) // receive socket closed
        fprintf(stderr, "Client disconnected upexpectedly.\n");
    else // message received
    {
        printf("%s", mesg);
        reqline[0] = strtok(mesg, " \t\n");
        if (strncmp(reqline[0], "GET\0", 4) == 0)
        {
            reqline[1] = strtok(NULL, " \t");
            reqline[2] = strtok(NULL, " \t\n");
            if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp(reqline[2], "HTTP/1.1", 8) != 0)
            {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                if (strncmp(reqline[1], "/\0", 2) == 0)
                    reqline[1] = "/index.html"; //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("file: %s\n", path);

                if ((fd = open(path, O_RDONLY)) != -1) //FILE FOUND
                {
                    send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
                    while ((bytes_read = read(fd, data_to_send, BYTES)) > 0)
                    {
                        printf("Bytes read: %d\n", bytes_read);
                        strncat(php, (char *)path, strlen(path));
                        printf("%s\n", php);
                        memset(buffer, '\0', sizeof(buffer));
                        FILE * read_fp;
                        int chars_read = 0;
                        printf("%s\n", php);
                        read_fp = popen(php, "r");

                        if (read_fp != NULL)
                        {
                            chars_read = fread(buffer, sizeof(char), BUFSIZ, read_fp);                            
                            if (chars_read > 0)
                            {
                                write(clients[n], buffer, bytes_read);
//                                bytes_read -= strlen(data_to_send);
                            }
                            else
                                printf("chars_read: %d\n", chars_read);
                        }

                        pclose(read_fp);
                        
                    }
                }
                else write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
            }
        }
    }

    //Closing SOCKET
    printf("N=%d\n", n);
    if (shutdown(clients[n], SHUT_RDWR) == 0)
    {
        printf("Close client request №%d\n", n);
        close(clients[n]);
        clients[n] = -1;
    }
}