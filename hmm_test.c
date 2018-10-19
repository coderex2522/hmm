#include "hmm.h"
static struct task_struct *test_handler_thread=NULL;

static int hmm_test_run(void *data)
{
	int cnt=0;
	while(1&&!kthread_should_stop()){
		;
	}
	return 0;
}
static int hmm_test_init(void)
{
	int err=0;
	test_handler_thread=kthread_run(hmm_test_run,NULL,"hmm test run");
	if(IS_ERR(test_handler_thread))
		err=1;
	pr_info("hmm test state1 %ld\n",test_handler_thread->state);
	return err;
}

static void hmm_test_clean(void)
{
	pr_info("%s start\n",__func__);

	if(test_handler_thread){
		pr_info("hmm test state %ld\n",test_handler_thread->state);
		kthread_stop(test_handler_thread);
	}
	pr_info("%s end\n",__func__);
}

module_init(hmm_test_init);
module_exit(hmm_test_clean);

