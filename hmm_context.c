#include "hmm_context.h"

static struct task_struct *context_handler_thread;
static struct hmm_context *ctx;

static int hmm_context_run(void *data)
{
	struct hmm_context *ctx=(struct hmm_context*)data;

	while(1){
		pr_info("run context handler.\n");
		msleep(2000);
	}
	return 0;
}

static int hmm_context_init(void)
{
	int epfd=sys_epoll_create(1024);

	ctx=kmalloc(sizeof(struct hmm_context),GFP_KERNEL);
	if(!ctx)
		goto out;
	ctx->epfd=epfd;
	
	context_handler_thread=kthread_run(hmm_context_run,ctx,"context_handler");
	if(IS_ERR(context_handler_thread)){
		pr_err("create context handler thread error.\n");
		goto out_release_ctx;
	}
	return 0;
out_release_ctx:
	kfree(ctx);
out:
	return 1;
}

static void hmm_context_cleanup(void)
{
	if(!IS_ERR(context_handler_thread))
		kthread_stop(context_handler_thread);
	
	if(ctx)
		kfree(ctx);
	printk("%s clean up.\n", __func__);
}

module_init(hmm_context_init);
module_exit(hmm_context_cleanup);

