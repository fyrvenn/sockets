#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <libubus.h>
#include <libubox/blob.h>
#include <libubox/blobmsg_json.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h> 

struct UdpHeader
{
    u_short src_port;
    u_short targ_port;
    u_short length;
    u_short checksum;
};

struct globalArgs_t {
	char *IP;  			/* -I option */
	const char * interface;		/* -i option */
	int Time;			/* -t option */

} globalArgs;

static const char *optString = "I:i:t:vh?";

static const struct option longOpts[] = {
	{ "ip", required_argument, NULL, 'I' },
	{ "interface", required_argument, NULL, 'i' },
	{ "time", required_argument, NULL, 't' },
	{ "version", no_argument, NULL, 'v' },
	{ "help", no_argument, NULL, 'h' },
	{ NULL, no_argument, NULL, 0 }
};

time_t rawtime;
struct tm * timeinfo;

pthread_mutex_t m_mutex;
void *dst, *thread_function();
static struct blob_buf b;
char *buf, **argv, *internet_addr;
int  sock, bbi, bytes_read = 0;
char str[INET_ADDRSTRLEN];
const char *arg_interface = "lo";
__useconds_t time_to_exit = 0;
struct stat{
    uint32_t pkts;
    uint32_t bytes;
} ;
struct ubus_context *ctx; 
static struct stat stat_gl  = {.pkts = 0, .bytes = 0};

void* read_stat(void *args)
{
    struct sockaddr_in addr, addr_remote;
    socklen_t s_t =  sizeof(addr_remote);    
    
    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = inet_addr(internet_addr);
    
    printf("Packets: %i, bytes: %i\n", stat_gl.pkts, stat_gl.bytes);
    while(1){
        bytes_read = recvfrom(sock, buf, 1024, 0, ((struct sockaddr *)&addr_remote), &s_t);
        inet_ntop(AF_INET, &(addr_remote.sin_addr), str, INET_ADDRSTRLEN);
        printf("Address: %s\n", str);
        if(addr_remote.sin_addr.s_addr == addr.sin_addr.s_addr)
        {
            pthread_mutex_lock(&m_mutex);
            stat_gl.bytes+=bytes_read;
            stat_gl.pkts++;
            pthread_mutex_unlock(&m_mutex);
            printf("Packets: %i, bytes: %i\n", stat_gl.pkts, stat_gl.bytes);  
        }                    
    }

}

static int ubus_send(struct ubus_context *ctx_, int argc)
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

    err = ubus_send_event(ctx_, "UDP_statistics", b.head);
    if (err) {
        printf("Cannot send!\n");
    }

	return err;
}

void sigint(int a)
{
    free(buf);
    free(dst);
    ubus_free(ctx);
    pthread_mutex_destroy(&m_mutex); 
    exit(2);
}

void display_usage( void )
{
    printf("\nUsage:\n ./ubus_receiver --time=TIME --ip=IP-ADDRESS --interface=INTERFACE...\n \n");
    printf("Parameters:\n");
    printf(" -I, --ip\t\tip-address (required)\n");
    printf(" -i, --interface\tnetwork interface (required)\n");
    printf(" -t, --time\t\twaiting time in milliseconds (required)\n\n");
    printf(" -h, --help\t\tdisplay this help\n -v, --version\t\tdisplay version\n");
    exit( EXIT_FAILURE );
}

void display_version( void )
{
	printf( "Version 2.0\n");  
	exit( EXIT_FAILURE );
}

int main(int argc, char **argv)
{    
    dst = malloc(sizeof(struct in6_addr));
    int opt = 0, longIndex =0;

    globalArgs.interface = NULL;
    globalArgs.IP = NULL;
    globalArgs.Time = 0;
    
    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );

    while( opt != -1 ) {
		switch( opt ) {
			case 'I':
				globalArgs.IP = optarg; //ip-address
				break;
				
			case 'i':
				globalArgs.interface = optarg; //network interface
				break;
				
			case 't':
				globalArgs.Time = atoi(optarg); //milliseconds of waiting
				break;
				
			case 'v':
                display_version();                              
				break;
				
			case 'h':	
			case '?':
			case 0:
				display_usage();
				break;
			
			default:
				
				break;
		}		
		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	}

    if ((globalArgs.interface == NULL)||(globalArgs.IP == NULL)||(globalArgs.Time == 0))
    {
        printf("Not enough arguments.\n");   
        display_usage();         
        exit(7);
    }
    
    int s = inet_pton(AF_INET, globalArgs.IP, dst);
    if (s <= 0) {
        if (s == 0)
            fprintf(stderr, "Internet address is in incorrectr format.\n");
        else
            perror("inet_pton");            
        exit(EXIT_FAILURE);
    }

    if (!sizeof(globalArgs.IP)==16) {    
        fprintf(stderr, "Failed to parse message data.\n");
        return -2;
    }

    buf = malloc(1024);  
    
    if (buf==NULL) 
    {
        perror("Cannot allocate buf\n");
        return -3;
    }

    memset(buf, 0, 1024);

    sock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }
        
    bbi = blob_buf_init(&b, 0);
    if (bbi!=0) {
        perror("Error blob buf init\n");		
        exit(5);
	} 
    const char *ubus_socket = NULL;   
    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
        exit(4);
	}

    time_to_exit = globalArgs.Time;
    printf("Wait %i milliseconds \n", time_to_exit);
    internet_addr = globalArgs.IP;
    printf("Internet address: %s\n", internet_addr);
    arg_interface = globalArgs.interface;
    printf("Network interface: %s\n", arg_interface);
    signal(SIGINT, sigint);
    pthread_t thread1, thread2;
    if (pthread_mutex_init(&m_mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    int f = 0;
    f = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, arg_interface, strlen(arg_interface));
    
    if (!(f==0)){
        perror("setsockopt failed");
        exit(5);
    }
    pthread_create(&thread1, NULL, read_stat, NULL);
    
    while(1)
    {
        pthread_mutex_lock(&m_mutex);
        if (ubus_send(ctx, 1)==1) {
            printf("Data sending failed!!\n");
        }
                
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        printf ( "Current local time and date: %s", asctime (timeinfo) );

        pthread_mutex_unlock(&m_mutex);
        usleep(time_to_exit*1000);

    }       
    return 0;
    
}
