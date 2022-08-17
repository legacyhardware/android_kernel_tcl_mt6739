
#ifndef __APUSYS_DEBUG_H__
#define __APUSYS_DEBUG_H__

extern bool apusys_dump_force;
extern bool apusys_dump_skip;

void apusys_dump_init(void);
void apusys_reg_dump(void);
void apusys_dump_exit(void);
int apusys_dump_show(struct seq_file *sfile, void *v);
void apusys_dump_reg_skip(int onoff);

#endif
