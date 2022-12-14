
#ifndef _MT6798_CCU_HW_H_
#define _MT6798_CCU_HW_H_

#include "ccu_reg.h"
#include "ccu_drv.h"
extern void cam_mtcmos_check(void);

/*spare register define*/
#define CCU_STA_REG_SW_INIT_DONE        CCU_INFO30
#define CCU_STA_REG_3A_INIT_DONE        CCU_INFO01
#define CCU_STA_REG_SP_ISR_TASK          CCU_INFO24
#define CCU_STA_REG_I2C_TRANSAC_LEN        CCU_INFO25
#define CCU_STA_REG_I2C_DO_DMA_EN         CCU_INFO26


#define CCU_DATA_REG_MAILBOX_APMCU        CCU_INFO00
#define CCU_DATA_REG_MAILBOX_CCU        CCU_INFO01
#define CCU_DATA_REG_LOG_BUF0                CCU_INFO02
#define CCU_DATA_REG_LOG_BUF1                CCU_INFO03

#endif
