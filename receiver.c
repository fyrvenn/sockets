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

time_t rawtime;
struct tm * timeinfo;

pthread_mutex_t m_mutex;
void *thread_function();
int time_to_exit = 0;
static struct blob_buf b;
char* buf;
struct ubus_context *ctx1;  

struct stat{
    uint32_t pkts;
    uint32_t bytes;
} ;

static struct stat stat_gl  = {.pkts = 0, .bytes = 0};

void* read_stat(void *args)
{
    int sock, bbi, flag = 1;
    struct sockaddr_in addr, addr_remote;
    int bytes_read;
    
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
    addr.sin_addr.s_addr = inet_addr("192.168.222.188");
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
        return NULL;
	}
    printf("Packets: %i, bytes: %i \n", stat_gl.pkts, stat_gl.bytes);    
    while(1){
        bytes_read = recvfrom(sock, buf, 1024, 0, ((struct sockaddr *)&addr_remote), &s_t);
        pthread_mutex_lock(&m_mutex);
        stat_gl.bytes+=bytes_read;
        stat_gl.pkts++;
        pthread_mutex_unlock(&m_mutex);
        printf("Packets: %i, bytes: %i \n", stat_gl.pkts, stat_gl.bytes);   
    }
        
    
}

static int ubus_send(struct ubus_context *ctx, int argc, uint32_t x, uint32_t y)
{
    int err = 0;

    err = blob_buf_init(&b, 0);
    if (err) {
        printf("Cannot init buf\n");
        return err;
    }

    if (blobmsg_add_u32(&b, "packets ", stat_gl.pkts)) {
        printf("Cannot add data!\n");
        return -1;
    }

    if (blobmsg_add_u32(&b, "bytes ", stat_gl.bytes)) {
        printf("Cannot add data!\n");
        return -1;
    }

    err = ubus_send_event(ctx, "UDP_statistics", b.head);
    if (err) {
        printf("Cannot send!\n");
    }

	return err;
}

void* send_stat_to_ubus(void *args)
{
    uint32_t count, i;  

    const char *ubus_socket = NULL;   
    ctx1 = ubus_connect(ubus_socket);
    if (!ctx1) {
		fprintf(stderr, "Failed to connect to ubus\n");
        return NULL;
	}
    while (1)
    {
        pthread_mutex_lock(&m_mutex);
        if (ubus_send(ctx1, 1, count, i)==1) {
            printf("Data sending failed!!\n");
        }
                
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        printf ( "Current local time and date: %s", asctime (timeinfo) );

        pthread_mutex_unlock(&m_mutex);
        sleep(60);
    }    	    
}
 
 
void sigint(int a)
{

    free(buf);
    ubus_free(ctx1);
    pthread_mutex_destroy(&m_mutex); 
    exit(4);
}

int main(int argc)
{

    int i = 1;
    int res;

    signal(SIGINT, sigint);
    pthread_t thread1, thread2;
     
    res = pthread_mutex_init(&m_mutex, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    pthread_create(&thread1, NULL, read_stat, NULL);
    pthread_create(&thread2, NULL, send_stat_to_ubus, NULL);

    while(1)
    {
       
    }       
    return 0;
}
