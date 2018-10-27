#ifndef KSIMPLE_PSTREE
#define KSIMPLE_PSTREE

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/list.h>
#include <linux/netlink.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/socket.h>

#define MY_NETLINK_TYPE 17
#define MAX_MSGSIZE 32768

struct ts_list {
    struct ts_list *prev, *next;
    struct task_struct *task;
};

void send_to_user(char msg[], int pid);
static void my_find_children(struct list_head *head, int count);
static void nl_input(struct sk_buff *skb);
#endif
