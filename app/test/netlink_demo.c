#include <linux/netlink.h>
#include <linux/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#define DISABLE_SCHEDULE_TEST _IOR('W', 11, int)
#define DISABLE_INTERRUPT_TEST _IOR('W', 12, int)

/* header part in struct diag_msg */
struct diag_msg_header
{
    uint8_t msg_type;  // 1:with env, 0:without env
    uint16_t msg_len;  // message length
    uint32_t checksum; // checksum of the data
};

/* data part in struct diag_msg */
struct diag_msg_data
{
    uint16_t module_id;
    uint16_t event_id;
    uint8_t event_sta; // event status
    uint32_t sec;      // record occurrence_time of the event
    uint16_t msec;
    uint8_t event_level;
    uint16_t err_code;
    uint8_t payload[128];
    uint8_t env_len;
};

/*
 * message send from kernel space to user space
 */
struct diag_msg
{
    struct diag_msg_header header;
    struct diag_msg_data data;
};

/*
 *diag write/read netlink info
 */
struct diag_netlink_ctl
{
    uint16_t port;    // pid
    uint8_t protocol; // 协议 大于21，小于32
};

#define MAX_PAYLOAD (150)

int wdg_fd0;
int wdg_fd1;
int stop;
pthread_t thread1;
int sock_fd;
struct nlmsghdr *nlh = NULL;
int char_fd;

void print_msg_info(struct diag_msg *msg)
{
    int i;
    struct timeval tv;
    printf("==============================msg info:===============================\n");
    gettimeofday(&tv, NULL);
    printf("kernel ts:%d.%03d, user space ts:%d.%03d, diff:%d ms\n", msg->data.sec, msg->data.msec, tv.tv_sec, tv.tv_usec / 1000,
           (tv.tv_sec - msg->data.sec) * 1000 + (tv.tv_usec / 1000 - msg->data.msec));
    printf("msg->header.msg_type: %d\n", msg->header.msg_type);
    printf("msg->header.msg_len: %d\n", msg->header.msg_len);
    printf("msg->header.checksum: %d\n", msg->header.checksum);
    printf("msg->data.module_id: %d\n", msg->data.module_id);
    printf("msg->data.event_id: %d\n", msg->data.event_id);
    printf("msg->data.event_sta: %d\n", msg->data.event_sta);
    printf("msg->data.sec: %d\n", msg->data.sec);
    printf("msg->data.msec: %d\n", msg->data.msec);
    printf("msg->data.event_level: %d\n", msg->data.event_level);
    printf("msg->data.err_code: %d\n", msg->data.err_code);
    for (i = 0; i < msg->data.env_len; i++)
    {
        printf("msg->data.payload[%d]: %d\n", i, msg->data.payload[i]);
    }
    printf("msg->data.env_len: %d\n", msg->data.env_len);
    printf("========================================================================\n");
}

void parse_data(unsigned char *buffer)
{
    struct diag_msg msg;
    int i;
    msg.header.msg_type = buffer[0];

    msg.header.msg_len = (buffer[1] << 8) + (buffer[2]);
    printf("buffer[1]:%x, buffer[2]:%x, msg.header.msg_len:%d\n", buffer[1], buffer[2], msg.header.msg_len);

    msg.header.checksum = (buffer[6]) + ((buffer[5]) << 8) + ((buffer[4]) << 16) + ((buffer[3]) << 24);
    printf("buffer[3]:%x, buffer[4]:%x, buffer[5]:%x, buffer[6]:%x\n", buffer[3], buffer[4], buffer[5], buffer[6]);

    msg.data.module_id = ((buffer[7]) << 8) + (buffer[8]);
    printf("buffer[7]:%x, buffer[8]:%x\n", buffer[7], buffer[8]);

    msg.data.event_id = ((buffer[9]) << 8) + (buffer[10]);
    printf("buffer[9]:%x, buffer[10]:%x\n", buffer[9], buffer[10]);

    msg.data.event_sta = buffer[11];
    printf("buffer[11]:%x", buffer[11]);

    msg.data.sec = (buffer[15]) + ((buffer[14]) << 8) + ((buffer[13]) << 16) + ((buffer[12]) << 24);
    printf("buffer[12]:%x, buffer[13]:%x, buffer[14]:%x, buffer[15]:%x\n", buffer[12], buffer[13], buffer[14], buffer[15]);

    msg.data.msec = ((buffer[16]) << 8) + (buffer[17]);
    printf("buffer[16]:%x, buffer[17]:%x\n", buffer[16], buffer[17]);

    msg.data.event_level = buffer[18];
    printf("buffer[18]:%x\n", buffer[18]);

    msg.data.err_code = ((buffer[19]) << 8) + (buffer[20]);
    printf("buffer[19]:%x, buffer[20]:%x\n", buffer[19], buffer[20]);

    msg.data.env_len = buffer[21 + 128];
    printf("buffer[149]:%x\n", buffer[21 + 128]);

    for (i = 0; i < msg.data.env_len; i++)
    {
        msg.data.payload[i] = buffer[21 + i];
    }
    print_msg_info(&msg);
}

void *select_test(void *arg)
{
    int choice;

    wdg_fd0 = open("/dev/wdg-stl0", O_RDWR);
    wdg_fd1 = open("/dev/wdg-stl1", O_RDWR);
    while (!stop)
    {
        scanf("%d", &choice);
        if (choice == 1)
        {
            ioctl(wdg_fd0, DISABLE_SCHEDULE_TEST);
            ioctl(wdg_fd1, DISABLE_SCHEDULE_TEST);
        }
        else if (choice == 2)
        {
            ioctl(wdg_fd0, DISABLE_INTERRUPT_TEST);
            ioctl(wdg_fd1, DISABLE_INTERRUPT_TEST);
        }
        else if(choice == 0){
            stop = 1;
        }
    }
}

int main()
{
    struct sockaddr_nl src_addr;

    struct iovec iov;
    struct msghdr msg;
    pid_t pid;

    struct diag_netlink_ctl ctl;
    char_fd = open("/dev/diagnosis", O_RDWR);

    read(char_fd, &ctl, sizeof(struct diag_netlink_ctl));
    printf("first read port:%d, protocol:%d\n", ctl.port, ctl.protocol);

    pid = getpid();
    ctl.port = pid;
    write(char_fd, &ctl, sizeof(struct diag_netlink_ctl));
    printf("write port:%d, protocol:%d\n", ctl.port, ctl.protocol);

    memset(&ctl, 0, sizeof(struct diag_netlink_ctl));
    read(char_fd, &ctl, sizeof(struct diag_netlink_ctl));
    printf("second read port:%d, protocol:%d\n", ctl.port, ctl.protocol);

    // 创建 Net socket
    sock_fd = socket(AF_NETLINK, SOCK_RAW, ctl.protocol);
    if (sock_fd < 0)
    {
        perror("socket creation failed");
        return -1;
    }
    memset(&src_addr, 0, sizeof(struct sockaddr_nl));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = ctl.port;
    // 绑定 socket
    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0)
    {
        perror("bind failed");
        close(sock_fd);
        return -1;
    }
    memset(&msg, 0, sizeof(msg));
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    stop = 0;

    if (pthread_create(&thread1, NULL, select_test, NULL) != 0)
    {
        fprintf(stderr, "create thread1_select_test failed\n");
        exit(1);
    }

    // 接收消息
    printf(" for message from kernel...\n");
    while (!stop){
        recvmsg(sock_fd, &msg, 0);
        parse_data((unsigned char *)NLMSG_DATA(nlh));
    }
    pthread_join(thread1, NULL);
    close(wdg_fd0);
    close(wdg_fd1);
    close(sock_fd);
    close(char_fd);
    free(nlh);
    return 0;
}
