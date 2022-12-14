
#include <linux/pm_qos.h>

static struct pm_qos_request ddr_opp_req;

void secure_perf_init(void)
{
	pm_qos_add_request(&ddr_opp_req, PM_QOS_DDR_OPP,
						PM_QOS_DDR_OPP_DEFAULT_VALUE);
}

void secure_perf_remove(void)
{
	pm_qos_remove_request(&ddr_opp_req);
}

void secure_perf_raise(void)
{
	/* Fix DDR to higher frequency (OPP 3) */
	//pm_qos_update_request(&ddr_opp_req, 3);

}

void secure_perf_restore(void)
{
	/* Reset DDR to default OPP */
	//pm_qos_update_request(&ddr_opp_req, PM_QOS_DDR_OPP_DEFAULT_VALUE);
}
