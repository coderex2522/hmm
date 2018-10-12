#include "hmm_socket.h"

struct task_struct hmm_tcp_server_thread;

static int hmm_server_init(void)
{
    struct socket *server_sock = NULL, *client_sock = NULL;
    struct sockaddr_in s_addr;
    unsigned short port_num = 0x8888;
    int err = 0;
    struct kvec vec;
    struct msghdr msg;
    char *recvbuf = NULL;
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

    what = "create client sock";
    err = sock_create_kern(sock_net(server_sock->sk), server_sock->sk->sk_family,
                           server_sock->sk->sk_type, server_sock->sk->sk_protocol,
                           &client_sock);;
    if (err < 0)
        goto out;
    client_sock->type = server_sock->type;
    client_sock->ops = server_sock->ops;
    
    err = server_sock->ops->accept(server_sock, client_sock, 10);
    if (err < 0)
        goto out;

    pr_info("server: accept ok, Connection Established\n");

    /*kmalloc a receive buffer*/
    recvbuf = kmalloc(1024, GFP_KERNEL);
    if (recvbuf == NULL)
        goto out;
    memset(recvbuf, 0x45, 1024);
	recvbuf[1024]='\0';
	pr_info("recv info before ：%s\n",recvbuf);
	
    /*receive message from client*/
    memset(&vec, 0, sizeof(vec));
    memset(&msg, 0, sizeof(msg));
    vec.iov_base = recvbuf;
    vec.iov_len = 1024;
    kernel_recvmsg(client_sock, &msg, &vec, 1, 1024, 0); /*receive message*/
	recvbuf[1024]='\0';
    pr_info("receive message: %s\n", recvbuf);

out:
    if (err < 0)
        pr_err("error exist in %s phase\n", what);
    if (recvbuf != NULL)
        kfree(recvbuf);
    if (client_sock)
        sock_release(client_sock);
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
	
	what="connect server";
	err = client_sock->ops->connect(client_sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr), 0);
	if (err < 0)
		goto out;	   

	what="malloc sendbuf";
	sendbuf = kmalloc(1024, GFP_KERNEL);
	if (sendbuf == NULL)
		goto out;
	
	what="send message";
	memset(sendbuf, 0x32, 1024);
	vec.iov_base = sendbuf;
	vec.iov_len = 1024;
	memset(&msg, 0, sizeof(msg));
	err = kernel_sendmsg(client_sock, &msg, &vec, 1, 1024); 			 
	if (err < 0)
		goto out;
	else if (err != 1024)
		pr_info("client: send message len less than 1024\n");

out:
	if(err)
		pr_err("error arise in %s phase\n",what);
	if(sendbuf)
		kfree(sendbuf);
	if(client_sock)
		sock_release(client_sock);
    return 0;
}

static void hmm_server_cleanup(void)
{
    printk("%s clean up.\n", __func__);
}

module_init(hmm_server_init);
module_exit(hmm_server_cleanup);

