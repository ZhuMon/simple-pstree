#include <linux/init.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/list.h>
#include <linux/netlink.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/sched.h>

#define MY_NETLINK_TYPE 17
/*
   struct pid
   {
	atomic_t count;
	unsigned int level;
	// lists of tasks that use this pid
	struct hlist_head tasks[PIDTYPE_MAX];
	struct rcu_head rcu;
	struct upid numbers[1];
    };
 */

MODULE_LICENSE("Dual BSD/GPL");

struct  sock *my_nl_sock = NULL ;
static  void  nl_input (struct sk_buff *skb)
{

    pr_info( "recv msg len: %d\n", skb->len);

    if (skb->data[skb->len - 1 ] == '\0' )
        pr_info( "recv msg str: %s\n", skb->data);

    pr_info( "portid : %d\n", NETLINK_CB(skb).portid);
    pr_info( "dst_group: %d\n", NETLINK_CB(skb).dst_group);

    struct pid *pid_struct;
    struct task_struct *my_pid_task;

    int rc_pid; //received pid
    //int a; //for test
    char mode;


    mode = skb->data[0];
    skb->data[0] = ' ';
    skb->data = skip_spaces(skb->data);

    pr_info( "skb->data = %s\n", skb->data);
    pr_info( "data's len = %ld\n", strlen(skb->data));

    if(!kstrtoint(skb->data, 0, &rc_pid)) {

        pr_info( "kstrtoint success:%d\n", rc_pid);
        pid_struct = find_get_pid(rc_pid);

        //a = pid_struct -> level;
        //pr_info( "receive pid:%d\n",a);
    } else {
        pr_info( "kstrtoint failed\n");

    }

    my_pid_task = pid_task(pid_struct, PIDTYPE_PID);

    struct list_head *my_child, *my_sibling;
    struct task_struct *now_task;
    switch(mode) {
    case 'c':
        //list_for_each(my_child, &my_pid_task->children);
        list_for_each(my_child, &my_pid_task->children) {
            now_task = list_entry(my_child, struct task_struct, children);
            if(my_child -> next != my_child) {
                pr_info( "%s(%d)\n",now_task -> comm, now_task -> pid);
            }
        }
        //my_child = my_pid_task -> children ->next;
        //if(my_child == &my_pid_task -> children){
        //	pr_info("fuck\n");
        //}
        //now_task = list_entry(my_child, struct task_struct, children);
        //pr_info( "%s(%d)\n",my_pid_task -> comm, my_pid_task-> pid);

        //pr_info( "%s(%d)\n",now_task -> comm, now_task -> pid);

        break;
    case 's':
        //list_for_each(my_sibling, &my_pid_task->sibling);
        //now_task = list_entry(my_sibling, struct task_struct, sibling);
        //pr_info( "%s(%d)\n",my_pid_task->comm, my_pid_task->pid);
        //pr_info( "%s(%d)\n",now_task ->comm, now_task->pid);

        list_for_each(my_sibling, my_pid_task->sibling.prev) {
            now_task = list_entry(my_sibling, struct task_struct, sibling);
            if(my_sibling -> next != my_sibling) {
                pr_info( "%s(%d)\n",now_task -> comm, now_task -> pid);
            }
        }
        break;
    case 'p':
        now_task = my_pid_task -> parent;
        int i;
        char blank[64];
        char *four_blank = "    ";
        for(i = 0; i < 5; i++) {
            //pr_info( "%s(%d)\n",my_pid_task->comm, my_pid_task->pid);
            pr_info( "%s%s(%d)\n",blank, now_task ->comm, now_task->pid);
            if(now_task -> pid == 1) {
                break;
            }
            now_task = now_task -> parent;
            strcat(blank, four_blank);
        }
        break;

    }


    pr_info( "====================\n" );
}

static int my_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .groups = 0,
        .input = nl_input,
    };
    pr_info( "my init.\n" );

    my_nl_sock = netlink_kernel_create(&init_net, MY_NETLINK_TYPE, &cfg);
    if (!my_nl_sock) {
        pr_info( "nl create failed\n" );
        pr_info( "====================\n" );
        return -1;
    }

    pr_info( "====================\n" );



    return 0;
}
static void my_exit(void)
{
    pr_info( "my exit.\n" );
    if (my_nl_sock) {
        netlink_kernel_release(my_nl_sock);
    }
    pr_info( "====================\n" ) ;

    //printk(KERN_ALERT "Goodbye, cruel world\n");
}
module_init(my_init);
module_exit(my_exit);
