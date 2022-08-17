
#include <linux/backlight.h>
#include <drm/drmP.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>

#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_graph.h>
#include <linux/platform_device.h>

#define CONFIG_MTK_PANEL_EXT
#if defined(CONFIG_MTK_PANEL_EXT)
#include "../mediatek/mtk_panel_ext.h"
#include "../mediatek/mtk_log.h"
#include "../mediatek/mtk_drm_graphics_base.h"
#endif

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
#include "../mediatek/mtk_corner_pattern/mtk_data_hw_roundedpattern.h"
#endif

struct lcm {
	struct device *dev;
	struct drm_panel panel;
	struct backlight_device *backlight;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *bias_pos, *bias_neg;

	bool prepared;
	bool enabled;

	int error;
};

#define lcm_dcs_write_seq(ctx, seq...) \
({\
	const u8 d[] = { seq };\
	BUILD_BUG_ON_MSG(ARRAY_SIZE(d) > 64, "DCS sequence too big for stack");\
	lcm_dcs_write(ctx, d, ARRAY_SIZE(d));\
})

#define lcm_dcs_write_seq_static(ctx, seq...) \
({\
	static const u8 d[] = { seq };\
	lcm_dcs_write(ctx, d, ARRAY_SIZE(d));\
})

static inline struct lcm *panel_to_lcm(struct drm_panel *panel)
{
	return container_of(panel, struct lcm, panel);
}

static void lcm_dcs_write(struct lcm *ctx, const void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	ssize_t ret;
	char *addr;

	if (ctx->error < 0)
		return;

	addr = (char *)data;
	if ((int)*addr < 0xB0)
		ret = mipi_dsi_dcs_write_buffer(dsi, data, len);
	else
		ret = mipi_dsi_generic_write(dsi, data, len);
	if (ret < 0) {
		dev_err(ctx->dev, "error %zd writing seq: %ph\n", ret, data);
		ctx->error = ret;
	}
}

#ifdef PANEL_SUPPORT_READBACK
static int lcm_dcs_read(struct lcm *ctx, u8 cmd, void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	ssize_t ret;

	if (ctx->error < 0)
		return 0;

	ret = mipi_dsi_dcs_read(dsi, cmd, data, len);
	if (ret < 0) {
		dev_err(ctx->dev, "error %d reading dcs seq:(%#x)\n", ret, cmd);
		ctx->error = ret;
	}

	return ret;
}

static void lcm_panel_get_data(struct lcm *ctx)
{
	u8 buffer[3] = {0};
	static int ret;

	if (ret == 0) {
		ret = lcm_dcs_read(ctx,  0x0A, buffer, 1);
		dev_info(ctx->dev, "return %d data(0x%08x) to dsi engine\n",
			 ret, buffer[0] | (buffer[1] << 8));
	}
}
#endif

#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
static struct regulator *disp_bias_pos;
static struct regulator *disp_bias_neg;


static int lcm_panel_bias_regulator_init(void)
{
	static int regulator_inited;
	int ret = 0;

	if (regulator_inited)
		return ret;

	/* please only get regulator once in a driver */
	disp_bias_pos = regulator_get(NULL, "dsv_pos");
	if (IS_ERR(disp_bias_pos)) { /* handle return value */
		ret = PTR_ERR(disp_bias_pos);
		pr_err("get dsv_pos fail, error: %d\n", ret);
		return ret;
	}

	disp_bias_neg = regulator_get(NULL, "dsv_neg");
	if (IS_ERR(disp_bias_neg)) { /* handle return value */
		ret = PTR_ERR(disp_bias_neg);
		pr_err("get dsv_neg fail, error: %d\n", ret);
		return ret;
	}

	regulator_inited = 1;
	return ret; /* must be 0 */

}

static int lcm_panel_bias_enable(void)
{
	int ret = 0;
	int retval = 0;

	lcm_panel_bias_regulator_init();

	/* set voltage with min & max*/
	ret = regulator_set_voltage(disp_bias_pos, 5400000, 5400000);
	if (ret < 0)
		pr_err("set voltage disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_set_voltage(disp_bias_neg, 5400000, 5400000);
	if (ret < 0)
		pr_err("set voltage disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	/* enable regulator */
	ret = regulator_enable(disp_bias_pos);
	if (ret < 0)
		pr_err("enable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_enable(disp_bias_neg);
	if (ret < 0)
		pr_err("enable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	return retval;
}

static int lcm_panel_bias_disable(void)
{
	int ret = 0;
	int retval = 0;

	lcm_panel_bias_regulator_init();

	ret = regulator_disable(disp_bias_neg);
	if (ret < 0)
		pr_err("disable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_disable(disp_bias_pos);
	if (ret < 0)
		pr_err("disable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	return retval;
}
#endif

static void lcm_panel_init(struct lcm *ctx)
{
	ctx->reset_gpio =
		devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
		dev_err(ctx->dev, "%s: cannot get reset_gpio %ld\n",
			__func__, PTR_ERR(ctx->reset_gpio));
		return;
	}
	gpiod_set_value(ctx->reset_gpio, 0);
	udelay(15 * 1000);
	gpiod_set_value(ctx->reset_gpio, 1);
	udelay(1 * 1000);
	gpiod_set_value(ctx->reset_gpio, 0);
	udelay(10 * 1000);
	gpiod_set_value(ctx->reset_gpio, 1);
	udelay(10 * 1000);
	devm_gpiod_put(ctx->dev, ctx->reset_gpio);

	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0x19, 0x11, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0x19, 0x11);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x51, 0x00, 0x08, 0x08, 0x51,
		0x04, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x92);
	lcm_dcs_write_seq_static(ctx, 0xB3, 0x18, 0x06);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x8B);
	//Panel Driving Mode	 //0x88
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x88);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	//1080RGBx2160
	lcm_dcs_write_seq_static(ctx, 0xB3, 0x04, 0x38, 0x08, 0x70);
	//vst
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x84, 0x01, 0x33, 0x34);//vst1
	lcm_dcs_write_seq_static(ctx, 0x00, 0x84);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x00, 0x00);//vst2
	//ckv
	lcm_dcs_write_seq_static(ctx, 0x00, 0xb0);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x85, 0x05, 0x11, 0x09, 0x00,
		0x85, 0x02, 0x22, 0x85, 0x03, 0x33, 0x85, 0x04, 0x00);
	//ckv1 + ckv2 + ckv3 + ckv4
	lcm_dcs_write_seq_static(ctx, 0x00, 0xc0);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	//ckv5 + ckv6 + ckv7 + ckv8
	lcm_dcs_write_seq_static(ctx, 0x00, 0xd0);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x33, 0x33, 0x00, 0x00, 0xf0);
	//vend
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE0);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x02, 0x01, 0x09, 0x07, 0x00, 0x00);
	//rst
	lcm_dcs_write_seq_static(ctx, 0x00, 0xF0);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x80, 0xff, 0x01, 0x08, 0x07);
	//cic
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xc2, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	//pch
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA2);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x00);
	//pwron/pwrof/lvd
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xcb, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xc0);
	lcm_dcs_write_seq_static(ctx, 0xcb, 0x04, 0x04, 0x04, 0xf4, 0x00,
		0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xf0);
	lcm_dcs_write_seq_static(ctx, 0xcb, 0xff, 0x30, 0x33, 0x80);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xcd, 0x01);//map_sel
	//ckh
	lcm_dcs_write_seq_static(ctx, 0x00, 0x94);
	lcm_dcs_write_seq_static(ctx, 0xc0, 0x00, 0x01, 0x06, 0x00, 0x01,
		0x15, 0x05);//170728 best for pixel column inv.
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xD8, 0x2B, 0x2B);//GVDDP/GVDDN
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);//For K16, Vcom by OTP
	lcm_dcs_write_seq_static(ctx, 0xD9, 0xAD, 0x00);//VCOM=-0.27V
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xE0, 0x01);//Gamma Separate Change
	//170929-2 foc to2.2
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xE1, 0x5c, 0x88, 0xdd, 0x13, 0x40,
		0x49, 0x72, 0xb5, 0xe4, 0x55, 0xca, 0xf6, 0x1c, 0x39, 0xa5,
		0x5c, 0x7a, 0xe9, 0x08, 0x9a, 0x24, 0x42, 0x5d, 0x7f, 0xaa,
		0xa7, 0xaa, 0xd9, 0x18, 0xea, 0x3d, 0x60, 0x9b, 0xe3, 0xff,
		0xf4, 0x03);
	//R POS
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xE2, 0x47, 0x88, 0xdd, 0x13, 0x40,
		0x49, 0x72, 0xb5, 0xe4, 0x55, 0xca, 0xf6, 0x1c, 0x39, 0xa5,
		0x5c, 0x7a, 0xe9, 0x08, 0x9a, 0x24, 0x42, 0x5d, 0x7f, 0xaa,
		0xa7, 0xaa, 0xd9, 0x18, 0xea, 0x3d, 0x60, 0x9b, 0xe3, 0xff,
		0xc4, 0x03);
	//R NEG
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xE3, 0x5c, 0x86, 0xdb, 0x2a, 0x40,
		0x4f, 0x7a, 0xb9, 0xea, 0x55, 0xd1, 0xfb, 0x1f, 0x3d, 0xa5,
		0x5e, 0x7e, 0xed, 0x0a, 0x9a, 0x26, 0x45, 0x60, 0x81, 0xaa,
		0xaa, 0xad, 0xda, 0x17, 0xea, 0x3c, 0x5f, 0x9c, 0xe4, 0xff,
		0xf4, 0x03);
	//G POS
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xE4, 0x47, 0x86, 0xdb, 0x2a, 0x40,
		0x4f, 0x7a, 0xb9, 0xea, 0x55, 0xd1, 0xfb, 0x1f, 0x3d, 0xa5,
		0x5e, 0x7e, 0xed, 0x0a, 0x9a, 0x26, 0x45, 0x60, 0x81, 0xaa,
		0xaa, 0xad, 0xda, 0x17, 0xea, 0x3c, 0x5f, 0x9c, 0xe4, 0xff,
		0xc4, 0x03);
	//G NEG
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xE5, 0x5c, 0x9f, 0x01, 0x46, 0x50,
		0x6c, 0x92, 0xce, 0xfb, 0x55, 0xe1, 0x08, 0x2a, 0x47, 0xa9,
		0x67, 0x84, 0xf2, 0x0e, 0x9a, 0x2b, 0x49, 0x64, 0x85, 0xaa,
		0xad, 0xb3, 0xe4, 0x20, 0xea, 0x43, 0x66, 0xa0, 0xe6, 0xff,
		0xf4, 0x03);
	//B POS
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xE6, 0x47, 0x9f, 0x01, 0x46, 0x50,
		0x6c, 0x92, 0xce, 0xfb, 0x55, 0xe1, 0x08, 0x2a, 0x47, 0xa9,
		0x67, 0x84, 0xf2, 0x0e, 0x9a, 0x2b, 0x49, 0x64, 0x85, 0xaa,
		0xad, 0xb3, 0xe4, 0x20, 0xea, 0x43, 0x66, 0xa0, 0xe6, 0xff,
		0xc4, 0x03);
	//B NEG
	//Gamma_End--------------------------
	//--------Down Power  Consumption-----------------

	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	//GAP 8->4 ; AP 8->5, 20160420 For Power Saving Setting Modify
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x45);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x91);
	//SAP 8->A, 20160524 For special pattern horizontal band
	lcm_dcs_write_seq_static(ctx, 0xC5, 0xA0);
	//VGH=8.8V  VGH Clamp off
	lcm_dcs_write_seq_static(ctx, 0x00, 0x83);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x1B);
	//VGL  0xAE=-8.6V,  0xC6=-11V
	lcm_dcs_write_seq_static(ctx, 0x00, 0x84);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0xAE);
	//VGHO  0x98=8.5V,  0xAC=10.5V,   0x84=6.5V
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x98);
	//VGLO  0xA8=-8.0V, 0xBC=-10V,    0x94=-6V
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA1);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0xA8);

	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xC3, 0x00, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x86);
	lcm_dcs_write_seq_static(ctx, 0xC3, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x91);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x0F);//timeout open
	//Source v-blank output min.
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xC4, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x81);
	lcm_dcs_write_seq_static(ctx, 0xC4, 0x02);//Chop 2line/2frame
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB1);
	//Gamma Calibration control disable
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x08);
	//Gamma chop = 2line/2frame
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB2);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x22);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xC3, 0x00, 0x00, 0x00, 0x22,
		0x22, 0x00, 0x22, 0x22);//gnd eq
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	//VSP_VSN EQ
	lcm_dcs_write_seq_static(ctx, 0xC3, 0x20, 0x20, 0x02, 0x02);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x92);
	//vdd lvdsvdd //EMI improving
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x33);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x81);
	//SSC //EMI improving
	lcm_dcs_write_seq_static(ctx, 0xC1, 0xB0, 0xC0, 0xF0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x89);
	lcm_dcs_write_seq_static(ctx, 0xc0, 0x10, 0x14);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xcb, 0x00, 0x00, 0x0C);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0xcb, 0x05, 0x04, 0x04, 0xf4,
		0x00, 0x00, 0x04, 0x00, 0x04, 0xf3, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xF0);
	lcm_dcs_write_seq_static(ctx, 0xcb, 0xff, 0x30, 0x33, 0x80);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x84);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x9A);//Vcom active region
	//TE with mode 1
	lcm_dcs_write_seq_static(ctx, 0x35, 0x00);
	//PWM BL
	lcm_dcs_write_seq_static(ctx, 0x51, 0xFC);
	lcm_dcs_write_seq_static(ctx, 0x53, 0x24);
	lcm_dcs_write_seq_static(ctx, 0x55, 0x10);

	lcm_dcs_write_seq_static(ctx, 0x11);
	msleep(200);
	lcm_dcs_write_seq_static(ctx, 0x29);

	msleep(100);
}

static int lcm_disable(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);

	if (!ctx->enabled)
		return 0;

	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_POWERDOWN;
		backlight_update_status(ctx->backlight);
	}

	ctx->enabled = false;

	return 0;
}

static int lcm_unprepare(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);

	if (!ctx->prepared)
		return 0;

	lcm_dcs_write_seq_static(ctx, 0x28);
	msleep(50);
	lcm_dcs_write_seq_static(ctx, 0x10);
	msleep(150);

	ctx->error = 0;
	ctx->prepared = false;
#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
	lcm_panel_bias_disable();
#else
	ctx->reset_gpio =
		devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
		dev_err(ctx->dev, "%s: cannot get reset_gpio %ld\n",
			__func__, PTR_ERR(ctx->reset_gpio));
		return PTR_ERR(ctx->reset_gpio);
	}
	gpiod_set_value(ctx->reset_gpio, 0);
	devm_gpiod_put(ctx->dev, ctx->reset_gpio);


	ctx->bias_neg = devm_gpiod_get_index(ctx->dev,
		"bias", 1, GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->bias_neg)) {
		dev_err(ctx->dev, "%s: cannot get bias_neg %ld\n",
			__func__, PTR_ERR(ctx->bias_neg));
		return PTR_ERR(ctx->bias_neg);
	}
	gpiod_set_value(ctx->bias_neg, 0);
	devm_gpiod_put(ctx->dev, ctx->bias_neg);

	udelay(1000);

	ctx->bias_pos = devm_gpiod_get_index(ctx->dev,
		"bias", 0, GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->bias_pos)) {
		dev_err(ctx->dev, "%s: cannot get bias_pos %ld\n",
			__func__, PTR_ERR(ctx->bias_pos));
		return PTR_ERR(ctx->bias_pos);
	}
	gpiod_set_value(ctx->bias_pos, 0);
	devm_gpiod_put(ctx->dev, ctx->bias_pos);
#endif

	return 0;
}

static int lcm_prepare(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);
	int ret;

	pr_info("%s\n", __func__);
	if (ctx->prepared)
		return 0;

#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
	lcm_panel_bias_enable();
#else
	ctx->bias_pos = devm_gpiod_get_index(ctx->dev,
		"bias", 0, GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->bias_pos)) {
		dev_err(ctx->dev, "%s: cannot get bias_pos %ld\n",
			__func__, PTR_ERR(ctx->bias_pos));
		return PTR_ERR(ctx->bias_pos);
	}
	gpiod_set_value(ctx->bias_pos, 1);
	devm_gpiod_put(ctx->dev, ctx->bias_pos);

	udelay(2000);

	ctx->bias_neg = devm_gpiod_get_index(ctx->dev,
		"bias", 1, GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->bias_neg)) {
		dev_err(ctx->dev, "%s: cannot get bias_neg %ld\n",
			__func__, PTR_ERR(ctx->bias_neg));
		return PTR_ERR(ctx->bias_neg);
	}
	gpiod_set_value(ctx->bias_neg, 1);
	devm_gpiod_put(ctx->dev, ctx->bias_neg);
#endif

	lcm_panel_init(ctx);

	ret = ctx->error;
	if (ret < 0)
		lcm_unprepare(panel);

	ctx->prepared = true;

#if defined(CONFIG_MTK_PANEL_EXT)
	mtk_panel_tch_rst(panel);
#endif
#ifdef PANEL_SUPPORT_READBACK
	lcm_panel_get_data(ctx);
#endif

	return ret;
}

static int lcm_enable(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);

	if (ctx->enabled)
		return 0;

	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(ctx->backlight);
	}

	ctx->enabled = true;

	return 0;
}

#define HFP (42)
#define HSA (10)
#define HBP (42)
#define VFP (10)
#define VSA (3)
#define VBP (15)
#define VAC (2160)
#define HAC (1080)
static u32 fake_heigh = 2160;
static u32 fake_width = 1080;
static bool need_fake_resolution;

static struct drm_display_mode default_mode = {
	.clock = 154122,
	.hdisplay = HAC,
	.hsync_start = HAC + HFP,
	.hsync_end = HAC + HFP + HSA,
	.htotal = HAC + HFP + HSA + HBP,
	.vdisplay = VAC,
	.vsync_start = VAC + VFP,
	.vsync_end = VAC + VFP + VSA,
	.vtotal = VAC + VFP + VSA + VBP,
	.vrefresh = 60,
};

#if defined(CONFIG_MTK_PANEL_EXT)
static int panel_ext_reset(struct drm_panel *panel, int on)
{
	struct lcm *ctx = panel_to_lcm(panel);

	ctx->reset_gpio =
		devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
		dev_err(ctx->dev, "%s: cannot get reset_gpio %ld\n",
			__func__, PTR_ERR(ctx->reset_gpio));
		return PTR_ERR(ctx->reset_gpio);
	}
	gpiod_set_value(ctx->reset_gpio, on);
	devm_gpiod_put(ctx->dev, ctx->reset_gpio);

	return 0;
}

static int panel_ata_check(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	unsigned char data[3] = {0x00, 0x00, 0x00};
	unsigned char id[3] = {0x00, 0x00, 0x00};
	ssize_t ret;

	ret = mipi_dsi_dcs_read(dsi, 0x4, data, 3);
	if (ret < 0) {
		pr_err("%s error\n", __func__);
		return 0;
	}

	DDPINFO("ATA read data %x %x %x\n", data[0], data[1], data[2]);

	if (data[0] == id[0] &&
			data[1] == id[1] &&
			data[2] == id[2])
		return 1;

	DDPINFO("ATA expect read data is %x %x %x\n",
			id[0], id[1], id[2]);

	return 0;
}

static int lcm_setbacklight_cmdq(void *dsi, dcs_write_gce cb,
	void *handle, unsigned int level)
{
	char bl_tb0[] = {0x51, 0xFF};

	bl_tb0[1] = level;

	if (!cb)
		return -1;

	cb(dsi, handle, bl_tb0, ARRAY_SIZE(bl_tb0));

	return 0;
}

static int lcm_get_virtual_heigh(void)
{
	return VAC;
}

static int lcm_get_virtual_width(void)
{
	return HAC;
}

static struct mtk_panel_params ext_params = {
	.pll_clk = 490,
	.vfp_low_power = 750,
	.cust_esd_check = 0,
	.esd_check_enable = 0,
	.lcm_esd_check_table[0] = {
		.cmd = 0x0A,
		.count = 1,
		.para_list[0] = 0x9D,
	},
	.phy_timcon = {
		.clk_hs_post = 36,
	},
};

static struct mtk_panel_funcs ext_funcs = {
	.reset = panel_ext_reset,
	.set_backlight_cmdq = lcm_setbacklight_cmdq,
	.ata_check = panel_ata_check,
	.get_virtual_heigh = lcm_get_virtual_heigh,
	.get_virtual_width = lcm_get_virtual_width,
};
#endif

struct panel_desc {
	const struct drm_display_mode *modes;
	unsigned int num_modes;

	unsigned int bpc;

	struct {
		unsigned int width;
		unsigned int height;
	} size;

	struct {
		unsigned int prepare;
		unsigned int enable;
		unsigned int disable;
		unsigned int unprepare;
	} delay;
};

static void change_drm_disp_mode_params(struct drm_display_mode *mode)
{
	if (fake_heigh > 0 && fake_heigh < VAC) {
		mode->vdisplay = fake_heigh;
		mode->vsync_start = fake_heigh + VFP;
		mode->vsync_end = fake_heigh + VFP + VSA;
		mode->vtotal = fake_heigh + VFP + VSA + VBP;
	}
	if (fake_width > 0 && fake_width < HAC) {
		mode->hdisplay = fake_width;
		mode->hsync_start = fake_width + HFP;
		mode->hsync_end = fake_width + HFP + HSA;
		mode->htotal = fake_width + HFP + HSA + HBP;
	}
}

static int lcm_get_modes(struct drm_panel *panel)
{
	struct drm_display_mode *mode;

	if (need_fake_resolution)
		change_drm_disp_mode_params(&default_mode);
	mode = drm_mode_duplicate(panel->drm, &default_mode);
	if (!mode) {
		dev_err(panel->drm->dev, "failed to add mode %ux%ux@%u\n",
			default_mode.hdisplay, default_mode.vdisplay,
			default_mode.vrefresh);
		return -ENOMEM;
	}

	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(panel->connector, mode);

	panel->connector->display_info.width_mm = 64;
	panel->connector->display_info.height_mm = 129;

	return 1;
}

static const struct drm_panel_funcs lcm_drm_funcs = {
	.disable = lcm_disable,
	.unprepare = lcm_unprepare,
	.prepare = lcm_prepare,
	.enable = lcm_enable,
	.get_modes = lcm_get_modes,
};

static void check_is_need_fake_resolution(struct device *dev)
{
	unsigned int ret = 0;

	ret = of_property_read_u32(dev->of_node, "fake_heigh", &fake_heigh);
	if (ret)
		need_fake_resolution = false;
	ret = of_property_read_u32(dev->of_node, "fake_width", &fake_width);
	if (ret)
		need_fake_resolution = false;
	if (fake_heigh > 0 && fake_heigh < VAC)
		need_fake_resolution = true;
	if (fake_width > 0 && fake_width < HAC)
		need_fake_resolution = true;
}

static int lcm_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct lcm *ctx;
	struct device_node *backlight;
	int ret;
	struct device_node *dsi_node, *remote_node = NULL, *endpoint = NULL;

	dsi_node = of_get_parent(dev->of_node);
	if (dsi_node) {
		endpoint = of_graph_get_next_endpoint(dsi_node, NULL);
		if (endpoint) {
			remote_node = of_graph_get_remote_port_parent(endpoint);
			if (!remote_node) {
				pr_info("No panel connected,skip probe lcm\n");
				return -ENODEV;
			}
			pr_info("device node name:%s\n", remote_node->name);
		}
	}
	if (remote_node != dev->of_node) {
		pr_info("%s+ skip probe due to not current lcm\n", __func__);
		return -ENODEV;
	}

	ctx = devm_kzalloc(dev, sizeof(struct lcm), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	mipi_dsi_set_drvdata(dsi, ctx);

	ctx->dev = dev;
	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE
			 | MIPI_DSI_MODE_LPM | MIPI_DSI_MODE_EOT_PACKET
			 | MIPI_DSI_CLOCK_NON_CONTINUOUS;

	backlight = of_parse_phandle(dev->of_node, "backlight", 0);
	if (backlight) {
		ctx->backlight = of_find_backlight_by_node(backlight);
		of_node_put(backlight);

		if (!ctx->backlight)
			return -EPROBE_DEFER;
	}

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
		dev_err(dev, "%s: cannot get reset-gpios %ld\n",
			__func__, PTR_ERR(ctx->reset_gpio));
		return PTR_ERR(ctx->reset_gpio);
	}
	devm_gpiod_put(dev, ctx->reset_gpio);

	ctx->bias_pos = devm_gpiod_get_index(dev, "bias", 0, GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->bias_pos)) {
		dev_err(dev, "%s: cannot get bias-pos 0 %ld\n",
			__func__, PTR_ERR(ctx->bias_pos));
		return PTR_ERR(ctx->bias_pos);
	}
	devm_gpiod_put(dev, ctx->bias_pos);

	ctx->bias_neg = devm_gpiod_get_index(dev, "bias", 1, GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->bias_neg)) {
		dev_err(dev, "%s: cannot get bias-neg 1 %ld\n",
			__func__, PTR_ERR(ctx->bias_neg));
		return PTR_ERR(ctx->bias_neg);
	}
	devm_gpiod_put(dev, ctx->bias_neg);

	ctx->prepared = true;
	ctx->enabled = true;

	drm_panel_init(&ctx->panel);
	ctx->panel.dev = dev;
	ctx->panel.funcs = &lcm_drm_funcs;

	ret = drm_panel_add(&ctx->panel);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_attach(dsi);
	if (ret < 0)
		drm_panel_remove(&ctx->panel);

#if defined(CONFIG_MTK_PANEL_EXT)
	mtk_panel_tch_handle_reg(&ctx->panel);
	ret = mtk_panel_ext_create(dev, &ext_params, &ext_funcs, &ctx->panel);
	if (ret < 0)
		return ret;
#endif
	check_is_need_fake_resolution(dev);
	pr_info("%s-\n", __func__);

	return ret;
}

static int lcm_remove(struct mipi_dsi_device *dsi)
{
	struct lcm *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct of_device_id lcm_of_match[] = {
	{ .compatible = "otm1911a,auo,vdo", },
	{ }
};

MODULE_DEVICE_TABLE(of, lcm_of_match);

static struct mipi_dsi_driver lcm_driver = {
	.probe = lcm_probe,
	.remove = lcm_remove,
	.driver = {
		.name = "panel-otm1911a-auo-vdo",
		.owner = THIS_MODULE,
		.of_match_table = lcm_of_match,
	},
};

module_mipi_dsi_driver(lcm_driver);

MODULE_AUTHOR("Yi-Lun Wang <Yi-Lun.Wang@mediatek.com>");
MODULE_DESCRIPTION("otm1911a VDO LCD Panel Driver");
MODULE_LICENSE("GPL v2");

