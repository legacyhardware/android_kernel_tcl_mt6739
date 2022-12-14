
#ifndef __CMDQ_SEC_TRUSTZONE_H__
#define __CMDQ_SEC_TRUSTZONE_H__

#include "tz_cross/trustzone.h"
#include "tz_cross/ta_mem.h"
#include "trustzone/kree/system.h"
#include "trustzone/kree/mem.h"
#include "cmdq_sec_iwc_common.h"

/* context for inhouse tee vendor */
struct cmdq_sec_tee_context {
	/* Universally Unique Identifier of secure tl/dr */
	char uuid[100];

	KREE_SESSION_HANDLE session;
	KREE_SESSION_HANDLE mem_session;

	struct iwcCmdqMessage_t *share_memory;
	struct iwcCmdqMessageEx_t *wsm_buf_ex;
	struct iwcCmdqMessageEx2_t *wsm_buf_ex2;
};

s32 cmdq_sec_trustzone_create_share_memory(void **va, u32 *pa, u32 size);
void cmdq_sec_alloc_iwc_buffer(void);


#endif	/* __CMDQ_SEC_TRUSTZONE_H__ */
