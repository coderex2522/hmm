#ifndef _HMM_CTX_H
#define _HMM_CTX_H
#include "hmm.h"

struct hmm_context{
	int epfd;
	struct ib_device *dev;
	struct ib_pd *pd;
	struct ib_mr *mr;
};

#endif
