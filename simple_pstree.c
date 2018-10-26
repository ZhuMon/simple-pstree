#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MY_NETLINK_TYPE 17

int main(int argc, char **argv)
{
    int cmd_opt = 0;
    char* msg = malloc(8); //max = 8
    char tmp[32];
    cmd_opt = getopt(argc, argv, "c::s::p::");

    printf("My pid = %d\n", getpid() );

    switch(cmd_opt) {
    default:
    case 'c':
        if(optarg) {

            tmp[0] = 'c';
            tmp[1] = ' ';
            msg = strcat(tmp, optarg);
            printf("%s\n", msg);
        } else {
            msg = "c 1";
            printf("%s\n", msg);
        }
        break;
    case 's':
        if(optarg) {
            tmp[0] = 's';
            tmp[1] = ' ';
            msg = strcat(tmp, optarg);
            printf("%s\n", msg);
        } else {
            sprintf(msg, "%d", getpid()); //int to string
            tmp[0] = 's';
            tmp[1] = ' ';
            strcat(tmp, msg);
            msg = tmp;
            printf("%s\n", msg);
        }
        break;
    case 'p':
        if(optarg) {
            tmp[0] = 'p';
            tmp[1] = ' ';
            msg = strcat(tmp, optarg);
            printf("%s\n", msg);
        } else {
            sprintf(msg, "%d", getpid());
            tmp[0] = 'p';
            tmp[1] = ' ';
            strcat(tmp, msg);
            msg = tmp;
            printf("%s\n", msg);
        }
        break;
    }

    int netlink_socket, err;
    struct sockaddr_nl src_addr, dst_addr;

    netlink_socket = socket(AF_NETLINK, SOCK_RAW, MY_NETLINK_TYPE);
    if(netlink_socket < 0) {
        perror("Link failed\n");
        return -1;
    }

    /*
    struct  sockaddr_nl {
    __kernel_sa_family_t     nl_family;   // AF_NETLINK
    unsigned short		 nl_pad;      // zero
    __u32			 nl_pid;      // port ID
    __u32			 nl_groups;   // multicast groups mask
    };
    */

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pad    = 0 ;
    src_addr.nl_pid    = getpid();
    src_addr.nl_groups = 0 ;

    err = bind(netlink_socket, (struct sockaddr *)&src_addr, sizeof(src_addr));
    if (err == -1 ) {
        perror( "bind failed\n" );
        close(netlink_socket);
        return -1;
    }

    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pad    = 0 ;
    dst_addr.nl_pid    = 0 ;
    dst_addr.nl_groups = 0 ;

    err = sendto(netlink_socket, msg, sizeof(msg), 0,
                 (struct sockaddr *)&dst_addr,
                 (socklen_t)sizeof(dst_addr));
    if (err == -1 ) {
        perror( "sendto failed\n" );
        close(netlink_socket);
        return - 1 ;
    }

    struct msghdr rc_msg; //received message
    struct nlmsghdr *nlh = NULL;
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(4096));
    if(!nlh) {
        perror( "malloc nlmsghdr failed\n");
        close(netlink_socket);
        return -1;
    }
    memset(nlh,0,NLMSG_SPACE(4096));

    //err = recvmsg(netlink_socket, &rc_msg, 0);
    err = recvfrom(netlink_socket, nlh, NLMSG_LENGTH(4096),0,NULL,NULL);
    if (err == -1) {
        perror( "receive failed\n");
        close(netlink_socket);
        return -1;
    }
    printf("Received message: %s\n",(char *) NLMSG_DATA(nlh));
    memset(nlh,0,NLMSG_SPACE(4096));



    close(netlink_socket);
    return 0;

}
