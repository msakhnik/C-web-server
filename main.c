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

#include "server.h"

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
    Init();
    strcpy(PORT, "10000");
    ParsingCommandLine(argc, argv);
    printf("Server started at port no. %s \n", PORT);

    StartServer(PORT);

    Connection();

    // ACCEPT connections
    return 0;
}