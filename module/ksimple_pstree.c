#include "ksimple_pstree.h"

MODULE_LICENSE("Dual BSD/GPL");

struct sock *my_nl_sock = NULL ;
char msg_to_send[32768] = {};
int msg_end = 0;

void send_to_user(char msg[], int pid)
{
    struct sk_buff *skb_1;
    struct nlmsghdr *nlh;
    int len = NLMSG_SPACE(MAX_MSGSIZE);
    int slen = 0;

    if(!msg || !my_nl_sock) {
        return ;
    }
    //printk(KERN_ERR "pid:%d\n",pid);
    skb_1 = alloc_skb(len,GFP_KERNEL);

    if(!skb_1) {
        printk(KERN_ERR "my_net_link:alloc_skb error\n");
    }

    for(slen = 0; msg[slen] != '\0'; ++slen);//count size of msg

    nlh = __nlmsg_put(skb_1,0,0,0,MAX_MSGSIZE,0);
    NETLINK_CB(skb_1).portid = 0;
    NETLINK_CB(skb_1).dst_group = 0;
    msg[slen]= '\0';
    memcpy(NLMSG_DATA(nlh),msg,slen+1);
    //pr_info("my_net_link:send message '%s'.\n",(char *)NLMSG_DATA(nlh));
    netlink_unicast(my_nl_sock,skb_1,pid,MSG_DONTWAIT);
}

static void my_find_children(struct list_head *head, int count)
{
    struct list_head *pos = NULL;
    struct task_struct *now_task = NULL;

    list_for_each(pos, head) {
        int i;
        char cp_pr[64]= {}; //completed process name (blank + process)
        char *four_blank = "    ";
        char pid_str[5] = {};

        now_task = list_entry(pos, struct task_struct, sibling);

        for(i = 0; i < count; ++i) {
            strcat(cp_pr, four_blank);
        }

        if(now_task -> pid != 0) { //may have problem, but so far so good
            pr_info( "%s%s(%d)\n", cp_pr, now_task -> comm, now_task -> pid);
            strcat(cp_pr, now_task -> comm);
            for(i = 0; cp_pr[i] != '\0'; i++) {
                msg_to_send[msg_end++] = cp_pr[i];
            }
            msg_to_send[msg_end++] = '(';
            sprintf(pid_str, "%d", now_task->pid);

            for(i = 0; pid_str[i] != '\0'; i++) {
                msg_to_send[msg_end++] = pid_str[i];
            }
            msg_to_send[msg_end++] = ')';
            msg_to_send[msg_end++] = '\n';

        } else {
            return;
        }
        my_find_children(&now_task->children, count+1);
    }

}

static  void  nl_input (struct sk_buff *skb)
{

    int i;
    int rc_pid; //received pid

    struct pid *pid_struct = NULL;
    struct task_struct *my_pid_task = NULL,
                            *now_task = NULL;
    struct ts_list *p_list = NULL,
                        *now_list = NULL;
    struct list_head *my_sibling = NULL;

    char mode;
    char blank[64] = {};
    char *four_blank = "    ";
    char pid_str[5] = {};

    pr_info( "recv msg len: %d\n", skb->len);

    if (skb->data[skb->len - 1 ] == '\0' )
        pr_info( "recv msg str: %s\n", skb->data);

    pr_info( "portid : %d\n", NETLINK_CB(skb).portid);
    pr_info( "dst_group: %d\n", NETLINK_CB(skb).dst_group);

    mode = skb->data[0];
    skb->data[0] = ' ';
    skb->data = skip_spaces(skb->data);

    pr_info( "skb->data = %s\n", skb->data);
    pr_info( "data's len = %ld\n", strlen(skb->data));

    if(!kstrtoint(skb->data, 0, &rc_pid)) {
        pr_info( "kstrtoint success:%d\n", rc_pid);
        pid_struct = find_get_pid(rc_pid);
    } else {
        pr_info( "kstrtoint failed\n");
    }

    my_pid_task = pid_task(pid_struct, PIDTYPE_PID);

    p_list = kmalloc(32, GFP_KERNEL );
    now_list = kmalloc(32, GFP_KERNEL );

    //clean msg
    for(i = 0; i < msg_end; i++) {
        msg_to_send[i] = '\0';
    }
    msg_end = 0;

    switch(mode) {
    case 'c':
        pr_info( "%s(%d)\n", my_pid_task -> comm, my_pid_task -> pid);

        for(i = 0; my_pid_task->comm[i] != '\0'; i++) {
            msg_to_send[msg_end++] = my_pid_task->comm[i];
        }
        msg_to_send[msg_end++] = '(';
        sprintf(pid_str, "%d", my_pid_task->pid);

        for(i = 0; pid_str[i] != '\0'; i++) {
            msg_to_send[msg_end++] = pid_str[i];
        }
        msg_to_send[msg_end++] = ')';
        msg_to_send[msg_end++] = '\n';

        my_find_children(&my_pid_task->children, 1);

        break;
    case 's':
        //mode_s(msg_to_send, &my_pid_task->sibling);
        //pr_info("pid_task:%d\n", my_pid_task->pid);
        list_for_each(my_sibling, &my_pid_task->sibling) {
            now_task = list_entry(my_sibling, struct task_struct, sibling);

            if(now_task -> pid < 100000 && now_task -> pid > 0) {
                pr_info( "%s(%d)\n",now_task -> comm, now_task -> pid);
                //} else {
                //break;

                for(i = 0; now_task->comm[i] != '\0'; i++) {
                    msg_to_send[msg_end++] = now_task->comm[i];
                    /*if(i == 15){
                    break;
                    }*/
                }
                //pr_info("i = %d\n", i);
                msg_to_send[msg_end++] = '(';
                sprintf(pid_str, "%d", now_task->pid);
                //pr_info("pid_str:%saaa\n", pid_str);

                for(i = 0; pid_str[i] != '\0'; i++) {
                    msg_to_send[msg_end++] = pid_str[i];
                    /*if(i == 5){
                    break;
                    }*/
                }
                //pr_info("i = %d\n", i);
                msg_to_send[msg_end++] = ')';
                msg_to_send[msg_end++] = '\n';

            }
        }
        break;
    case 'p':

        p_list -> task = my_pid_task;
        now_list = p_list;
        now_task = my_pid_task;
        //for(i = 0; i < 5; i++) {
        while(1) {
            struct ts_list *new_list;
            new_list = kmalloc(32, GFP_KERNEL );

            now_list -> next = new_list;
            new_list -> prev = now_list;
            new_list -> task = now_task -> parent;

            if(now_task -> pid == 1) {
                break;
            }

            now_list = new_list;
            now_task = now_task -> parent;
        }

        for(i = 0; now_list->task->comm[i] != '\0'; i++) {
            msg_to_send[msg_end++] = now_list->task->comm[i];
        }
        msg_to_send[msg_end++] = '(';
        sprintf(pid_str, "%d", now_list->task->pid);

        for(i = 0; pid_str[i] != '\0'; i++) {
            msg_to_send[msg_end++] = pid_str[i];
        }
        msg_to_send[msg_end++] = ')';
        msg_to_send[msg_end++] = '\n';

        pr_info( "%s(%d)\n", now_list->task->comm, now_list->task->pid);
        now_list = now_list -> prev;
        //for(i = 0; i < 5; i++) {
        while(1) {
            char tmp[64] = {};
            strcat(blank, four_blank);
            pr_info( "%s%s(%d)\n", blank, now_list->task->comm, now_list->task->pid);
            strcat(tmp, blank);
            strcat(tmp, now_list -> task -> comm);
            for(i = 0; tmp[i] != '\0'; i++) {
                msg_to_send[msg_end++] = tmp[i];
            }
            msg_to_send[msg_end++] = '(';
            sprintf(pid_str, "%d", now_list->task->pid);

            for(i = 0; pid_str[i] != '\0'; i++) {
                msg_to_send[msg_end++] = pid_str[i];
            }
            msg_to_send[msg_end++] = ')';
            msg_to_send[msg_end++] = '\n';

            if(now_list == p_list) {
                break;
            }

            now_list = now_list -> prev;
        }
        break;

    }

    send_to_user(msg_to_send, NETLINK_CB(skb).portid);


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
