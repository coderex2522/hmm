#include "hmm.h"

MODULE_AUTHOR("huststephen");
MODULE_LICENSE("GPL");

struct task_struct *thread_cq_poller;

struct hmm_context *hmm_ctx;

int hmm_waiting_queue_process(void *data)
{
	int time_count=0;

	do{
		pr_info("hmm waiting process %d\n",++time_count);
		msleep(1000);
	}while(!kthread_should_stop()&&time_count<10);

	return time_count;
}

void hmm_init_ctx(struct hmm_context *ctx)
{
	//struct hmm_context *ctx=NULL;
	//thread_cq_poller=kthread_run(hmm_waiting_queue_process,NULL,"hmm waiting queue process");
	ctx->pd=ib_alloc_pd(ctx->dev);
	if(!ctx->pd){
		pr_err("create pd error.\n");
		return ;
	}

	ctx->mr=ib_get_dma_mr(ctx->pd, IB_ACCESS_LOCAL_WRITE | IB_ACCESS_REMOTE_WRITE | IB_ACCESS_REMOTE_READ | IB_ACCESS_REMOTE_ATOMIC);
	if(!ctx->mr){
		pr_err("create mr error.\n");
		goto clear_pd;
	}

	return ;
clear_pd:
	ib_dealloc_pd(ctx->pd);
	return ;
}

static void hmm_client_add_one_dev(struct ib_device *dev)
{
#ifdef TEST_IN_VM
	if(strcmp(dev->name, "siw_ens33"))
		return ;
#endif

	pr_info("we get a new device:[%s].\n",dev->name);
	hmm_ctx->dev=dev;
	
	return ;
}

static void hmm_client_remove_one_dev(struct ib_device *dev, void *ctx)
{
#ifdef TEST_IN_VM
	if(strcmp(dev->name, "siw_ens33"))
		return ;
#endif
	ib_dealloc_pd(hmm_ctx->pd);

	pr_info("we remove a device:[%s].\n",dev->name);	
	return ;
}

static struct ib_client hmm_client = {
	.name = "hmm server",
	.add = hmm_client_add_one_dev,
	.remove = hmm_client_remove_one_dev
};
	
static int __init hmm_init_module(void)
{
	int ret;

	hmm_ctx=(struct hmm_context*)kmalloc(sizeof(struct hmm_context),GFP_KERNEL);

	ret=ib_register_client(&hmm_client);
	if(ret){
		pr_err("ib register error.\n");
		return ret;
	}
	hmm_init_ctx(hmm_ctx);
	return 0;
}

static void __exit hmm_cleanup_module(void)
{
	ib_unregister_client(&hmm_client);
}

module_init(hmm_init_module);
module_exit(hmm_cleanup_module);
