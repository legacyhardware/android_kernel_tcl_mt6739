
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <mtu3.h>
#include <mtu3_hal.h>
#include <mtk_idle.h>
#include "mtu3_priv.h"

int get_ssusb_ext_rscs(struct ssusb_mtk *ssusb)
{
	struct device *dev = ssusb->dev;
	struct ssusb_priv *priv;

	/* all elements are set to ZERO as default value */
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->vusb10 = devm_regulator_get(dev, "va09");
	if (IS_ERR(priv->vusb10)) {
		dev_info(dev, "failed to get vusb10\n");
		return PTR_ERR(priv->vusb10);
	}

	/* private mode setting */
	ssusb->force_vbus = true;
	ssusb->u1u2_disable = true;
	ssusb->u3_loopb_support = true;

	ssusb->priv_data = priv;
	return 0;
}

static int ssusb_host_clk_on(struct ssusb_mtk *ssusb)
{
	return 0;
}

static int ssusb_host_clk_off(struct ssusb_mtk *ssusb)
{
	return 0;
}

static int ssusb_sysclk_on(struct ssusb_mtk *ssusb)
{
	int ret = 0;

	ret = clk_prepare_enable(ssusb->sys_clk);
	if (ret)
		dev_info(ssusb->dev, "failed to enable sys_clk\n");

	ret = clk_prepare_enable(ssusb->ref_clk);
	if (ret)
		dev_info(ssusb->dev, "failed to enable ref_clk\n");

	return ret;
}

static void ssusb_sysclk_off(struct ssusb_mtk *ssusb)
{
	clk_disable_unprepare(ssusb->sys_clk);
	clk_disable_unprepare(ssusb->ref_clk);
}

int ssusb_clk_on(struct ssusb_mtk *ssusb, int host_mode)
{
	if (host_mode) {
		ssusb_sysclk_on(ssusb);
		ssusb_host_clk_on(ssusb);
	} else {
		ssusb_sysclk_on(ssusb);
	}
	return 0;
}

int ssusb_clk_off(struct ssusb_mtk *ssusb, int host_mode)
{
	if (host_mode) {
		ssusb_host_clk_off(ssusb);
		ssusb_sysclk_off(ssusb);
	} else {
		ssusb_sysclk_off(ssusb);
	}
	return 0;
}

int ssusb_ext_pwr_on(struct ssusb_mtk *ssusb, int mode)
{
	int ret = 0;
	struct ssusb_priv *priv;

	priv = ssusb->priv_data;
	ret = regulator_enable(priv->vusb10);
	if (ret)
		dev_info(ssusb->dev, "failed to enable vusb10\n");
	return ret;
}

int ssusb_ext_pwr_off(struct ssusb_mtk *ssusb, int mode)
{
	int ret = 0;
	struct ssusb_priv *priv;

	priv = ssusb->priv_data;
	ret = regulator_disable(priv->vusb10);
	if (ret)
		dev_info(ssusb->dev, "failed to disable vusb10\n");
	return ret;
}

void ssusb_dpidle_request(int mode)
{
	/* not support */
}
