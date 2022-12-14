
#include <linux/proc_fs.h>

#include "boost_ctrl.h"

int init_boostctrl(struct proc_dir_entry *parent)
{
	struct proc_dir_entry *bstctrl_root = NULL;

	pr_debug("__init %s\n", __func__);


	bstctrl_root = proc_mkdir("boost_ctrl", parent);

    /* init topology info first */
	topo_ctrl_init(bstctrl_root);

	cpu_ctrl_init(bstctrl_root);

	dram_ctrl_init(bstctrl_root);

	eas_ctrl_init(bstctrl_root);

	return 0;
}
