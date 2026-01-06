#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/io.h>

#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>


struct lcd_regs {
    volatile unsigned int fb_base_phys;
    volatile unsigned int fb_xres;
    volatile unsigned int fb_yres;
    volatile unsigned int fb_bpp;
};

static struct lcd_regs *mylcd_regs;

static struct fb_info *myfb_info;

static unsigned int pseudo_palette[16];

static struct gpio_desc *bl_gpio;
static struct clk *clk_pix;
static struct clk *clk_axi;

/*lcd控制器寄存器操作*/


static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
    chan &= 0xffff;
    chan >>= 16 - bf->length;
    return chan << bf->offset;
}

static int mylcd_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue,
                            unsigned transp, struct fb_info *info)
{
    unsigned int val;

    switch(info->fix.visual)
    {
        case FB_VISUAL_TRUECOLOR:
            if(regno < 16) 
            {
                u32 *pal = info->paseudo_palette;

                val = chan_to_field(red, &info->var.red);
                val |= chan_to_field(green, &info->var.green);
                val |= chan_to_field(blue, &info->var.blue);

                pal[regno] = val;
            }
            break;
        
        default:
            return 1;

    }

    return 0;
}

static struct fb_ops myfb_ops = {
    .owner = THIS_MODULE,
    .fb_setcolreg = mylcd_setcolreg,
    .fb_fillrect = cfb_fillrect,
    .fb_copyarea = cfb_copyarea,
    .fb_imageblit = cfb_imageblit,
};

static int mylcd_probe(struct platform_device *pdev)
{
    dma_addr_t phy_addr;
	int ret;
	int width;
	int bits_per_pixel;
	struct display_timings *timings = NULL;
    struct device_node *display_np;

    display_np = of_parse_phandle(pdev->dev.of_node, "display", 0);

	/* get common info */
	ret = of_property_read_u32(display_np, "bus-width", &width);
	ret = of_property_read_u32(display_np, "bits-per-pixel",
				   &bits_per_pixel);

    /* get timming */
	timings = of_get_display_timings(display_np);


    bl_gpio = gpio_get(&pdev->dev, "backlight", 0);

    gpio_direction_output(bl_gpio, 1);

    clk_pix = devm_clk_get(&pdev->dev, "pix");
    clk_aix = devm_clk_get(&pdev->dev, "aix");

    clk_set_rate(clk_pix, 50000000);

	/* enable clk */
	clk_prepare_enable(clk_pix);
	clk_prepare_enable(clk_axi);

    myfb_info = framebuffer_alloc(0, NULL);

	/* 1.2 设置fb_info */
	/* a. var : LCD分辨率、颜色格式 */
	myfb_info->var.xres_virtual = myfb_info->var.xres = 500;
	myfb_info->var.yres_virtual = myfb_info->var.yres = 300;
    myfb_info->var.bits_per_pixel = 16; //rgb565

	myfb_info->var.red.offset = 11;
	myfb_info->var.red.length = 5;

	myfb_info->var.green.offset = 5;
	myfb_info->var.green.length = 6;

	myfb_info->var.blue.offset = 0;
	myfb_info->var.blue.length = 5;

    /*显存大小（字节）fix   */
	strcpy(myfb_info->fix.id, "100ask_lcd");
    myfb_info->fix.sem_len = myfb_info->var.xres * myfb_info->var.yres * myfb_info->var.bits_per_pixel / 8;
	if (myfb_info->var.bits_per_pixel == 24)
		myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * 4;
    

    /* fb的虚拟地址 */
    myfb_info->screen_base = dma_alloc_wc(NULL, myfb_info->fix.sem_len, &phy_addr, GFP_KERNEL);
    
    myfb_info->fix.sem_start = phy_addr;

    myfb_info->fix.type = FB_TYPE_PACKED_PIXELS;
	myfb_info->fix.visual = FB_VISUAL_TRUECOLOR;

    /* c. fbops */
    myfb_info->fops = &myfb_ops;

    /* 1.3 注册fb_info */
    register_framebuffer(myfb_ops);
    
	/* 1.4 硬件操作 */
    mylcd_regs = ioremap(0x021c8000, sizeof(struct lcd_regs));
    mylcd_regs->fb_base_phys = phy_addr;
    mylcd_regs->fb_xres = 500;
    mylcd_regs->fb_yres = 300;
    mylcd_regs->fb_bpp = 16;

    return 0;
}

static int mylcd_remove(struct platform_device *pdev)
{
	/* 反过来操作 */
	/* 2.1 反注册fb_info */
	unregister_framebuffer(myfb_info);

	/* 2.2 释放fb_info */
	framebuffer_release(myfb_info);

    iounmap(mylcd_regs);

}

static const struct of_device_id mylcd_of_match[] = {
    {.compatible = "100ask, lcd_drv"},
    {},
};
MODULE_DEVICE_TABLE(of, simplefb_of_match);

static struct platform_driver mylcd_driver = {
    .driver = {
        .name = "mylcd",
        .of_match_table = mylcd_of_match,
    },
    .probe = mylcd_probe,
    .remove = mylcd_remove,
};

static int __init lcd_drv_init(void)
{
    int ret;
    struct device_node *np;

    ret = platform_driver_register(&mylcd_driver);
    if(ret)
        return ret;

    return 0;
}

static void __exit lcd_drv_exit(void)
{
    platform_driver_unregister(&mylcd_driver);
}

module_init(lcd_drv_init);
module_exit(lcd_drv_exit);

MODULE_AUTHOR("yuji");
MODULE_DESCRIPTION("Framebuffer driver for the linux");
MODULE_LICENSE("GPL");



/**
	framebuffer-mylcd {
			compatible = "100ask,lcd_drv";
	        pinctrl-names = "default";
			pinctrl-0 = <&mylcd_pinctrl>;
			backlight-gpios = <&gpio1 8 GPIO_ACTIVE_HIGH>;

            clocks = <&clks IMX6UL_CLK_LCDIF_PIX>,
                     <&clks IMX6UL_CLK_LCDIF_APB>;
            clock-names = "pix", "axi";

            display = <&display0>;

			display0: display {
				bits-per-pixel = <24>;
				bus-width = <24>;

				display-timings {
					native-mode = <&timing0>;

					 timing0: timing0_1024x600 {
					 clock-frequency = <50000000>;
					 hactive = <1024>;
					 vactive = <600>;
					 hfront-porch = <160>;
					 hback-porch = <140>;
					 hsync-len = <20>;
					 vback-porch = <20>;
					 vfront-porch = <12>;
					 vsync-len = <3>;

					 hsync-active = <0>;
					 vsync-active = <0>;
					 de-active = <1>;
					 pixelclk-active = <0>;
					 };

				};
			};            
			
	};
 * **/