#ifndef __MT_OTP_H__
#define __MT_OTP_H__
#include <linux/types.h>
// CCCI Command
#define CCCI_CMD_EFUSE_BLOW_START          0x10000000
#define CCCI_CMD_EFUSE_BLOW_END            0x20000000
#define CCCI_CMD_EFUSE_IS_BLOW_LOCKED      0x40000000
// OTP Error Code
#define STATUS_DONE                        0x0
#define ERR_MUTEX_LOCKED_NEED_RETRY        0x1
#define ERR_PMIC_VCORE_DVFS_NOT_READY      0x2
#define ERR_PMIC_REQUEST_VCORE_DVFS_HPM    0x3
#define ERR_PMIC_RELEASE_VCORE_DVFS_HPM    0x4
#define ERR_PMIC_VCORE_DVFS_DISABLED       0x5
#define ERR_PMIC_CONFIG_FSOURCE            0x6
#define ERR_SHOULD_CALL_BLOW_START_FIRST   0x7
#define ERR_UNSUPPORTED_CCCI_CMD           0x8
extern u32 otp_ccci_handler(u32 ccci_cmd);
#endif // __MT_OTP_H__
