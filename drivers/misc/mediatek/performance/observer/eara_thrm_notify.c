#define pr_fmt(fmt) "pob_eara_thrm_notifier: " fmt
#include <linux/notifier.h>
#include <mt-plat/mtk_perfobserver.h>

static BLOCKING_NOTIFIER_HEAD(pob_eara_thrm_notifier_list);

int pob_eara_thrm_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(
			&pob_eara_thrm_notifier_list, nb);
}

int pob_eara_thrm_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(
			&pob_eara_thrm_notifier_list, nb);
}

int pob_eara_thrm_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(
			&pob_eara_thrm_notifier_list, val, v);
}

int pob_eara_thrm_stats_update(enum pob_eara_thrm_info_num infonum)
{
	pob_eara_thrm_notifier_call_chain(infonum, NULL);

	return 0;
}

