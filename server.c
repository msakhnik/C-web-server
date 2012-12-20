#include "server.h"

void StartServer(char *port)
{
    struct addrinfo *res, *p;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    for (p = res; p != NULL; p = p->ai_next)
    {
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1)
            continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
    }
    if (p == NULL)
    {
        perror("socket() or bind()");
        exit(1);
    }
    freeaddrinfo(res);
    // Listen
    if (listen(listenfd, 1000000) != 0)
    {
        perror("listen() error");
        exit(1);
    }
}

void Init()
{
    ROOT = getenv("PWD");
    memset(clients, -1, CONNMAX);

    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
}

void Connection()
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    int slot = 0;
    while (1)
    {
        addrlen = sizeof (clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *)
                               &clientaddr, &addrlen);
        if (clients[slot] < 0)
            perror("accept() error");
        else
        {
//            InitEpol();
            if (fork() == 0)
            {
                Respond(slot);
                exit(0);
            }
        }
        while (clients[slot] != -1) slot = (slot + 1) % CONNMAX;
    }
}

void InitEpol()
{
    int s = 0;
    efd = epoll_create1(0);
    if (efd == -1)
    {
        perror("epoll_create");
        abort();
    }

    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event);
    if (s == -1)
    {
        perror("epoll_ctl");
        abort();
    }

    /* Buffer where events are returned */
    events = calloc(CONNMAX, sizeof event);
}

void GetHttpHeader(char * mesg, int n)
{
    char *reqline[3];
    char path[MESSAGE_LENGHT];
    int fd;
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
                reqline[1] = "/index.html";

            strcpy(path, ROOT);
            strcpy(&path[strlen(ROOT)], reqline[1]);
            printf(" uploaded file: %s\n", path);

            if ((fd = open(path, O_RDONLY)) != -1) //FILE FOUND
            {
                send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
                SendRequest(path, n, fd);
            }
            else write(clients[n], "HTTP/1.0 404 Not Found\n", 23);
        }
    }

}

void SendRequest(char *path, int n, int fd)
{
    char data_to_send[BYTES], php[99999] = "php ";
    int bytes_read;
    char buffer[BUFSIZ + 1];

    while ((bytes_read = read(fd, data_to_send, BYTES)) > 0)
    {
        printf("-+-%s----\n", path);
        printf("Bytes read: %d\n", bytes_read);
        strncat(php, (char *) path, strlen(path));
        printf("%s\n", php);
        memset(buffer, '\0', sizeof (buffer));
        FILE * read_fp;
        int chars_read = 0;
        printf("%s\n", php);
        read_fp = popen(php, "r");

        if (read_fp != NULL)
        {
            chars_read = fread(buffer, sizeof (char), BUFSIZ, read_fp);
            if (chars_read > 0)
            {
                write(clients[n], buffer, bytes_read);
            }
            else
                printf("chars_read: %d\n", chars_read);
        }

        pclose(read_fp);
    }
}

//client connection

void Respond(int n)
{
    printf("In Response\n");

    char mesg[MESSAGE_LENGHT];
    int rcvd;

    memset((void*) mesg, (int) '\0', MESSAGE_LENGHT);

    rcvd = recv(clients[n], mesg, MESSAGE_LENGHT, 0);

    if (rcvd < 0) // receive error
        fprintf(stderr, ("recv() error\n"));
    else if (rcvd == 0) // receive socket closed
        fprintf(stderr, "Client disconnected upexpectedly.\n");
    else // message received
    {
        printf("%s", mesg);
        GetHttpHeader(mesg, n);
    }

    //Closing SOCKET
    if (shutdown(clients[n], SHUT_RDWR) == 0)
    {
        printf("Close client request №%d\n", n);
        close(clients[n]);
        clients[n] = -1;
    }
}