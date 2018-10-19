#include "hmm_socket.h"
#define HMM_RECV_MESSAGE_LEN 32

struct task_struct *hmm_tcp_server_thread = NULL;

static int hmm_server_run(void *data)
{
    struct socket *server_sock = NULL, *client_sock = NULL;
    char *recvbuf = NULL;
	char datac=0x45;
    struct kvec vec;
    struct msghdr msg;
    int err = 0;

    server_sock = (struct socket *)data;
    if (!server_sock)
        goto out;

    recvbuf = kmalloc(HMM_RECV_MESSAGE_LEN, GFP_KERNEL);
    if (recvbuf == NULL)
        goto out;

    /*receive message from client*/
    memset(&vec, 0, sizeof(vec));
    memset(&msg, 0, sizeof(msg));
    vec.iov_base = recvbuf;
    vec.iov_len = HMM_RECV_MESSAGE_LEN;

    pr_info("%s is running.\n", __func__);

	while(!kthread_should_stop()){
		err = sock_create_kern(sock_net(server_sock->sk), server_sock->sk->sk_family,
                           server_sock->sk->sk_type, server_sock->sk->sk_protocol,
                           &client_sock);
    	if (err < 0)
        	goto out;
    	client_sock->type = server_sock->type;
    	client_sock->ops = server_sock->ops;

		memset(recvbuf, datac, HMM_RECV_MESSAGE_LEN);
		recvbuf[HMM_RECV_MESSAGE_LEN]='\0';
		pr_info("memset message: %s\n", recvbuf);
		datac++;
    	err = server_sock->ops->accept(server_sock, client_sock, 10);
    	if (err < 0)
        	goto out;
    	kernel_recvmsg(client_sock, &msg, &vec, 1, HMM_RECV_MESSAGE_LEN, 0);
    	recvbuf[HMM_RECV_MESSAGE_LEN] = '\0';
    	pr_info("receive message: %s\n", recvbuf);

    	sock_release(client_sock);
	}
    
out:
    if (server_sock)
        sock_release(server_sock);
    if (recvbuf)
        kfree(recvbuf);
    return 0;
}

static int hmm_server_init(void)
{
    struct socket *server_sock = NULL;
    struct sockaddr_in s_addr;
    unsigned short port_num = 0x8888;
    int err = 0;
    const char *what;

    /*create a socket*/
    what = "create sock";
    err = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &server_sock);
    if (err < 0)
        goto out;

    /*init server listen tcp port*/
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port_num);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*bind the socket*/
    what = "bind sock";
    err = server_sock->ops->bind(server_sock, (struct sockaddr *)&s_addr,
                                 sizeof(struct sockaddr_in));
    if (err < 0)
        goto out;

    /*listen*/
    what = "listen sock";
    err = server_sock->ops->listen(server_sock, 10);
    if (err < 0)
        goto out;

    what = "create kernel thread";
    hmm_tcp_server_thread = kthread_run(hmm_server_run, server_sock,
                                        "hmm_tcp_server_thread");
    if (IS_ERR(hmm_tcp_server_thread))
        goto out;
    pr_info("hmm_server_init end.\n");
    return 0;
out:
    if (err < 0)
        pr_err("error exist in %s phase\n", what);
    if (server_sock)
        sock_release(server_sock);

    return err;
}


static int hmm_client_init(void)
{
    struct socket *client_sock;
    struct sockaddr_in sock_addr;
    unsigned short port_num = 0x8888;
    int err = 0;
    struct kvec vec;
    struct msghdr msg;
    char *sendbuf = NULL;
    const char *what;

    /*create a socket*/
    what = "create sock";
    err = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &client_sock);
    if (err < 0)
        goto out;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port_num);
    sock_addr.sin_addr.s_addr = in_aton("192.168.77.134");

    what = "connect server";
    err = client_sock->ops->connect(client_sock, (struct sockaddr *)&sock_addr,
                                    sizeof(sock_addr), 0);
    if (err < 0)
        goto out;

    what = "malloc sendbuf";
    sendbuf = kmalloc(HMM_RECV_MESSAGE_LEN, GFP_KERNEL);
    if (sendbuf == NULL)
        goto out;

    what = "send message";
    memset(sendbuf, 0x32, HMM_RECV_MESSAGE_LEN);
    vec.iov_base = sendbuf;
    vec.iov_len = HMM_RECV_MESSAGE_LEN;
    memset(&msg, 0, sizeof(msg));
    err = kernel_sendmsg(client_sock, &msg, &vec, 1, HMM_RECV_MESSAGE_LEN);
    if (err < 0)
        goto out;
    else if (err != HMM_RECV_MESSAGE_LEN)
        pr_info("client: send message len less than 1024\n");

out:
    if (err)
        pr_err("error arise in %s phase\n", what);
    if (sendbuf)
        kfree(sendbuf);
    if (client_sock)
        sock_release(client_sock);
    return 0;
}

static void hmm_server_cleanup(void)
{
    if (hmm_tcp_server_thread && hmm_tcp_server_thread->state==0)
        kthread_stop(hmm_tcp_server_thread);
    printk("%s clean up.\n", __func__);
}

module_init(hmm_client_init);
module_exit(hmm_server_cleanup);

