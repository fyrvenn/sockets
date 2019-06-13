#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

struct UdpHeader
{
    u_short src_port;
    u_short targ_port;
    u_short length;
    u_short checksum;
};


int main()
{
    int sock;
    struct sockaddr_in addr, addr_remote;
    char* buf;
    int bytes_read;

    buf = malloc(1024);
    
    if (buf==NULL) 
    {
        perror("Cannot allocate buf");
        exit(3);
    }
    memset(buf, 0, 1024);
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr = inet_addr("192.168.222.229");
    // addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }
    
    char word[100];

    socklen_t s_t =  sizeof(struct sockaddr_in);
    while(1)
    {
        bytes_read = recvfrom(sock, buf, 1024, 0, ((struct sockaddr *)&addr_remote), &s_t);
        buf[bytes_read] = '\0';
        //printf("%s \n", buf);
        printf("%x \n", ntohl(addr_remote.sin_addr.s_addr));
        printf("%x \n", ntohs(addr_remote.sin_port));
        //printf("%x \n", addr_remote.sin_family);
        //for(int i=0; i<8; i++)
        // for(int i=0; i<=bytes_read; i++)
        // {
        //     //printf("Ox%x", buf[i]);
        //     printf(" [%i] %x ", i, buf[i]);        
        // }

        printf("\n \n");
        
    }

    free(buf);
    return 0;
}
