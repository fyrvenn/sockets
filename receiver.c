#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <libubus.h>
#include </usr/local/include/libubox/blob.h>
#include </usr/local/include/libubox/blobmsg_json.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

struct UdpHeader
{
    u_short src_port;
    u_short targ_port;
    u_short length;
    u_short checksum;
};

static struct blob_buf b;

static int ubus_send(struct ubus_context *ctx, int argc, char **argv, uint32_t x, uint32_t y)
{
    int err = 0;

    printf("argc = %d\n", argc);

    err = blob_buf_init(&b, 0);
    if (err) {
        printf("Cannot init buf\n");
        return err;
    }

    if (blobmsg_add_u32(&b, "packets ", y)) {
        printf("Cannot add data!\n");
        return -1;
    }

    if (blobmsg_add_u32(&b, "bytes ", x)) {
        printf("Cannot add data!\n");
        return -1;
    }

    err = ubus_send_event(ctx, "UDP_statistics", b.head);
    if (err) {
        printf("Cannot send!\n");
    }

	return err;
} 

// static wait = 1;
 
void sigint(int a)
{
    // printf("^C caught\n");
    perror("halt");
    exit(4);
}

int main(int argc, char **argv)
{
    int sock, bbi;
    struct sockaddr_in addr, addr_remote;
    char* buf;
    int bytes_read;
    struct ubus_context *ctx;
    const char *ubus_socket = NULL;
    bool parse_correct;

    buf = malloc(1024);
    
    if (buf==NULL) 
    {
        perror("Cannot allocate buf");
        exit(3);
    }

    memset(buf, 0, 1024);

    sock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = PF_INET;
    addr.sin_port = htons(3425);
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr = inet_addr("192.168.222.195");
    // addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }
    
    char word[100];    
    addr_remote.sin_port =  addr.sin_port;
    socklen_t s_t =  sizeof(struct sockaddr_in);

    bbi = blob_buf_init(&b, 0);
    if (bbi!=0) {
        fprintf(stderr, "Error blob buf init\n");
		return -1;
	}   
    else 
    {
        printf("blob_buf_init==0\n"); 
    }
    
    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}
    
    int i = 1, count = 0;

    while(1)
    {
        signal(SIGINT, sigint);
        bytes_read = recvfrom(sock, buf, 1024, 0, ((struct sockaddr *)&addr_remote), &s_t);
        buf[bytes_read] = '\0';
        count+=bytes_read;
        if (ubus_send(ctx, argc, argv, count, i)==1) {
            printf("Data sending failed!!\n");
        }
        printf("%i \n", i);
        i++;        
    }

    free(buf);
	ubus_free(ctx);
    return 0;
}
