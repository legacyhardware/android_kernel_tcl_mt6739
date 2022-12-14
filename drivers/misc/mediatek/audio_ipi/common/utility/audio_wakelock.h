

#ifndef AUDIO_WAKELOCK_H
#define AUDIO_WAKELOCK_H


#include <linux/device.h>
#include <linux/pm_wakeup.h>

/* wake lock relate*/
#define aud_wake_lock_init(ws, name) wakeup_source_init(ws, name)
#define aud_wake_lock_destroy(ws) wakeup_source_trash(ws)
#define aud_wake_lock(ws) __pm_stay_awake(ws)
#define aud_wake_unlock(ws) __pm_relax(ws)

#endif
