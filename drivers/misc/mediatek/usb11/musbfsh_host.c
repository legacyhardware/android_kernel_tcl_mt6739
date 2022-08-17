
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>

#include "musbfsh_core.h"
#include "musbfsh_host.h"
#include "musbfsh_dma.h"
#include "usb.h"
#include "musbfsh_qmu.h"
#include "mtk11_qmu.h"



static u8 dynamic_fifo_total_slot = 15;
int musbfsh_host_alloc_ep_fifo(struct musbfsh *musbfsh,
	struct musbfsh_qh *qh, u8 is_in)
{
	void __iomem *mbase = musbfsh->mregs;
	int epnum = qh->hw_ep->epnum;
	u16 maxpacket;
	u16 request_fifo_sz, fifo_unit_nr;
	u16 idx_start = 0;
	u8 index, i;
	u16 c_off = 0;
	u8 c_size = 0;
	u16 free_uint = 0;
	u8 found = 0;

	maxpacket = qh->maxpacket * qh->hb_mult;
	if (maxpacket <= 512) {
		request_fifo_sz = 512;
		fifo_unit_nr = 1;
		c_size = 6;
	} else if (maxpacket <= 1024) {
		request_fifo_sz = 1024;
		fifo_unit_nr = 2;
		c_size = 7;
	} else if (maxpacket <= 2048) {
		request_fifo_sz = 2048;
		fifo_unit_nr = 4;
		c_size = 8;
	} else if (maxpacket <= 4096) {
		request_fifo_sz = 4096;
		fifo_unit_nr = 8;
		c_size = 9;
	} else {
		ERR("should not be here qh maxp:%d maxp:%d\n",
			qh->maxpacket, maxpacket);
		request_fifo_sz = 0;
		fifo_unit_nr = 0;
		musbfsh_bug();
		return -ENOSPC;
	}

	for (i = 0; i < dynamic_fifo_total_slot; i++) {
		if (!(musbfsh_host_dynamic_fifo_usage_msk & (1 << i)))
			free_uint++;
		else
			free_uint = 0;

		if (free_uint == fifo_unit_nr) {
			found = 1;
			break;
		}
	}

	if (found == 0) {
		ERR("!enough,fifo msk:0x%x,maxp:%d,req_len:%d,ep%d-%s\n",
				musbfsh_host_dynamic_fifo_usage_msk, maxpacket,
				request_fifo_sz, epnum, is_in ? "in":"out");
		return -1;
	}

	idx_start = i - (fifo_unit_nr - 1);
	c_off = (64 >> 3) + idx_start * (512 >> 3);

	for (i = 0; i < fifo_unit_nr; i++)
		musbfsh_host_dynamic_fifo_usage_msk |= (1 << (idx_start + i));

	index = musbfsh_readb(mbase, MUSBFSH_INDEX);
	musbfsh_writeb(musbfsh->mregs, MUSBFSH_INDEX, epnum);
	if (is_in) {
		musbfsh_write_rxfifosz(mbase, c_size);
		musbfsh_write_rxfifoadd(mbase, c_off);

		INFO("addr:0x%x, size:0x%x\n",
			musbfsh_read_rxfifoadd(mbase),
			musbfsh_read_rxfifosz(mbase));
	} else {
		musbfsh_write_txfifosz(mbase, c_size);
		musbfsh_write_txfifoadd(mbase, c_off);
		INFO("addr:0x%x, size:0x%x\n",
			musbfsh_read_txfifoadd(mbase),
			musbfsh_read_txfifosz(mbase));
	}
	musbfsh_writeb(mbase, MUSBFSH_INDEX, index);

	INFO("maxp:%d req_len:%d fifo msk:0x%x, ep%d-%s, qh->type:%d\n",
	    maxpacket, request_fifo_sz,
	    musbfsh_host_dynamic_fifo_usage_msk,
	    epnum, is_in ? "in":"out", qh->type);
	return 0;
}

void musbfsh_host_free_ep_fifo(struct musbfsh *musbfsh,
	struct musbfsh_qh *qh, u8 is_in)
{
	void __iomem *mbase = musbfsh->mregs;
	int epnum = qh->hw_ep->epnum;
	u16 maxpacket = qh->maxpacket;
	u16 request_fifo_sz, fifo_unit_nr;
	u16 idx_start = 0;
	u8 index, i;
	u16 c_off = 0;

	maxpacket = qh->maxpacket * qh->hb_mult;
	if (maxpacket <= 512) {
		request_fifo_sz = 512;
		fifo_unit_nr = 1;
	} else if (maxpacket <= 1024) {
		request_fifo_sz = 1024;
		fifo_unit_nr = 2;
	} else if (maxpacket <= 2048) {
		request_fifo_sz = 2048;
		fifo_unit_nr = 4;
	} else if (maxpacket <= 4096) {
		request_fifo_sz = 4096;
		fifo_unit_nr = 8;
	} else {
		ERR("should not be here qh maxp:%d maxp:%d\n",
			qh->maxpacket, maxpacket);
		request_fifo_sz = 0;
		fifo_unit_nr = 0;
		musbfsh_bug();
	}

	index = musbfsh_readb(mbase, MUSBFSH_INDEX);
	musbfsh_writeb(mbase, MUSBFSH_INDEX, epnum);

	if (is_in)
		c_off =  musbfsh_read_rxfifoadd(mbase);
	else
		c_off = musbfsh_read_txfifoadd(mbase);

	idx_start = (c_off - (64 >> 3)) / (512 >> 3);

	for (i = 0; i < fifo_unit_nr; i++)
		musbfsh_host_dynamic_fifo_usage_msk &= ~(1 << (idx_start + i));

	if (is_in) {
		musbfsh_write_rxfifosz(mbase, 0);
		musbfsh_write_rxfifoadd(mbase, 0);
	} else {
		musbfsh_write_txfifosz(mbase, 0);
		musbfsh_write_txfifoadd(mbase, 0);
	}
	musbfsh_writeb(mbase, MUSBFSH_INDEX, index);

	INFO("maxp:%d, req_len:%d, fifomsk:0x%x, ep%d-%s, qh->type:%d\n",
	    maxpacket, request_fifo_sz,
	    musbfsh_host_dynamic_fifo_usage_msk,
	    epnum, is_in ? "in":"out", qh->type);
}

static void musbfsh_ep_program(struct musbfsh *musbfsh, u8 epnum,
			       struct urb *urb, int is_out, u8 *buf,
			       u32 offset, u32 len);

void musbfsh_bug(void)
{
	/* make KE happen */
	char *ptr = NULL;

	*ptr = 10;
}

void musbfsh_h_tx_flush_fifo(struct musbfsh_hw_ep *ep)
{
	void __iomem *epio = ep->regs;
	u16 csr;
	u16 lastcsr = 0;
	int retries = 1000;

	INFO("%s++\r\n", __func__);
	csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
	while (csr & MUSBFSH_TXCSR_FIFONOTEMPTY) {
		if (csr != lastcsr)
			INFO("Host TX FIFONOTEMPTY csr: %02x\n", csr);
		lastcsr = csr;
		csr &= ~MUSBFSH_TXCSR_TXPKTRDY;
		csr |= MUSBFSH_TXCSR_FLUSHFIFO;
		musbfsh_writew(epio, MUSBFSH_TXCSR, csr);
		csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
		if (retries-- < 1) {
			WARNING("Could not flush host TX%d fifo: csr: %04x\n",
				ep->epnum, csr);
			return;
		}
		mdelay(1);
	}
}

static void musbfsh_h_ep0_flush_fifo(struct musbfsh_hw_ep *ep)
{
	void __iomem *epio = ep->regs;
	u16 csr;
	int retries = 5;

	INFO("%s++\r\n", __func__);
	/* scrub any data left in the fifo */
	do {
		csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
		if (!(csr & (MUSBFSH_CSR0_TXPKTRDY | MUSBFSH_CSR0_RXPKTRDY)))
			break;
		musbfsh_writew(epio, MUSBFSH_TXCSR, MUSBFSH_CSR0_FLUSHFIFO);
		csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
		udelay(10);
	} while (--retries);

	if (!retries)
		WARNING("Could not flush host TX%d fifo: csr: %04x\n",
			ep->epnum, csr);

	/* and reset for the next transfer */
	musbfsh_writew(epio, MUSBFSH_TXCSR, 0);
}

static inline void musbfsh_h_tx_start(struct musbfsh_hw_ep *ep)
{
	u16 txcsr;

	INFO("%s++\r\n", __func__);
	/* NOTE: no locks here; caller should lock and select EP */
	if (ep->epnum) {
		txcsr = musbfsh_readw(ep->regs, MUSBFSH_TXCSR);
		INFO("txcsr=0x%x for ep%d\n", txcsr, ep->epnum);
		txcsr |= MUSBFSH_TXCSR_TXPKTRDY | MUSBFSH_TXCSR_H_WZC_BITS;
		musbfsh_writew(ep->regs, MUSBFSH_TXCSR, txcsr);
		txcsr = musbfsh_readw(ep->regs, MUSBFSH_TXCSR);
		INFO("txcsr=0x%x for ep%d\n", txcsr, ep->epnum);
	} else {
		txcsr = musbfsh_readw(ep->regs, MUSBFSH_CSR0);
		INFO("txcsr=0x%x for ep%d\n", txcsr, ep->epnum);
		txcsr = MUSBFSH_CSR0_H_DIS_PING |
			MUSBFSH_CSR0_H_SETUPPKT | MUSBFSH_CSR0_TXPKTRDY;
		musbfsh_writew(ep->regs, MUSBFSH_CSR0, txcsr);
		txcsr = musbfsh_readw(ep->regs, MUSBFSH_TXCSR);
		INFO("txcsr=0x%x for ep%d\n", txcsr, ep->epnum);
	}

}

void musbfsh_ep_set_qh(struct musbfsh_hw_ep *ep, int is_in,
			      struct musbfsh_qh *qh)
{
	if (is_in != 0 || ep->is_shared_fifo)
		ep->in_qh = qh;
	if (is_in == 0 || ep->is_shared_fifo)
		ep->out_qh = qh;
}

struct musbfsh_qh *musbfsh_ep_get_qh(struct musbfsh_hw_ep *ep,
	int is_in)
{
	INFO("%s++, hw_ep%d, is_in=%d\r\n",
	     __func__, ep->epnum, is_in);
	return is_in ? ep->in_qh : ep->out_qh;
}

static void musbfsh_start_urb(struct musbfsh *musbfsh, int is_in,
			      struct musbfsh_qh *qh)
{
	u16 frame;
	u32 len;
	struct urb *urb = next_urb(qh);
	void *buf = urb->transfer_buffer;
	u32 offset = 0;
	struct musbfsh_hw_ep *hw_ep = qh->hw_ep;
	unsigned int pipe = urb->pipe;
	u8 address = usb_pipedevice(pipe);
	int epnum = hw_ep->epnum;
	void __iomem *mbase = musbfsh->mregs;

	INFO("%s++, addr=%d, hw_ep->epnum=%d, urb_ep_addr:0x%x \r\n",
	     __func__, address, epnum, urb->ep->desc.bEndpointAddress);
	/*
	 * MYDBG("urb:%x, blen:%d, alen:%d, hep:%x, ep:%x\n",
	 *	urb, urb->transfer_buffer_length, urb->actual_length,
	 *	epnum, urb->ep->desc.bEndpointAddress);
	 */

	/* initialize software qh state */
	/* indicate the buffer pointer now. */
	qh->offset = 0;
	qh->segsize = 0;

	/* gather right source of data */
	switch (qh->type) {
	case USB_ENDPOINT_XFER_CONTROL:	/* PIO mode only */
		/* control transfers always start with SETUP */
		/* setup packet should be sent out of the controller. */
		is_in = 0;
		musbfsh->ep0_stage = MUSBFSH_EP0_START;
		buf = urb->setup_packet;	/* contain the request. */
		len = 8;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		qh->iso_idx = 0;
		qh->frame = 0;
		offset = urb->iso_frame_desc[0].offset;
		len = urb->iso_frame_desc[0].length;
		break;
	default:
		/* bulk, interrupt */
		/* actual_length may be nonzero on retry paths */
		/* before the urb, actual_length should be 0. */
		buf = urb->transfer_buffer + urb->actual_length;
		len = urb->transfer_buffer_length - urb->actual_length;
	}
	INFO("qh %p urb %p dev%d ep%d %s %s, hw_ep %d, %p/%d\n",
		qh, urb, address, qh->epnum, is_in ? "in" : "out",
		({ char *s;
			switch (qh->type) {
			case USB_ENDPOINT_XFER_CONTROL:
				s = "-ctl";
				break;
			case USB_ENDPOINT_XFER_BULK:
				s = "-bulk";
				break;
			default:
				s = "-intr";
				break;
			};
			s;
		}),
		epnum, buf + offset, len);
	/* Configure endpoint */
	musbfsh_ep_set_qh(hw_ep, is_in, qh);

	/* !is_in, because the fourth parameter of
	 *this func is is_out
	 */
	musbfsh_ep_program(musbfsh, epnum, urb, !is_in, buf, offset, len);

	/* transmit may have more work: start it when it is time */
	/*
	 * Rx,has configure OK in the func: musbfsh_ep_program,
	 * so return directly
	 */
	if (is_in)
		return;

	INFO("Start TX%d %s\n", epnum, hw_ep->tx_channel ? "dma" : "pio");
	switch (qh->type) {
	case USB_ENDPOINT_XFER_ISOC:
	case USB_ENDPOINT_XFER_INT:
		INFO("check whether there's still time for periodic Tx\n");
		frame = musbfsh_readw(mbase, MUSBFSH_FRAME);
		/* FIXME this doesn't implement that scheduling policy ...
		 * or handle framecounter wrapping
		 */
		if ((urb->transfer_flags & URB_ISO_ASAP)
		    || (frame >= urb->start_frame)) {
			/* REVISIT the SOF irq handler shouldn't duplicate
			 * this code; and we don't init urb->start_frame...
			 */
			qh->frame = 0;
			goto start;
		} else {
			qh->frame = urb->start_frame;
			/* enable SOF interrupt so we can count down */
			INFO("SOF for %d\n", epnum);
			musbfsh_writeb(mbase, MUSBFSH_INTRUSBE, 0xff);
		}
		break;
	default:
start:
		INFO("Start TX%d %s\n", epnum,
			hw_ep->tx_channel ? "dma" : "pio");

		if (!hw_ep->tx_channel) {
			/* for pio mode, dma mode will send data after
			 *the configuration of the dma channel
			 */
			musbfsh_h_tx_start(hw_ep);
		}
		/* else if (is_cppi_enabled() || tusb_dma_omap()) */
		/* musb_h_tx_dma_start(hw_ep); */
	}
}

/* Context: caller owns controller lock, IRQs are blocked */
static void musbfsh_giveback(struct musbfsh *musbfsh, struct urb *urb,
			     int status)
__releases(musbfsh->lock) __acquires(musbfsh->lock)
{
	INFO("%s++, complete %p %pF (%d), dev%d ep%d%s, %d/%d\n",
		__func__, urb, urb->complete, status,
		usb_pipedevice(urb->pipe),
		usb_pipeendpoint(urb->pipe),
		usb_pipein(urb->pipe) ? "in" : "out",
		urb->actual_length, urb->transfer_buffer_length);

	/*
	 * MYDBG("urb:%x, blen:%d, alen:%d, ep:%x\n", urb,
	 * urb->transfer_buffer_length, urb->actual_length,
	 * urb->ep->desc.bEndpointAddress);
	 */

	usb_hcd_unlink_urb_from_ep(musbfsh_to_hcd(musbfsh), urb);
	spin_unlock(&musbfsh->lock);
	usb_hcd_giveback_urb(musbfsh_to_hcd(musbfsh), urb, status);
	spin_lock(&musbfsh->lock);
}

/* For bulk/interrupt endpoints only */
static inline void musbfsh_save_toggle(struct musbfsh_qh *qh, int is_in,
				       struct urb *urb)
{
	struct musbfsh *musbfsh = qh->hw_ep->musbfsh;
	u8 epnum = qh->hw_ep->epnum;
	int toggle;

	INFO("%s++\r\n", __func__);
	/*
	 * FIXME: the current Mentor DMA code seems to have
	 * problems getting toggle correct.
	 */
	if (is_in) {
		toggle = musbfsh_readl(musbfsh->mregs, MUSBFSH_RXTOG);
		INFO("toggle_IN=0x%x\n", toggle);
	} else {
		toggle = musbfsh_readl(musbfsh->mregs, MUSBFSH_TXTOG);
		INFO("toggle_OUT=0x%x\n", toggle);
	}

	if (toggle & (1 << epnum))
		usb_settoggle(urb->dev, qh->epnum, !is_in, 1);
	else
		usb_settoggle(urb->dev, qh->epnum, !is_in, 0);
}

static inline void musbfsh_set_toggle(struct musbfsh_qh *qh, int is_in,
				      struct urb *urb)
{
	struct musbfsh *musbfsh = qh->hw_ep->musbfsh;
	u8 epnum = qh->hw_ep->epnum;
	int tog; /* toggle */

	INFO("%s++: qh->hw_ep->epnum %d, qh->epnum %d\n",
	     __func__, qh->hw_ep->epnum,
	     qh->epnum);

	tog = usb_gettoggle(urb->dev, qh->epnum, !is_in);
	if (is_in) {
		INFO("qh->dev->toggle[IN]=0x%x\n", qh->dev->toggle[!is_in]);
		musbfsh_writel(musbfsh->mregs, MUSBFSH_RXTOG,
			       (((1 << epnum) << 16) | (tog << epnum)));
		musbfsh_writel(musbfsh->mregs, MUSBFSH_RXTOG, (tog << epnum));
	} else {
		INFO("qh->dev->toggle[OUT]=0x%x\n", qh->dev->toggle[!is_in]);
		musbfsh_writel(musbfsh->mregs, MUSBFSH_TXTOG,
			       (((1 << epnum) << 16) | (tog << epnum)));
		musbfsh_writel(musbfsh->mregs, MUSBFSH_TXTOG, (tog << epnum));
	}
}

void musbfsh_advance_schedule(struct musbfsh *musbfsh, struct urb *urb,
				     struct musbfsh_hw_ep *hw_ep, int is_in)
{
	struct musbfsh_qh *qh;
	struct musbfsh_hw_ep *ep;
	int ready;
	int status;

	/* the current qh */
	qh = musbfsh_ep_get_qh(hw_ep, is_in);
	ep = qh->hw_ep;
	ready = qh->is_ready;

	INFO("%s++\r\n", __func__);
	status = (urb->status == -EINPROGRESS) ? 0 : urb->status;

	/* save toggle eagerly, for paranoia */
	switch (qh->type) {
	case USB_ENDPOINT_XFER_BULK:
	case USB_ENDPOINT_XFER_INT:
		/* after the urb, should save the toggle for the ep! */
		musbfsh_save_toggle(qh, is_in, urb);
		break;
	case USB_ENDPOINT_XFER_ISOC:
		if (status == 0 && urb->error_count)
			status = -EXDEV;
		break;
	}

	qh->is_ready = 0;
	musbfsh_giveback(musbfsh, urb, status);
	if ((is_in && !hw_ep->in_qh)
			|| (!is_in && !hw_ep->out_qh)) {
		WARNING("QH already freed\n");
		return;
	}
	qh->is_ready = ready;

	/* work around from tablet, avoid KE for qh->hep content 0x6b6b6b6b...
	 * side effect will cause touch memory after free
	 */
	/* if the urb list is empty, the next qh will be excute. */
	if (list_empty(&qh->hep->urb_list)) {
		struct list_head *head;

		if (is_in)
			ep->rx_reinit = 1;
		else
			ep->tx_reinit = 1;

		/* Clobber old pointers to this qh */
#ifdef CONFIG_MTK_DT_USB_SUPPORT
		mark_qh_activity(qh->epnum, ep->epnum, is_in, 1);
#endif
		musbfsh_ep_set_qh(ep, is_in, NULL);
		qh->hep->hcpriv = NULL;

		if (musbfsh_host_dynamic_fifo &&
			qh->type != USB_ENDPOINT_XFER_CONTROL)
			musbfsh_host_free_ep_fifo(musbfsh, qh, is_in);

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
		if (qh->is_use_qmu)
			mtk11_disable_q(musbfsh, hw_ep->epnum, is_in);
#endif

		switch (qh->type) {
		case USB_ENDPOINT_XFER_CONTROL:
		case USB_ENDPOINT_XFER_BULK:
			/*
			 * fifo policy for these lists, except that NAKing
			 * should rotate a qh to the end (for fairness).
			 */
			if (qh->mux == 1) {
				head = qh->ring.prev;
				list_del(&qh->ring);
				kfree(qh);
				qh = first_qh(head);
				break;
			}
		case USB_ENDPOINT_XFER_INT:
		case USB_ENDPOINT_XFER_ISOC:
			/*
			 * this is where periodic bandwidth should be
			 * de-allocated if it's tracked and allocated;
			 * and where we'd update the schedule tree...
			 */
			kfree(qh);
			qh = NULL;
			break;
		}
	}

	if (qh != NULL && qh->is_ready) {
		INFO("... next ep%d %cX urb %p\n",
		     hw_ep->epnum, is_in ? 'R' : 'T', next_urb(qh));
#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
		if (qh->is_use_qmu && !mtk11_host_qmu_concurrent) {
			musbfsh_ep_set_qh(hw_ep, is_in, qh);
			mtk11_kick_CmdQ(musbfsh,
			is_in ? 1:0, qh, next_urb(qh));
		} else if (!qh->is_use_qmu)
			musbfsh_start_urb(musbfsh, is_in, qh);
#else
		musbfsh_start_urb(musbfsh, is_in, qh);
#endif
	}
}

u16 musbfsh_h_flush_rxfifo(struct musbfsh_hw_ep *hw_ep, u16 csr)
{
	/* we don't want fifo to fill itself again;
	 * ignore dma (various models),
	 * leave toggle alone (may not have been saved yet)
	 */
	INFO("%s++\r\n", __func__);
	csr |= MUSBFSH_RXCSR_FLUSHFIFO | MUSBFSH_RXCSR_RXPKTRDY;
	csr &= ~(MUSBFSH_RXCSR_H_REQPKT | MUSBFSH_RXCSR_H_AUTOREQ |
		MUSBFSH_RXCSR_AUTOCLEAR);

	/* write 2x to allow double buffering */
	musbfsh_writew(hw_ep->regs, MUSBFSH_RXCSR, csr);
	musbfsh_writew(hw_ep->regs, MUSBFSH_RXCSR, csr);

	/* flush writebuffer */
	return musbfsh_readw(hw_ep->regs, MUSBFSH_RXCSR);
}

static bool musbfsh_host_packet_rx(struct musbfsh *musbfsh, struct urb *urb,
				   u8 epnum, u8 iso_err)
{
	u16 rx_count;
	u8 *buf;
	u16 csr;
	bool done = false;
	u32 length;
	int do_flush = 0;
	struct musbfsh_hw_ep *hw_ep = musbfsh->endpoints + epnum;
	void __iomem *epio = hw_ep->regs;
	struct musbfsh_qh *qh = hw_ep->in_qh;
	void *buffer = urb->transfer_buffer;

	/* musbfsh_ep_select(mbase, epnum); */
	rx_count = musbfsh_readw(epio, MUSBFSH_RXCOUNT);
	INFO("%s++: real RX%d count %d, buffer %p len %d/%d\n",
	     __func__, epnum, rx_count, urb->transfer_buffer, qh->offset,
	     urb->transfer_buffer_length);
	/* unload FIFO */
	if (usb_pipeisoc(urb->pipe)) {
		int status = 0;
		struct usb_iso_packet_descriptor *d;

		if (iso_err) {
			status = -EILSEQ;
			urb->error_count++;
		}

		d = urb->iso_frame_desc + qh->iso_idx;
		buf = buffer + d->offset;
		length = d->length;
		if (rx_count > length) {
			if (status == 0) {
				status = -EOVERFLOW;
				urb->error_count++;
			}
			WARNING("** OVERFLOW %d into %d\n", rx_count, length);
			do_flush = 1;
		} else
			length = rx_count;
		urb->actual_length += length;
		d->actual_length = length;

		d->status = status;

		/* see if we are done */
		done = (++qh->iso_idx >= urb->number_of_packets);
	} else {
		/* non-isoch */
		buf = buffer + qh->offset;
		length = urb->transfer_buffer_length - qh->offset;
		if (rx_count > length) {
			if (urb->status == -EINPROGRESS)
				urb->status = -EOVERFLOW;
			WARNING("** OVERFLOW %d into %d\n", rx_count, length);
			do_flush = 1;
		} else
			length = rx_count;
		urb->actual_length += length;
		qh->offset += length;

		/* see if we are done */
		done = (urb->actual_length == urb->transfer_buffer_length)
		    || (rx_count < qh->maxpacket)
		    || (urb->status != -EINPROGRESS);
		if (done && (urb->status == -EINPROGRESS)
		    && (urb->transfer_flags & URB_SHORT_NOT_OK)
		    && (urb->actual_length < urb->transfer_buffer_length))
			urb->status = -EREMOTEIO;
	}

	musbfsh_read_fifo(hw_ep, length, buf);

	csr = musbfsh_readw(epio, MUSBFSH_RXCSR);
	csr |= MUSBFSH_RXCSR_H_WZC_BITS;
	if (unlikely(do_flush))
		musbfsh_h_flush_rxfifo(hw_ep, csr);
	else {
		/* REVISIT this assumes AUTOCLEAR is never set */
		csr &= ~(MUSBFSH_RXCSR_RXPKTRDY | MUSBFSH_RXCSR_H_REQPKT);
		if (!done)
			csr |= MUSBFSH_RXCSR_H_REQPKT;
		musbfsh_writew(epio, MUSBFSH_RXCSR, csr);
	}

	return done;
}

static void
musbfsh_rx_reinit(struct musbfsh *musbfsh, struct musbfsh_qh *qh,
		  struct musbfsh_hw_ep *ep)
{
	u16 csr;

	INFO("%s++\r\n", __func__);
	/* NOTE:  we know the "rx" fifo reinit never triggers for ep0.
	 * That always uses tx_reinit since ep0 repurposes TX register
	 * offsets; the initial SETUP packet is also a kind of OUT.
	 */

	/* if programmed for Tx, put it in RX mode */
	if (ep->is_shared_fifo) {
		csr = musbfsh_readw(ep->regs, MUSBFSH_TXCSR);
		if (csr & MUSBFSH_TXCSR_MODE) {
			musbfsh_h_tx_flush_fifo(ep);
			csr = musbfsh_readw(ep->regs, MUSBFSH_TXCSR);
			musbfsh_writew(ep->regs, MUSBFSH_TXCSR,
				       csr | MUSBFSH_TXCSR_FRCDATATOG);
		}

		/*
		 * Clear the MODE bit (and everything else) to enable Rx.
		 * NOTE: we mustn't clear the DMAMODE bit before DMAENAB.
		 */
		if (csr & MUSBFSH_TXCSR_DMAMODE)
			musbfsh_writew(ep->regs, MUSBFSH_TXCSR,
				       MUSBFSH_TXCSR_DMAMODE);
		musbfsh_writew(ep->regs, MUSBFSH_TXCSR, 0);

		/* scrub all previous state, clearing toggle */
	} else {
		csr = musbfsh_readw(ep->regs, MUSBFSH_RXCSR);
		if (csr & MUSBFSH_RXCSR_RXPKTRDY)
			INFO("musbfsh::rx%d, packet/%d ready?\n", ep->epnum,
			     musbfsh_readw(ep->regs, MUSBFSH_RXCOUNT));

		musbfsh_h_flush_rxfifo(ep, 0);
	}

	/* target addr and (for multipoint) hub addr/port */
	if (musbfsh->is_multipoint) {
		musbfsh_write_rxfunaddr(musbfsh->mregs, ep->epnum,
					qh->addr_reg);
		musbfsh_write_rxhubaddr(musbfsh->mregs, ep->epnum,
					qh->h_addr_reg);
		musbfsh_write_rxhubport(musbfsh->mregs, ep->epnum,
					qh->h_port_reg);
	} else {
		musbfsh_writeb(musbfsh->mregs, MUSBFSH_FADDR, qh->addr_reg);
	}

	/* protocol/endpoint, interval/NAKlimit, i/o size */
	musbfsh_writeb(ep->regs, MUSBFSH_RXTYPE, qh->type_reg);
	musbfsh_writeb(ep->regs, MUSBFSH_RXINTERVAL, qh->intv_reg);

	musbfsh_writew(ep->regs, MUSBFSH_RXMAXP, qh->maxpacket);

	ep->rx_reinit = 0;
}

static bool musbfsh_tx_dma_program(struct dma_controller *dma,
				   struct musbfsh_hw_ep *hw_ep,
				   struct musbfsh_qh *qh,
				   struct urb *urb, u32 offset, u32 len)
{
	struct dma_channel *channel = hw_ep->tx_channel;
	void __iomem *epio = hw_ep->regs;
	u16 pkt_size = qh->maxpacket;
	u16 csr;
	u8 mode;

	INFO("%s++\r\n", __func__);
	if (len > channel->max_len)
		len = channel->max_len;

	csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
	if (len > pkt_size) {
		INFO("%s: mode 1\r\n", __func__);
		mode = 1;
		csr |= MUSBFSH_TXCSR_DMAMODE | MUSBFSH_TXCSR_DMAENAB;
		csr |= MUSBFSH_TXCSR_AUTOSET;
	} else {
		INFO("%s: mode 0\r\n", __func__);
		mode = 0;
		csr &= ~(MUSBFSH_TXCSR_AUTOSET | MUSBFSH_TXCSR_DMAMODE);
		csr |= MUSBFSH_TXCSR_DMAENAB; /* against programmer's guide */
	}
	channel->desired_mode = mode;
	INFO("%s: txcsr=0x%x\r\n", __func__, csr);
	/* finish the configration for TXCSR register. */
	musbfsh_writew(epio, MUSBFSH_TXCSR, csr);
	qh->segsize = len;

	/*
	 * Ensure the data reaches to main memory before starting
	 * DMA transfer
	 */
	wmb();

	if (!dma->channel_program(channel, pkt_size, mode,
				  urb->transfer_dma + offset, len)) {
		/* give up the channel, so other ep can use it */
		dma->channel_release(channel);
		hw_ep->tx_channel = NULL;

		csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
		csr &= ~(MUSBFSH_TXCSR_AUTOSET | MUSBFSH_TXCSR_DMAENAB);
		musbfsh_writew(epio, MUSBFSH_TXCSR,
			       csr | MUSBFSH_TXCSR_H_WZC_BITS);
		return false;
	}
	return true;
}

static void musbfsh_ep_program(struct musbfsh *musbfsh, u8 epnum,
			       struct urb *urb, int is_out,
			       u8 *buf, u32 offset, u32 len)
{
	struct dma_controller *dma_controller;
	struct dma_channel *dma_channel;
	void __iomem *mbase = musbfsh->mregs;
	struct musbfsh_hw_ep *hw_ep = musbfsh->endpoints + epnum;
	void __iomem *epio = hw_ep->regs;
	/* the parameter sent to musbfsh_ep_get_qh is is_in */
	struct musbfsh_qh *qh = musbfsh_ep_get_qh(hw_ep, !is_out);
	u16 packet_sz = qh->maxpacket;

	INFO("%s++: %s hw%d urb %p spd%d",
	     __func__, is_out ? "-->" : "<--",
	     epnum, urb, urb->dev->speed);
	INFO("%s  : dev%d ep%d%s h_addr%02x h_port%02x bytes %d\n",
	     __func__,
	     qh->addr_reg, qh->epnum, is_out ? "out" : "in",
	     qh->h_addr_reg, qh->h_port_reg, len);

	/* very important, then we can use the register via epio */
	musbfsh_ep_select(mbase, epnum);

	/* candidate for DMA? */
	/*
	 * wz:for MT65xx, there are not enough dma
	 * channels for all of the eps,
	 * so I think we should add a flag in the hw_ep struct to indicate
	 * whether it has a dma channel.
	 * And check it here to set the dma_channel
	 */
	dma_controller = musbfsh->dma_controller;

	/* will check epnum, indicate dma is not used for ep0! */
	if (is_dma_capable() && epnum && dma_controller) {
		INFO("Using DMA epnum%d\n", epnum);

		/* not all eps have dma channel */
		dma_channel = is_out ? hw_ep->tx_channel : hw_ep->rx_channel;

		/*
		 * if no dma channel yet,
		 * will allocate a channel for this ep!
		 */
		if (!dma_channel) {
			/*
			 * maybe return NULL, if all of the dma channels
			 * have been used.
			 */
			dma_channel =
				dma_controller->channel_alloc(dma_controller,
							      hw_ep, is_out);
			if (dma_channel) {
				INFO("Got a DMA channel for ep%d\n", epnum);
				if (is_out)
					hw_ep->tx_channel = dma_channel;
				else
					hw_ep->rx_channel = dma_channel;
			} else {
				WARNING("DMA channel alloc fail for ep%d\n",
					epnum);
			}
		}
	} else {
		INFO("Using PIO for ep%d\n", epnum);
		dma_channel = NULL;
	}

	/* make sure we clear DMAEnab, autoSet bits from previous run */

	/* OUT/transmit/EP0 or IN/receive? */
	if (is_out) {
		u16 csr;
		u16 int_txe;
		u16 load_count;

		csr = musbfsh_readw(epio, MUSBFSH_TXCSR);

		/* disable interrupt in case we flush */
		int_txe = musbfsh_readw(mbase, MUSBFSH_INTRTXE);
		musbfsh_writew(mbase, MUSBFSH_INTRTXE, int_txe & ~(1 << epnum));

		/* general endpoint setup, not ep0 */
		if (epnum) {	/* Tx endpoint */
			/* flush all old state, set default */
			musbfsh_h_tx_flush_fifo(hw_ep);

			/*
			 * We must not clear the DMAMODE bit before or in
			 * the same cycle with the DMAENAB bit, so we clear
			 * the latter first...
			 */
			csr &= ~(MUSBFSH_TXCSR_H_NAKTIMEOUT	|
				MUSBFSH_TXCSR_AUTOSET		|
				MUSBFSH_TXCSR_DMAENAB		|
				MUSBFSH_TXCSR_FRCDATATOG	|
				MUSBFSH_TXCSR_H_RXSTALL		|
				MUSBFSH_TXCSR_H_ERROR		|
				MUSBFSH_TXCSR_TXPKTRDY);

			/* wz add to init the toggle */
			musbfsh_set_toggle(qh, !is_out, urb);

			musbfsh_writew(epio, MUSBFSH_TXCSR, csr);
			/* REVISIT may need to clear FLUSHFIFO ... */
			csr &= ~MUSBFSH_TXCSR_DMAMODE;
			musbfsh_writew(epio, MUSBFSH_TXCSR, csr);
			csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
		} else {
			/* endpoint 0: just flush */
			musbfsh_h_ep0_flush_fifo(hw_ep);
		}

		/* target addr and (for multipoint) hub addr/port */
		if (musbfsh->is_multipoint) {
			musbfsh_write_txfunaddr(mbase, epnum, qh->addr_reg);
			musbfsh_write_txhubaddr(mbase, epnum, qh->h_addr_reg);
			musbfsh_write_txhubport(mbase, epnum, qh->h_port_reg);
			INFO("set address! h_port_reg 0x%x h_addr_reg 0x%x\n",
			     qh->h_port_reg, qh->h_addr_reg);
		} else {
			/* set the address of the device,very important!! */
			musbfsh_writeb(mbase, MUSBFSH_FADDR, qh->addr_reg);
			INFO("set address! 0x%x\n", qh->addr_reg);
		}

		/* protocol/endpoint/interval/NAKlimit */
		if (epnum) {
			/* set the transfer type and endpoint number */
			musbfsh_writeb(epio, MUSBFSH_TXTYPE, qh->type_reg);
			musbfsh_writew(epio, MUSBFSH_TXMAXP, qh->maxpacket);
			musbfsh_writeb(epio, MUSBFSH_TXINTERVAL, qh->intv_reg);
		} else {	/* ep0 */
			musbfsh_writeb(epio, MUSBFSH_NAKLIMIT0, qh->intv_reg);
			if (musbfsh->is_multipoint)
				musbfsh_writeb(epio, MUSBFSH_TYPE0,
					       qh->type_reg);
		}
		load_count = min_t(u32, packet_sz, len);

		/* write data to the fifo */
		if (dma_channel && musbfsh_tx_dma_program(dma_controller,
							  hw_ep, qh, urb,
							  offset, len))
			load_count = 0;

		if (load_count) {	/* dma is not available */
			/* PIO to load FIFO */
			qh->segsize = load_count;
			musbfsh_write_fifo(hw_ep, load_count, buf);
		}

		/* re-enable interrupt */
		/* after load the data to fifo, but not set txpakready */
		musbfsh_writew(mbase, MUSBFSH_INTRTXE, int_txe);

		/* IN/receive */
	} else {
		u16 csr;

		if (hw_ep->rx_reinit) {
			musbfsh_rx_reinit(musbfsh, qh, hw_ep);
			/* wz add to init the toggle */
			musbfsh_set_toggle(qh, !is_out, urb);
			csr = 0;

			/* disable NYET for interrupt transfer */
			if (qh->type == USB_ENDPOINT_XFER_INT)
				csr |= MUSBFSH_RXCSR_DISNYET;

		} else {	/* bulk IN */
			csr = musbfsh_readw(hw_ep->regs, MUSBFSH_RXCSR);

			if (csr & (MUSBFSH_RXCSR_RXPKTRDY |
				   MUSBFSH_RXCSR_DMAENAB  |
				   MUSBFSH_RXCSR_H_REQPKT))
				ERR("broken !rx_reinit, ep%d csr %04x\n",
				    hw_ep->epnum, csr);

			/* scrub any stale state, leaving toggle alone */
			csr &= MUSBFSH_RXCSR_DISNYET;
		}

		/* kick things off */

		csr |= MUSBFSH_RXCSR_H_REQPKT; /* ask packet from the device */
		INFO("RXCSR%d := %04x\n", epnum, csr);
		musbfsh_writew(hw_ep->regs, MUSBFSH_RXCSR, csr);
		csr = musbfsh_readw(hw_ep->regs, MUSBFSH_RXCSR);
	}
}


static bool musbfsh_h_ep0_continue(struct musbfsh *musbfsh, u16 len,
				   struct urb *urb)
{
	bool more = false;
	u8 *fifo_dest = NULL;
	u16 fifo_count = 0;
	struct musbfsh_hw_ep *hw_ep = musbfsh->control_ep;
	struct musbfsh_qh *qh = hw_ep->in_qh;
	struct usb_ctrlrequest *request;

	INFO("%s++\r\n", __func__);
	switch (musbfsh->ep0_stage) {
	case MUSBFSH_EP0_IN:
		/* actual_length: the data number transferred */
		fifo_dest = urb->transfer_buffer + urb->actual_length;
		fifo_count = min_t(size_t, len,
				   urb->transfer_buffer_length -
					urb->actual_length);

		/* len: the data number in the EP0 fifo */
		if (fifo_count < len)
			urb->status = -EOVERFLOW;

		/* not use dma for ep0 */
		musbfsh_read_fifo(hw_ep, fifo_count, fifo_dest);

		/* update the actual_length! */
		urb->actual_length += fifo_count;
		/*
		 * the in transaction is complete,
		 * should run to status stage.
		 */
		if (len < qh->maxpacket) {
			/* always terminate on short read; it's
			 * rarely reported as an error.
			 more = false;//add by zheng wang
			 */
		} else if (urb->actual_length < urb->transfer_buffer_length)
			more = true;
		break;
	case MUSBFSH_EP0_START:
		request = (struct usb_ctrlrequest *)urb->setup_packet;

		if (!request->wLength) {
			INFO("start no-DATA\n");
			break;
		} else if (request->bRequestType & USB_DIR_IN) {
			INFO("start IN-DATA\n");
			musbfsh->ep0_stage = MUSBFSH_EP0_IN;
			more = true;
			break;	/*wait for next interrupt! */
		}

		INFO("start OUT-DATA\n");
		musbfsh->ep0_stage = MUSBFSH_EP0_OUT;
		more = true;
		/* no break here, send data right now! */
		/* FALLTHROUGH */
	case MUSBFSH_EP0_OUT:
		fifo_count = min_t(size_t, qh->maxpacket,
				   urb->transfer_buffer_length -
					urb->actual_length);
		if (fifo_count) {
			fifo_dest =
				(u8 *)(urb->transfer_buffer +
					urb->actual_length);
			INFO("Sending %d byte%s to ep0 fifo %p\n",
			     fifo_count,
			     (fifo_count == 1) ? "" : "s",
			     fifo_dest);
			musbfsh_write_fifo(hw_ep, fifo_count, fifo_dest);

			urb->actual_length += fifo_count;
			more = true;
		}
		break;
	default:
		ERR("bogus ep0 stage %d\n", musbfsh->ep0_stage);
		break;
	}

	return more;
}

irqreturn_t musbfsh_h_ep0_irq(struct musbfsh *musbfsh)
{
	struct urb *urb;
	u16 csr, len;
	int status = 0;
	void __iomem *mbase = musbfsh->mregs;
	struct musbfsh_hw_ep *hw_ep = musbfsh->control_ep;
	void __iomem *epio = hw_ep->regs;
	struct musbfsh_qh *qh = hw_ep->in_qh;
	bool complete = false;
	irqreturn_t retval = IRQ_NONE;

	INFO("%s++\r\n", __func__);
	/* ep0 only has one queue, "in" */
	urb = next_urb(qh);

	musbfsh_ep_select(mbase, 0);
	csr = musbfsh_readw(epio, MUSBFSH_CSR0);
	len = (csr & MUSBFSH_CSR0_RXPKTRDY)
	    ? musbfsh_readb(epio, MUSBFSH_COUNT0)
	    : 0;

	INFO("<== csr0 %04x, qh %p, count %d, urb %p, stage %d\n",
		csr, qh, len, urb, musbfsh->ep0_stage);

	/* if we just did status stage, we are done */
	if (musbfsh->ep0_stage == MUSBFSH_EP0_STATUS) {
		retval = IRQ_HANDLED;
		complete = true;
	}

	/* prepare status */
	if (csr & MUSBFSH_CSR0_H_RXSTALL) {
		WARNING("STALLING ENDPOINT\n");
		status = -EPIPE;

	} else if (csr & MUSBFSH_CSR0_H_ERROR) {
		WARNING("no response, csr0 %04x\n", csr);
		status = -EPROTO;

	} else if (csr & MUSBFSH_CSR0_H_NAKTIMEOUT) {
		WARNING("control NAK timeout\n");

		/* NOTE:  this code path would be a good place to PAUSE a
		 * control transfer, if another one is queued, so that
		 * ep0 is more likely to stay busy.  That's already done
		 * for bulk RX transfers.
		 *
		 * if (qh->ring.next != &musbfsh->control), then
		 * we have a candidate... NAKing is *NOT* an error
		 */
		musbfsh_writew(epio, MUSBFSH_CSR0, 0);
		retval = IRQ_HANDLED;
	}

	/* if there is an error in the control transfer, and abort it! */
	if (status) {
		INFO("aborting\n");
		retval = IRQ_HANDLED;
		if (urb)
			urb->status = status;
		complete = true;

		/* use the proper sequence to abort the transfer */
		if (csr & MUSBFSH_CSR0_H_REQPKT) {
			csr &= ~MUSBFSH_CSR0_H_REQPKT;
			musbfsh_writew(epio, MUSBFSH_CSR0, csr);
			csr &= ~MUSBFSH_CSR0_H_NAKTIMEOUT;
			musbfsh_writew(epio, MUSBFSH_CSR0, csr);
		} else {
			musbfsh_h_ep0_flush_fifo(hw_ep);
		}

		musbfsh_writeb(epio, MUSBFSH_NAKLIMIT0, 0);

		/* clear it */
		musbfsh_writew(epio, MUSBFSH_CSR0, 0);
	}

	if (unlikely(!urb)) {
		/* stop endpoint since we have no place for its data, this
		 * SHOULD NEVER HAPPEN!
		 */
		ERR("no URB for end 0\n");

		musbfsh_h_ep0_flush_fifo(hw_ep);
		goto done;
	}

	if (!complete) {	/* not the status stage */
		/* call common logic and prepare response */
		if (musbfsh_h_ep0_continue(musbfsh, len, urb)) {
			/* more packets required */
			/*
			 * wz, I think the following code can be
			 * run in musbfsh_h_ep0_continue
			 */
			csr = (musbfsh->ep0_stage == MUSBFSH_EP0_IN)
			    ? MUSBFSH_CSR0_H_REQPKT : MUSBFSH_CSR0_TXPKTRDY;
		} else {
			/* data transfer complete; perform status phase */
			/*
			 * indicate no data stage,
			 * so there is no need to set the stage in
			 * musbfsh_h_ep0_continue
			 */
			if (usb_pipeout(urb->pipe)
			    || !urb->transfer_buffer_length)
				csr = MUSBFSH_CSR0_H_STATUSPKT |
				      MUSBFSH_CSR0_H_REQPKT;
			else
				csr = MUSBFSH_CSR0_H_STATUSPKT |
				      MUSBFSH_CSR0_TXPKTRDY;

			/* flag status stage */
			musbfsh->ep0_stage = MUSBFSH_EP0_STATUS;

			INFO("ep0 STATUS, csr %04x\n", csr);

		}
		musbfsh_writew(epio, MUSBFSH_CSR0, csr);
		retval = IRQ_HANDLED;
	} else
		musbfsh->ep0_stage = MUSBFSH_EP0_IDLE;

	/* call completion handler if done */
	if (complete) {
		/* MYDBG(""); */
		musbfsh_advance_schedule(musbfsh, urb, hw_ep, 1);
	}
done:
	return retval;
}


/* Service a Tx-Available or dma completion irq for the endpoint */
void musbfsh_host_tx(struct musbfsh *musbfsh, u8 epnum)	/* real ep num */
{
	int pipe;
	bool done = false;
	u16 tx_csr;
	size_t length = 0;
	size_t offset = 0;
	struct musbfsh_hw_ep *hw_ep = musbfsh->endpoints + epnum;
	void __iomem *epio = hw_ep->regs;
	struct musbfsh_qh *qh = hw_ep->out_qh;

	struct urb *urb = next_urb(qh);	/* the current urb been processing */

	/* indicate the transfer error, if status=0, there is no error! */
	u32 status = 0;
	void __iomem *mbase = musbfsh->mregs;
	struct dma_channel *dma;
	bool transfer_pending = false;

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
		if (qh && qh->is_use_qmu)
			return;
#endif

	INFO("%s++, real ep=%d\r\n", __func__, epnum);
	if (epio == NULL) {
		/* WARNING("epio: NULL!ep:%d\n", epnum); */
		return;
	}

	musbfsh_ep_select(mbase, epnum);
	tx_csr = musbfsh_readw(epio, MUSBFSH_TXCSR);

	/* with CPPI, DMA sometimes triggers "extra" irqs */
	if (!urb) {
		WARNING("extra TX%d ready, csr %04x\n", epnum, tx_csr);
		return;
	}

	pipe = urb->pipe;
	dma = is_dma_capable() ? hw_ep->tx_channel : NULL;
	INFO("OUT/TX%d end, csr %04x%s\n",
	     epnum, tx_csr, dma ? ", dma" : "pio");

	/* check for errors */
	if (tx_csr & MUSBFSH_TXCSR_H_RXSTALL) {
		/* dma was disabled, fifo flushed */
		WARNING("TX end %d stall\n", epnum);

		/* stall; record URB status */
		status = -EPIPE;

	} else if (tx_csr & MUSBFSH_TXCSR_H_ERROR) {
		/* (NON-ISO) dma was disabled, fifo flushed */
		WARNING("TX 3strikes on ep=%d\n", epnum);

		status = -ETIMEDOUT;

	} else if (tx_csr & MUSBFSH_TXCSR_H_NAKTIMEOUT) {
		WARNING("TX end=%d device not responding\n", epnum);

		/* NOTE:  this code path would be a good place to PAUSE a
		 * transfer, if there's some other (nonperiodic) tx urb
		 * that could use this fifo.  (dma complicates it...)
		 * That's already done for bulk RX transfers.
		 *
		 * if (bulk && qh->ring.next != &musbfsh->out_bulk), then
		 * we have a candidate... NAKing is *NOT* an error
		 */
		musbfsh_ep_select(mbase, epnum);
		musbfsh_writew(epio, MUSBFSH_TXCSR,
			       MUSBFSH_TXCSR_H_WZC_BITS |
			       MUSBFSH_TXCSR_TXPKTRDY);
		return;
	}

	/* if status is not 0, have error, will stop to send data. */
	if (status) {
		if (dma_channel_status(dma) == MUSBFSH_DMA_STATUS_BUSY) {
			dma->status = MUSBFSH_DMA_STATUS_CORE_ABORT;
			(void)musbfsh->dma_controller->channel_abort(dma);
		}

		/* do the proper sequence to abort the transfer in the
		 * usb core; the dma engine should already be stopped.
		 */
		musbfsh_h_tx_flush_fifo(hw_ep);
		tx_csr &= ~(MUSBFSH_TXCSR_AUTOSET |
			    MUSBFSH_TXCSR_DMAENAB |
			    MUSBFSH_TXCSR_H_ERROR |
			    MUSBFSH_TXCSR_H_RXSTALL |
			    MUSBFSH_TXCSR_H_NAKTIMEOUT);

		musbfsh_ep_select(mbase, epnum);
		musbfsh_writew(epio, MUSBFSH_TXCSR, tx_csr);
		/* REVISIT may need to clear FLUSHFIFO ... */
		musbfsh_writew(epio, MUSBFSH_TXCSR, tx_csr);
		musbfsh_writeb(epio, MUSBFSH_TXINTERVAL, 0);

		done = true;
	}

	/* second cppi case */
	if (dma_channel_status(dma) == MUSBFSH_DMA_STATUS_BUSY) {
		WARNING("extra TX%d ready, csr %04x\n", epnum, tx_csr);
		return;
	}

	if (is_dma_capable() && dma && !status) {
		/*
		 * DMA has completed.  But if we're using DMA mode 1 (multi
		 * packet DMA), we need a terminal TXPKTRDY interrupt before
		 * we can consider this transfer completed, lest we trash
		 * its last packet when writing the next URB's data.  So we
		 * switch back to mode 0 to get that interrupt; we'll come
		 * back here once it happens.
		 */
		if (tx_csr & MUSBFSH_TXCSR_DMAMODE) {
			/*
			 * We shouldn't clear DMAMODE with DMAENAB set; so
			 * clear them in a safe order.  That should be OK
			 * once TXPKTRDY has been set (and I've never seen
			 * it being 0 at this moment -- DMA interrupt latency
			 * is significant) but if it hasn't been then we have
			 * no choice but to stop being polite and ignore the
			 * programmer's guide... :-)
			 *
			 * Note that we must write TXCSR with TXPKTRDY cleared
			 * in order not to re-trigger the packet send (this bit
			 * can't be cleared by CPU), and there's another caveat:
			 * TXPKTRDY may be set shortly and then cleared in the
			 * double-buffered FIFO mode, so we do an extra TXCSR
			 * read for debouncing...
			 */
			tx_csr &= musbfsh_readw(epio, MUSBFSH_TXCSR);
			if (tx_csr & MUSBFSH_TXCSR_TXPKTRDY) {
				tx_csr &= ~(MUSBFSH_TXCSR_DMAENAB |
					    MUSBFSH_TXCSR_TXPKTRDY);
				musbfsh_writew(epio, MUSBFSH_TXCSR,
					       tx_csr |
					       MUSBFSH_TXCSR_H_WZC_BITS);
			}
			tx_csr &= ~(MUSBFSH_TXCSR_DMAMODE |
				    MUSBFSH_TXCSR_TXPKTRDY);
			musbfsh_writew(epio, MUSBFSH_TXCSR,
				       tx_csr | MUSBFSH_TXCSR_H_WZC_BITS);

			/*
			 * There is no guarantee that we'll get an interrupt
			 * after clearing DMAMODE as we might have done this
			 * too late (after TXPKTRDY was cleared by controller).
			 * Re-read TXCSR as we have spoiled its previous value.
			 */
			tx_csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
		}

		/*
		 * We may get here from a DMA completion or TXPKTRDY interrupt.
		 * In any case, we must check the FIFO status here and bail out
		 * only if the FIFO still has data -- that should prevent the
		 * "missed" TXPKTRDY interrupts and deal with double-buffered
		 * FIFO mode too...
		 */
		if (tx_csr &
			(MUSBFSH_TXCSR_FIFONOTEMPTY | MUSBFSH_TXCSR_TXPKTRDY)) {
			INFO("DMA complete but Data still in FIFO, CSR %04x\n",
			     tx_csr);
			return;
		}
	}

	if (!status || dma || usb_pipeisoc(pipe)) {
		if (dma)
			length = dma->actual_len;
		else
			length = qh->segsize;
		qh->offset += length;

		if (usb_pipeisoc(pipe)) {
			struct usb_iso_packet_descriptor *d;

			d = urb->iso_frame_desc + qh->iso_idx;
			d->actual_length = length;
			d->status = status;
			if (++qh->iso_idx >= urb->number_of_packets) {
				done = true;
			} else {
				d++;
				offset = d->offset;
				length = d->length;
			}
/* } else if (dma) { */
		} else if (dma && urb->transfer_buffer_length == qh->offset) {
			done = true;
		} else {
			/* see if we need to send more data, or ZLP */
			/* sent a short packet */
			if (qh->segsize < qh->maxpacket)
				done = true;
			else if (qh->offset == urb->transfer_buffer_length
				 && !(urb->transfer_flags & URB_ZERO_PACKET))
				done = true;
			if (!done) {
				offset = qh->offset;
				length = urb->transfer_buffer_length - offset;
				transfer_pending = true;
			}
		}
	}

	/* urb->status != -EINPROGRESS means request has been faulted,
	 * so we must abort this transfer after cleanup
	 */
	if (urb->status != -EINPROGRESS) {
		done = true;
		if (status == 0)
			status = urb->status;
	}

	if (done) {
		/* set status */
		urb->status = status;
		urb->actual_length = qh->offset;
		musbfsh_advance_schedule(musbfsh, urb, hw_ep, USB_DIR_OUT);
		return;
	} else if (transfer_pending && dma) {
		if (musbfsh_tx_dma_program(musbfsh->dma_controller,
					   hw_ep, qh, urb, offset, length))
			return;
	} else if (tx_csr & MUSBFSH_TXCSR_DMAENAB) {
		WARNING("not complete, but DMA enabled?\n");
		return;
	}

	/*
	 * PIO: start next packet in this URB.
	 *
	 * REVISIT: some docs say that when hw_ep->tx_double_buffered,
	 * (and presumably, FIFO is not half-full) we should write *two*
	 * packets before updating TXCSR; other docs disagree...
	 */
	if (length > qh->maxpacket)
		length = qh->maxpacket;
	/* Unmap the buffer so that CPU can use it */
	usb_hcd_unmap_urb_for_dma(musbfsh_to_hcd(musbfsh), urb);
	musbfsh_write_fifo(hw_ep, length, urb->transfer_buffer + offset);
	qh->segsize = length;

	musbfsh_ep_select(mbase, epnum);
	musbfsh_writew(epio, MUSBFSH_TXCSR, MUSBFSH_TXCSR_H_WZC_BITS |
		       MUSBFSH_TXCSR_TXPKTRDY);
}



static void musbfsh_bulk_rx_nak_timeout(struct musbfsh *musbfsh,
					struct musbfsh_hw_ep *ep)
{
	struct dma_channel *dma;
	struct urb *urb;
	void __iomem *mbase = musbfsh->mregs;
	void __iomem *epio = ep->regs;
	struct musbfsh_qh *cur_qh, *next_qh;
	u16 rx_csr;

	INFO("called++\r\n");
	if (epio == NULL) {
		/* WARNING("epio: NULL!\n"); */
		return;
	}

	musbfsh_ep_select(mbase, ep->epnum);
	dma = is_dma_capable() ? ep->rx_channel : NULL;

	/* clear nak timeout bit */
	rx_csr = musbfsh_readw(epio, MUSBFSH_RXCSR);
	rx_csr |= MUSBFSH_RXCSR_H_WZC_BITS;
	rx_csr &= ~MUSBFSH_RXCSR_DATAERROR;
	musbfsh_writew(epio, MUSBFSH_RXCSR, rx_csr);

	cur_qh = first_qh(&musbfsh->in_bulk);
	if (cur_qh) {
		urb = next_urb(cur_qh);
		if (dma_channel_status(dma) == MUSBFSH_DMA_STATUS_BUSY) {
			dma->status = MUSBFSH_DMA_STATUS_CORE_ABORT;
			musbfsh->dma_controller->channel_abort(dma);
			urb->actual_length += dma->actual_len;
			dma->actual_len = 0L;
		}
		musbfsh_save_toggle(cur_qh, 1, urb);

		/* move cur_qh to end of queue */
		list_move_tail(&cur_qh->ring, &musbfsh->in_bulk);

		/* get the next qh from musbfsh->in_bulk */
		next_qh = first_qh(&musbfsh->in_bulk);

		/* set rx_reinit and schedule the next qh */
		ep->rx_reinit = 1;
		/* MYDBG("musbfsh_start_urb go\n"); */
		musbfsh_start_urb(musbfsh, 1, next_qh);
	}
}

void musbfsh_host_rx(struct musbfsh *musbfsh, u8 epnum)
{
	struct urb *urb;
	struct musbfsh_hw_ep *hw_ep = musbfsh->endpoints + epnum;
	void __iomem *epio = hw_ep->regs;
	struct musbfsh_qh *qh = hw_ep->in_qh;
	size_t xfer_len;
	void __iomem *mbase = musbfsh->mregs;
	int pipe;
	u16 rx_csr, val;
	bool done = false;
	u32 status;
	struct dma_channel *dma;
	bool iso_err = false;

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
	if (qh && qh->is_use_qmu)
		return;
#endif

	INFO("real ep=%d\r\n", epnum);

	if (epio == NULL) {
		/* WARNING("epio: NULL!ep:%d\n", epnum); */
		return;
	}

	musbfsh_ep_select(mbase, epnum);

	urb = next_urb(qh);	/* current urb */
	dma = is_dma_capable() ? hw_ep->rx_channel : NULL;
	status = 0;
	xfer_len = 0;

	rx_csr = musbfsh_readw(epio, MUSBFSH_RXCSR);
	val = rx_csr;

	if (unlikely(!urb)) {
		/* REVISIT -- THIS SHOULD NEVER HAPPEN ... but, at least
		 * usbtest #11 (unlinks) triggers it regularly, sometimes
		 * with fifo full.  (Only with DMA??)
		 */
		WARNING("BOGUS RX%d ready, csr %04x, count %d\n", epnum, val,
			musbfsh_readw(epio, MUSBFSH_RXCOUNT));
		musbfsh_h_flush_rxfifo(hw_ep, 0);
		return;
	}

	pipe = urb->pipe;

	INFO("<==real hw %d rxcsr %04x, urb actual %d (+dma %zu)\n",
	     epnum, rx_csr, urb->actual_length, dma ? dma->actual_len : 0);

	/* check for errors, concurrent stall & unlink is not really
	 * handled yet!
	 */
	if (rx_csr & MUSBFSH_RXCSR_H_RXSTALL) {
		WARNING("RX end %d STALL\n", epnum);
		rx_csr &= ~MUSBFSH_RXCSR_H_RXSTALL;
		musbfsh_writew(epio, MUSBFSH_RXCSR, rx_csr);
		/* stall; record URB status */
		status = -EPIPE;

	} else if (rx_csr & MUSBFSH_RXCSR_H_ERROR) {
		WARNING("end %d RX proto error\n", epnum);

		status = -EPROTO;
		musbfsh_writeb(epio, MUSBFSH_RXINTERVAL, 0);

	} else if (rx_csr & MUSBFSH_RXCSR_DATAERROR) {
		if (qh->type != USB_ENDPOINT_XFER_ISOC) {
			INFO("RX end %d NAK timeout\n", epnum);
			/* removed due to too many logs */

			/* NOTE: NAKing is *NOT* an error, so we want to
			 * continue.  Except ... if there's a request for
			 * another QH, use that instead of starving it.
			 *
			 * Devices like Ethernet and serial adapters keep
			 * reads posted at all times, which will starve
			 * other devices without this logic.
			 */
			if (usb_pipebulk(urb->pipe) && qh->mux == 1 &&
			    !list_is_singular(&musbfsh->in_bulk)) {
				musbfsh_bulk_rx_nak_timeout(musbfsh, hw_ep);
				return;
			}
			musbfsh_ep_select(mbase, epnum);
			rx_csr |= MUSBFSH_RXCSR_H_WZC_BITS;
			rx_csr &= ~MUSBFSH_RXCSR_DATAERROR;
			musbfsh_writew(epio, MUSBFSH_RXCSR, rx_csr);
			goto finish;
		} else {
			INFO("RX end %d ISO data error\n", epnum);
			/* packet error reported later */
			iso_err = true;
		}
	} else if (rx_csr & MUSBFSH_RXCSR_INCOMPRX) {
		WARNING("end %d high bandwidth incomplete ISO packet RX\n",
			epnum);
		status = -EPROTO;
	}

	/* faults abort the transfer */
	if (status) {
		/* clean up dma and collect transfer count */
		if (dma_channel_status(dma) == MUSBFSH_DMA_STATUS_BUSY) {
			dma->status = MUSBFSH_DMA_STATUS_CORE_ABORT;
			(void)musbfsh->dma_controller->channel_abort(dma);
			xfer_len = dma->actual_len;
		}
		musbfsh_h_flush_rxfifo(hw_ep, 0);
		musbfsh_writeb(epio, MUSBFSH_RXINTERVAL, 0);

#ifdef CONFIG_MTK_DT_USB_SUPPORT
		if (!musbfsh_connect_flag) {
			MYDBG("err(%d) after disc\n", status);
			return;
		}
#endif

		done = true;
		goto finish;
	}

	if (unlikely(dma_channel_status(dma) == MUSBFSH_DMA_STATUS_BUSY)) {
		/* SHOULD NEVER HAPPEN ... but at least DaVinci has done it */
		ERR("RX%d dma busy, csr %04x\n", epnum, rx_csr);
		goto finish;
	}

	/* thorough shutdown for now ... given more precise fault handling
	 * and better queueing support, we might keep a DMA pipeline going
	 * while processing this irq for earlier completions.
	 */

	/* FIXME this is _way_ too much in-line logic for Mentor DMA */
	/* here rx_csr & MUSBFSH_RXCSR_DMAENAB is very very important! */
	if (dma && (rx_csr & MUSBFSH_RXCSR_DMAENAB)) {
		xfer_len = dma->actual_len;

		/* These should be clear! */
		val &= ~(MUSBFSH_RXCSR_DMAENAB |
			MUSBFSH_RXCSR_H_AUTOREQ |
			MUSBFSH_RXCSR_AUTOCLEAR |
			MUSBFSH_RXCSR_RXPKTRDY);
		musbfsh_writew(hw_ep->regs, MUSBFSH_RXCSR, val);

		if (usb_pipeisoc(pipe)) {
			struct usb_iso_packet_descriptor *d;

			d = urb->iso_frame_desc + qh->iso_idx;
			d->actual_length = xfer_len;

			/* even if there was an error, we did the dma
			 * for iso_frame_desc->length
			 */
			if (d->status != -EILSEQ && d->status != -EOVERFLOW)
				d->status = 0;

			if (++qh->iso_idx >= urb->number_of_packets)
				done = true;
			else
				done = false;
		} else
			done = (urb->actual_length + xfer_len >=
				urb->transfer_buffer_length ||
				dma->actual_len < qh->maxpacket);

		/* send IN token for next packet, without AUTOREQ */
		if (!done) {
			val |= MUSBFSH_RXCSR_H_REQPKT;
			musbfsh_writew(epio, MUSBFSH_RXCSR,
				       MUSBFSH_RXCSR_H_WZC_BITS | val);
		}

		INFO("ep %d dma %s, rxcsr %04x, rxcount %d\n", epnum,
		     done ? "off" : "reset",
		     musbfsh_readw(epio, MUSBFSH_RXCSR),
				   musbfsh_readw(epio, MUSBFSH_RXCOUNT));
	} else if (urb->status == -EINPROGRESS) {
		/* if no errors, be sure a packet is ready for unloading */
		if (unlikely(!(rx_csr & MUSBFSH_RXCSR_RXPKTRDY))) {
			status = -EPROTO;
			ERR("Rx interrupt with no errors or packet!\n");

			/* FIXME this is another "SHOULD NEVER HAPPEN" */

			/* SCRUB (RX) */
			/* do the proper sequence to abort the transfer */
			musbfsh_ep_select(mbase, epnum);
			val &= ~MUSBFSH_RXCSR_H_REQPKT;
			musbfsh_writew(epio, MUSBFSH_RXCSR, val);
			goto finish;
		}

		/* we are expecting IN packets */
		if (dma) {
			struct dma_controller *c;
			u16 rx_count;
			int ret, length;
			dma_addr_t buf;

			rx_count = musbfsh_readw(epio, MUSBFSH_RXCOUNT);

			INFO("RX%d count %d, buffer 0x%x len %d/%d\n",
			     epnum, rx_count,
			     (unsigned int)urb->transfer_dma
			     + urb->actual_length, qh->offset,
			     urb->transfer_buffer_length);

			c = musbfsh->dma_controller;
			if (usb_pipeisoc(pipe)) {
				int d_status = 0;
				struct usb_iso_packet_descriptor *d;

				d = urb->iso_frame_desc + qh->iso_idx;

				if (iso_err) {
					d_status = -EILSEQ;
					urb->error_count++;
				}
				if (rx_count > d->length) {
					if (d_status == 0) {
						d_status = -EOVERFLOW;
						urb->error_count++;
					}
					INFO("** OVERFLOW %d into %d\n",
						rx_count, d->length);

					length = d->length;
				} else
					length = rx_count;
				d->status = d_status;
				buf = urb->transfer_dma + d->offset;
			} else {
				length = rx_count;
				buf = urb->transfer_dma + urb->actual_length;
			}
			dma->desired_mode = 0;
#ifdef USE_MODE1
			/* because of the issue below, mode 1 will
			 * only rarely behave with correct semantics.
			 */
			if ((urb->transfer_flags & URB_SHORT_NOT_OK) &&
			    (urb->transfer_buffer_length - urb->actual_length) >
			    qh->maxpacket)
				dma->desired_mode = 1;
			if (rx_count < hw_ep->max_packet_sz_rx) {
				length = rx_count;
				dma->desired_mode = 0;
			} else {
				length = urb->transfer_buffer_length;
			}
#endif

			/*
			 * Disadvantage of using mode 1:
			 *      It's basically usable only for mass storage
			 *	class; essentially all other protocols also
			 *	terminate transfers on short packets.
			 *
			 * Details:
			 *      An extra IN token is sent at the end of the
			 *	transfer (due to AUTOREQ). If you try to use
			 *	mode 1 for (transfer_buffer_length - 512),
			 *	and try to use the extra IN token to grab the
			 *	last packet using mode 0, then the problem is
			 *	that you cannot be sure when the device will
			 *	send the last packet and RxPktRdy set.
			 *	Sometimes the packet is recd too soon such that
			 *	it gets lost when RxCSR is re-set at the end of
			 *	the mode 1 transfer, while sometimes it is recd
			 *	just a little late so that if you try to
			 *	configure for mode 0 soon after the mode 1
			 *	transfer is completed, you will find rxcount 0.
			 *
			 *	Okay, so you might think why not
			 *      wait for an interrupt when the pkt is recd.
			 *	Well, you won't get any!
			 */

			val = musbfsh_readw(epio, MUSBFSH_RXCSR);
			val &= ~MUSBFSH_RXCSR_H_REQPKT;

			if (dma->desired_mode == 0)
				val &= ~MUSBFSH_RXCSR_H_AUTOREQ;
			else
				val |= MUSBFSH_RXCSR_H_AUTOREQ;

			val |= MUSBFSH_RXCSR_DMAENAB;

			musbfsh_writew(epio, MUSBFSH_RXCSR,
				       MUSBFSH_RXCSR_H_WZC_BITS | val);

			/* REVISIT if when actual_length != 0,
			 * transfer_buffer_length needs to be
			 * adjusted first...
			 */
			/*
			 * dma is a dma channel, which is already allocated for
			 * the Rx EP in the func:musbfsh_ep_program
			 */
			ret = c->channel_program(dma, qh->maxpacket,
						 dma->desired_mode, buf,
						 length);

			if (!ret) {
				c->channel_release(dma);
				hw_ep->rx_channel = NULL;
				dma = NULL;
				/* REVISIT reset CSR */
			}
		}

		if (!dma) {
			/* Unmap the buffer so that CPU can use it */
			usb_hcd_unmap_urb_for_dma(musbfsh_to_hcd(musbfsh), urb);
			done = musbfsh_host_packet_rx(musbfsh,
				urb, epnum, iso_err);
			INFO("read %spacket\n", done ? "last " : "");
		}
	}

finish:
	urb->actual_length += xfer_len;
	qh->offset += xfer_len;
	if (done) {
		if (urb->status == -EINPROGRESS)
			urb->status = status;
		musbfsh_advance_schedule(musbfsh, urb, hw_ep, USB_DIR_IN);
	}
}

static int musbfsh_schedule(struct musbfsh *musbfsh, struct musbfsh_qh *qh,
			    int is_in)
{
	int idle;
	int epnum, hw_end = 0;
	struct musbfsh_hw_ep *hw_ep = NULL;
	struct list_head *head = NULL;

	INFO("%s++, qh->epnum=%d, is_in=%d\r\n",
	     __func__, qh->epnum, (unsigned int)is_in);
	/* use fixed hardware for control and bulk */
	if (qh->type == USB_ENDPOINT_XFER_CONTROL) {
		head = &musbfsh->control;
		hw_ep = musbfsh->control_ep;
		goto success;
	}

#ifdef MUSBFSH_QMU_LIMIT_SUPPORT
	if (mtk11_isoc_ep_gpd_count
		&& qh->is_use_qmu) {
		for (epnum = 1, hw_ep = musbfsh->endpoints + 1;
				epnum <= MAX_QMU_EP; epnum++, hw_ep++) {
			/* int	diff; */

			if (musbfsh_ep_get_qh(hw_ep, is_in) != NULL)
				continue;

			hw_end = epnum;
			/* got the right ep */
			hw_ep = musbfsh->endpoints + hw_end;
			break;
		}

		if (hw_end) {
			idle = 1;
			qh->mux = 0;
			goto success;
		}
	}
	qh->is_use_qmu = 0;
	for (epnum = (MAX_QMU_EP + 1),
		hw_ep = musbfsh->endpoints + (MAX_QMU_EP + 1);
		epnum < musbfsh->nr_endpoints; epnum++, hw_ep++) {
		if (musbfsh_ep_get_qh(hw_ep, is_in) != NULL)
			continue;

		hw_end = epnum;
		hw_ep = musbfsh->endpoints + hw_end; /* got the right ep */
		break;
	}

	if (hw_end) {
		idle = 1;
		qh->mux = 0;
		goto success;
	}

	if (!hw_end) {
		for (epnum = 1, hw_ep = musbfsh->endpoints + 1;
				epnum <= MAX_QMU_EP; epnum++, hw_ep++) {
			/* int	diff; */

			if (musbfsh_ep_get_qh(hw_ep, is_in) != NULL)
				continue;

			hw_end = epnum;
			/* got the right ep */
			hw_ep = musbfsh->endpoints + hw_end;
			break;
		}
	}

	if (hw_end) {
		idle = 1;
		qh->mux = 0;
		goto success;
	} else {
		WARNING("EP OVERFLOW.\n");
		return -ENOSPC;
	}
#endif

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
	if (mtk11_isoc_ep_gpd_count
		&& qh->type == USB_ENDPOINT_XFER_ISOC) {
		for (epnum = mtk11_isoc_ep_start_idx,
			hw_ep = musbfsh->endpoints + mtk11_isoc_ep_start_idx;
			epnum < musbfsh->nr_endpoints; epnum++, hw_ep++) {
			/* int	diff; */

			if (musbfsh_ep_get_qh(hw_ep, is_in) != NULL)
				continue;

			hw_end = epnum;
			/* got the right ep */
			hw_ep = musbfsh->endpoints + hw_end;
			break;
		}

		if (hw_end) {
			idle = 1;
			qh->mux = 0;
			goto success;
		}
	}
#endif

	/* else, periodic transfers get muxed to other endpoints */

	/*
	 * We know this qh hasn't been scheduled, so all we need to do
	 * is choose which hardware endpoint to put it on ...
	 *
	 * REVISIT what we really want here is a regular schedule tree
	 * like e.g. OHCI uses.
	 */
	for (epnum = 1, hw_ep = musbfsh->endpoints + 1;
	     epnum < musbfsh->nr_endpoints; epnum++, hw_ep++) {

		if (musbfsh_ep_get_qh(hw_ep, is_in) != NULL)
			continue;

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
		if (mtk11_isoc_ep_gpd_count &&
			(epnum >= mtk11_isoc_ep_start_idx)) {
			epnum = musbfsh->nr_endpoints;
			continue;
		}
#endif

		hw_end = epnum;
		hw_ep = musbfsh->endpoints + hw_end;	/* got the right ep */
		break;
	}

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
	/* grab isoc ep if no other ep is available */
	if (mtk11_isoc_ep_gpd_count &&
		!hw_end &&
		qh->type != USB_ENDPOINT_XFER_ISOC) {
		for (epnum = mtk11_isoc_ep_start_idx,
			hw_ep = musbfsh->endpoints + mtk11_isoc_ep_start_idx;
			epnum < musbfsh->nr_endpoints; epnum++, hw_ep++) {
			/* int	diff; */

			if (musbfsh_ep_get_qh(hw_ep, is_in) != NULL)
				continue;

			hw_end = epnum;
			/* got the right ep */
			hw_ep = musbfsh->endpoints + hw_end;
			break;
		}
	}
#endif
	if (!hw_end)
		return -ENOSPC;

	idle = 1;
	qh->mux = 0;
success:
	if (head) {
#ifdef CONFIG_MTK_DT_USB_SUPPORT
		MYDBG("head!=NULL\n");
#endif
		idle = list_empty(head);
		list_add_tail(&qh->ring, head);
		qh->mux = 1;
	}
	qh->hw_ep = hw_ep;
	qh->hep->hcpriv = qh;

	if (musbfsh_host_dynamic_fifo &&
		qh->type != USB_ENDPOINT_XFER_CONTROL) {
		int ret;

		/* take this after qh->hw_ep is set */
		ret = musbfsh_host_alloc_ep_fifo(musbfsh, qh, is_in);
		if (ret) {
			qh->hw_ep = NULL;
			qh->hep->hcpriv = NULL;
			WARNING("NOT ENOUGH FIFO\n");
			return -ENOSPC;
		}
	}
	hw_ep->type = qh->type;
	/* the new urb added is the first urb now, excute it! */
	if (idle) {
#ifdef CONFIG_MTK_DT_USB_SUPPORT
		mark_qh_activity(qh->epnum, hw_ep->epnum, is_in, 0);
#endif

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
#ifdef MUSBFSH_QMU_LIMIT_SUPPORT
		if (mtk11_isoc_ep_gpd_count &&
			qh->is_use_qmu &&
			hw_end <= MAX_QMU_EP)
			qh->is_use_qmu = 1;
#else
		if (mtk11_isoc_ep_gpd_count &&
			qh->type == USB_ENDPOINT_XFER_ISOC &&
			hw_end < mtk11_isoc_ep_start_idx)
			qh->is_use_qmu = 0;
#endif
		if (qh->is_use_qmu) {
			musbfsh_ep_set_qh(hw_ep, is_in, qh);
			mtk11_kick_CmdQ(musbfsh, is_in ? 1:0,
				qh, next_urb(qh));
		} else
			musbfsh_start_urb(musbfsh, is_in, qh);
#else
		musbfsh_start_urb(musbfsh, is_in, qh);
#endif

	}
	return 0;
}

static int musbfsh_urb_enqueue(struct usb_hcd *hcd, struct urb *urb,
			       gfp_t mem_flags)
{
	unsigned long flags;
	struct musbfsh *musbfsh = hcd_to_musbfsh(hcd);
	struct usb_host_endpoint *hep = urb->ep;
	struct musbfsh_qh *qh;
	struct usb_endpoint_descriptor *epd = &hep->desc;
	int ret;
	unsigned int type_reg;
	unsigned int interval;

	INFO("urb addr=0x%p\r\n", urb);

	/*
	 * MYDBG("urb:%x, blen:%d, alen:%d, ep:%x\n",
	 * urb, urb->transfer_buffer_length, urb->actual_length,
	 * epd->bEndpointAddress);
	 */
	/*
	 * workaround for DMA issue,
	 * to make usb core jump over unmap_urb_for_dma
	 * in usb_hcd_giveback_urb for control message
	 */
	if (usb_endpoint_num(epd) == 0)
		urb->transfer_flags &= ~URB_DMA_MAP_SINGLE;
	spin_lock_irqsave(&musbfsh->lock, flags);

	/* add the urb to the ep, return 0 for no error. */
	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	qh = ret ? NULL : hep->hcpriv;
	if (qh)
		urb->hcpriv = qh;
	spin_unlock_irqrestore(&musbfsh->lock, flags);

	/* DMA mapping was already done, if needed, and this urb is on
	 * hep->urb_list now ... so we're done, unless hep wasn't yet
	 * scheduled onto a live qh.
	 *
	 * REVISIT best to keep hep->hcpriv valid until the endpoint gets
	 * disabled, testing for empty qh->ring and avoiding qh setup costs
	 * except for the first urb queued after a config change.
	 */
#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
	if (mtk11_host_qmu_concurrent && qh && qh->is_use_qmu && (ret == 0)) {
		mtk11_kick_CmdQ(musbfsh,
			(epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? 1:0,
			qh, urb);
		return ret;
	}
#endif
	if (qh || ret)
		return ret;

	/* Allocate and initialize qh, minimizing the work done each time
	 * hw_ep gets reprogrammed, or with irqs blocked.  Then schedule it.
	 *
	 * REVISIT consider a dedicated qh kmem_cache, so it's harder
	 * for bugs in other kernel code to break this driver...
	 */
	qh = kzalloc(sizeof(*qh), mem_flags);
	if (!qh) {
		spin_lock_irqsave(&musbfsh->lock, flags);
		usb_hcd_unlink_urb_from_ep(hcd, urb);
		spin_unlock_irqrestore(&musbfsh->lock, flags);
		return -ENOMEM;
	}

	qh->hep = hep;
	qh->dev = urb->dev;
	INIT_LIST_HEAD(&qh->ring);
	qh->is_ready = 1;

	qh->maxpacket = le16_to_cpu(epd->wMaxPacketSize);
	qh->type = usb_endpoint_type(epd);
	INFO("desc type=%d\r\n", qh->type);
	/* Bits 11 & 12 of wMaxPacketSize encode high bandwidth multiplier.
	 * Some musb cores don't support high bandwidth ISO transfers; and
	 * we don't (yet!) support high bandwidth interrupt transfers.
	 */
	qh->hb_mult = 1 + ((qh->maxpacket >> 11) & 0x03);
	if (qh->hb_mult > 1) {
		int ok = (qh->type == USB_ENDPOINT_XFER_ISOC);

		if (ok)
			ok = (usb_pipein(urb->pipe) && musbfsh->hb_iso_rx) ||
				(usb_pipeout(urb->pipe) && musbfsh->hb_iso_tx);

		if (!ok) {
			ret = -EMSGSIZE;
			goto done;
		}
		qh->maxpacket &= 0x7ff;
	}
	qh->epnum = usb_endpoint_num(epd);
	INFO("desc epnum=%d\r\n", qh->epnum);

	/* NOTE: urb->dev->devnum is wrong during SET_ADDRESS */
	qh->addr_reg = (u8) usb_pipedevice(urb->pipe);
	INFO("desc pipe=0x%x, desc devnum=%d\r\n",
		urb->pipe, urb->dev->devnum);

	/* precompute rxtype/txtype/type0 register */
	type_reg = (qh->type << 4) | qh->epnum;
	switch (urb->dev->speed) {
	case USB_SPEED_LOW:
		type_reg |= 0xc0;
		break;
	case USB_SPEED_FULL:
		type_reg |= 0x80;
		break;
	default:
		type_reg |= 0x40;
	}
	qh->type_reg = type_reg;

	/* Precompute RXINTERVAL/TXINTERVAL register */
	switch (qh->type) {
	case USB_ENDPOINT_XFER_INT:
		/*
		 * Full/low speeds use the  linear encoding,
		 * high speed uses the logarithmic encoding.
		 */
		if (urb->dev->speed <= USB_SPEED_FULL) {
			interval = max_t(u8, epd->bInterval, 1);
			break;
		}
	case USB_ENDPOINT_XFER_ISOC:
		/* ISO always uses logarithmic encoding */
		interval = min_t(u8, epd->bInterval, 16);
		break;
	default:
		/* REVISIT we actually want to use NAK limits, hinting to the
		 * transfer scheduling logic to try some other qh, e.g. try
		 * for 2 msec first:
		 *
		 * interval = (USB_SPEED_HIGH == urb->dev->speed) ? 16 : 2;
		 *
		 * The downside of disabling this is that transfer scheduling
		 * gets VERY unfair for nonperiodic transfers; a misbehaving
		 * peripheral could make that hurt.  That's perfectly normal
		 * for reads from network or serial adapters ... so we have
		 * partial NAKlimit support for bulk RX.
		 *
		 * The upside of disabling it is simpler transfer scheduling.
		 */
		interval = 0;
	}
	qh->intv_reg = interval;

	/* precompute addressing for external hub/tt ports */
	if (musbfsh->is_multipoint) {
		struct usb_device *parent = urb->dev->parent;

		if (parent != hcd->self.root_hub) {
			qh->h_addr_reg = (u8) parent->devnum;

			/* set up tt info if needed */
			if (urb->dev->tt) {
				qh->h_port_reg = (u8) urb->dev->ttport;
				if (urb->dev->tt->hub)
					qh->h_addr_reg =
						(u8)urb->dev->tt->hub->devnum;
				if (urb->dev->tt->multi)
					qh->h_addr_reg |= 0x80;
			}
		}
		INFO("addr_reg=0x%x,h_addr_reg=0x%x,h_port_reg=0x%x",
			qh->addr_reg, qh->h_addr_reg, qh->h_port_reg);
	}

	/* invariant: hep->hcpriv is null OR the qh that's already scheduled.
	 * until we get real dma queues (with an entry for each urb/buffer),
	 * we only have work to do in the former case.
	 */
	spin_lock_irqsave(&musbfsh->lock, flags);
	if (hep->hcpriv) {
		/* some concurrent activity submitted another urb to hep...
		 * odd, rare, error prone, but legal.
		 */
		kfree(qh);
		qh = NULL;
		ret = 0;
	} else {
#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
		if ((!usb_pipecontrol(urb->pipe)) &&
			((usb_pipetype(urb->pipe) + 1) &
			 mtk11_host_qmu_pipe_msk))
			qh->is_use_qmu = 1;
#endif
		ret = musbfsh_schedule(musbfsh, qh,
			(epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK));
		/*
		 * MYDBG("after musbfsh_schedule,
		 * urb:%x, ret:%d, ep:%x\n", urb, ret,
		 * epd->bEndpointAddress);
		 */
	}

	if (ret == 0) {
		urb->hcpriv = qh;
		/*
		 * FIXME: set urb->start_frame for iso/intr, it's tested in
		 * musbfsh_start_urb(), but otherwise only konicawc cares ...
		 */
	}
	spin_unlock_irqrestore(&musbfsh->lock, flags);
done:
	if (ret != 0) {
		spin_lock_irqsave(&musbfsh->lock, flags);
		usb_hcd_unlink_urb_from_ep(hcd, urb);
		spin_unlock_irqrestore(&musbfsh->lock, flags);
		kfree(qh);
	}
	return ret;
}

static int musbfsh_cleanup_urb(struct urb *urb, struct musbfsh_qh *qh)
{
	struct musbfsh_hw_ep *ep = qh->hw_ep;
	void __iomem *epio = ep->regs;
	unsigned int hw_end = ep->epnum;
	void __iomem *regs = ep->musbfsh->mregs;
	int is_in = usb_pipein(urb->pipe);
	int stat = 0;
	u16 csr;

	INFO("%s++\r\n", __func__);
	musbfsh_ep_select(regs, hw_end);

	if (is_dma_capable()) {
		struct dma_channel *dma;

		dma = is_in ? ep->rx_channel : ep->tx_channel;
		if (dma) {
			stat = ep->musbfsh->dma_controller->channel_abort(dma);
			WARNING("abort %cX%d DMA for urb %p --> %d\n",
				is_in ? 'R' : 'T', ep->epnum, urb, stat);
			urb->actual_length += dma->actual_len;
		}
	}

	/* turn off DMA requests, discard state, stop polling ... */
	if (is_in) {
		/* giveback saves bulk toggle */
		csr = musbfsh_h_flush_rxfifo(ep, 0);

		/* REVISIT we still get an irq; should likely clear the
		 * endpoint's irq stat here to avoid bogus irqs.
		 * clearing that stat is platform-specific...
		 */
	} else if (ep->epnum) {
		musbfsh_h_tx_flush_fifo(ep);
		csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
		csr &= ~(MUSBFSH_TXCSR_AUTOSET
			 | MUSBFSH_TXCSR_DMAENAB
			 | MUSBFSH_TXCSR_H_RXSTALL
			 | MUSBFSH_TXCSR_H_NAKTIMEOUT
			 | MUSBFSH_TXCSR_H_ERROR | MUSBFSH_TXCSR_TXPKTRDY);
		musbfsh_writew(epio, MUSBFSH_TXCSR, csr);
		/* REVISIT may need to clear FLUSHFIFO ... */
		musbfsh_writew(epio, MUSBFSH_TXCSR, csr);
		/* flush cpu writebuffer */
		csr = musbfsh_readw(epio, MUSBFSH_TXCSR);
	} else {
		musbfsh_h_ep0_flush_fifo(ep);
	}
	if (stat == 0)
		musbfsh_advance_schedule(ep->musbfsh, urb, ep, is_in);
	return stat;
}

static int musbfsh_urb_dequeue(struct usb_hcd *hcd,
	struct urb *urb, int status)
{
	struct musbfsh *musbfsh = hcd_to_musbfsh(hcd);
	struct musbfsh_qh *qh;
	unsigned long flags;
	int is_in = usb_pipein(urb->pipe);
	int ret;

	INFO("urb=%p, dev%d ep%d%s\n", urb,
		usb_pipedevice(urb->pipe), usb_pipeendpoint(urb->pipe),
		is_in ? "in" : "out");

	spin_lock_irqsave(&musbfsh->lock, flags);
	ret = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (ret)
		goto done;

	qh = urb->hcpriv;
	if (!qh)
		goto done;

	/*
	 * Any URB not actively programmed into endpoint hardware can be
	 * immediately given back; that's any URB not at the head of an
	 * endpoint queue, unless someday we get real DMA queues.  And even
	 * if it's at the head, it might not be known to the hardware...
	 *
	 * Otherwise abort current transfer, pending DMA, etc.; urb->status
	 * has already been updated.  This is a synchronous abort; it'd be
	 * OK to hold off until after some IRQ, though.
	 *
	 * NOTE: qh is invalid unless !list_empty(&hep->urb_list)
	 */
	if (!qh->is_ready
	    || urb->urb_list.prev != &qh->hep->urb_list
	    || musbfsh_ep_get_qh(qh->hw_ep, is_in) != qh) {
		int ready = qh->is_ready;

		qh->is_ready = 0;
		musbfsh_giveback(musbfsh, urb, 0);
		qh->is_ready = ready;

		/* If nothing else (usually musbfsh_giveback) is using it
		 * and its URB list has emptied, recycle this qh.
		 */
		if (ready && list_empty(&qh->hep->urb_list)) {
#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
			if (qh->is_use_qmu)
				mtk11_disable_q(musbfsh,
					qh->hw_ep->epnum, is_in);
#endif
			qh->hep->hcpriv = NULL;
			list_del(&qh->ring);
			if (musbfsh_host_dynamic_fifo &&
				qh->type != USB_ENDPOINT_XFER_CONTROL)
				musbfsh_host_free_ep_fifo(musbfsh, qh, is_in);
			kfree(qh);
		}
	} else
		ret = musbfsh_cleanup_urb(urb, qh);
done:
	spin_unlock_irqrestore(&musbfsh->lock, flags);
	return ret;
}

/* disable an endpoint */
static void musbfsh_h_disable(struct usb_hcd *hcd,
			      struct usb_host_endpoint *hep)
{
	u8 is_in = hep->desc.bEndpointAddress & USB_DIR_IN;
	unsigned long flags;
	struct musbfsh *musbfsh = hcd_to_musbfsh(hcd);
	struct musbfsh_qh *qh;
	struct urb *urb;

	WARNING("%s++: ep: 0x%x\r\n",
		__func__, hep->desc.bEndpointAddress);
	spin_lock_irqsave(&musbfsh->lock, flags);

	qh = hep->hcpriv;
	if (qh == NULL) {
		MYDBG("qh == NULL\n");
		goto exit;
	}

	/* NOTE: qh is invalid unless !list_empty(&hep->urb_list) */

	/* Kick the first URB off the hardware, if needed */
	qh->is_ready = 0;
	if (musbfsh_ep_get_qh(qh->hw_ep, is_in) == qh) {
		urb = next_urb(qh);

		/*
		 * work around from tablet,
		 * avoid KE for qh->hep content 0x6b6b6b6b...
		 * side effect will cause touch memory after free
		 */

		/*
		 * enable this workaround for
		 * irq->adv_schedule / musbfsh_h_disable
		 * cocurrency issue
		 */
		if (!virt_addr_valid(urb)) {
			MYDBG("urb(%p) addr error\n", urb);
			goto exit;
		}
		/* make software (then hardware) stop ASAP */
		if (!urb->unlinked)
			urb->status = -ESHUTDOWN;

		/* cleanup */
		musbfsh_cleanup_urb(urb, qh);

		/* Then nuke all the others ... and advance the
		 * queue on hw_ep (e.g. bulk ring) when we're done.
		 */
		while (!list_empty(&hep->urb_list)) {
			urb = next_urb(qh);
			urb->status = -ESHUTDOWN;
			musbfsh_advance_schedule(musbfsh, urb, qh->hw_ep,
						 is_in);
		}
	} else {
		/* Just empty the queue; the hardware is busy with
		 * other transfers, and since !qh->is_ready nothing
		 * will activate any of these as it advances.
		 */
		while (!list_empty(&hep->urb_list))
			musbfsh_giveback(musbfsh, next_urb(qh), -ESHUTDOWN);

#ifdef CONFIG_MTK_MUSBFSH_QMU_SUPPORT
		if (qh->is_use_qmu)
			mtk11_disable_q(musbfsh, qh->hw_ep->epnum, is_in);
#endif
		hep->hcpriv = NULL;
		list_del(&qh->ring);

		if (musbfsh_host_dynamic_fifo &&
			qh->type != USB_ENDPOINT_XFER_CONTROL)
			musbfsh_host_free_ep_fifo(musbfsh, qh, is_in);
		kfree(qh);
	}
exit:
	spin_unlock_irqrestore(&musbfsh->lock, flags);
}

static int musbfsh_h_get_frame_number(struct usb_hcd *hcd)
{
	struct musbfsh *musbfsh = hcd_to_musbfsh(hcd);

	return musbfsh_readw(musbfsh->mregs, MUSBFSH_FRAME);
}

static int musbfsh_h_start(struct usb_hcd *hcd)
{
	struct musbfsh *musbfsh = hcd_to_musbfsh(hcd);

	DBG(1, "called++\r\n");
	/* NOTE: musbfsh_start() is called when the hub driver turns
	 * on port power, or when (OTG) peripheral starts.
	 */
	hcd->state = HC_STATE_RUNNING;
	musbfsh->port1_status = 0;
	return 0;
}

static void musbfsh_h_stop(struct usb_hcd *hcd)
{
	DBG(1, "called\r\n");
	musbfsh_stop(hcd_to_musbfsh(hcd));
	hcd->state = HC_STATE_HALT;
}

/* only send suspend signal to bus */
static int musbfsh_bus_suspend(struct usb_hcd *hcd)
{
	struct musbfsh *musbfsh = hcd_to_musbfsh(hcd);
	unsigned char power = musbfsh_readb(musbfsh->mregs, MUSBFSH_POWER);

	WARNING("power=0x%x\n", power);
#ifdef CONFIG_MTK_DT_USB_SUPPORT
#if defined(CONFIG_PM_RUNTIME)
	usb11_plat_suspend();
#endif
#endif

#ifdef CONFIG_MTK_DT_USB_SUPPORT
#if defined(CONFIG_PM_RUNTIME) && defined(USB11_REMOTE_IRQ_NON_AUTO_MASK)
	enable_remote_wake_up();
#endif
#endif

#ifdef MTK_USB_RUNTIME_SUPPORT
	/*
	 * Edge triggered EINT interrupt will be hold after masked (only one),
	 * and reported after unmasked.
	 */
	mt_eint_unmask(CUST_EINT_MT6280_USB_WAKEUP_NUM);
#endif
	/*
	 * wx, let child port do the job;
	 * joson,runtime suspend not ready now,i
	 * set suspend signal here
	 */

	/* suspend bus */
#ifdef MUSBFSH_ENABLE_BUS_SUSPEND
	power = musbfsh_readb(musbfsh->mregs, MUSBFSH_POWER);
	DBG(1, "power:0x%x\n", power);
	power |= MUSBFSH_POWER_SUSPENDM | MUSBFSH_POWER_ENSUSPEND;
	musbfsh_writeb(musbfsh->mregs, MUSBFSH_POWER, power);
	mdelay(15);
	power = musbfsh_readb(musbfsh->mregs, MUSBFSH_POWER);
	DBG(1, "after set, power:0x%x\n", power);
#endif

	return 0;
}

/* only send resume signal to bus */
static int musbfsh_bus_resume(struct usb_hcd *hcd)
{
	/* resuming child port does the work */
	struct musbfsh *musbfsh = hcd_to_musbfsh(hcd);
	unsigned char power;

#ifdef CONFIG_MTK_DT_USB_SUPPORT
#if defined(CONFIG_PM_RUNTIME)
	usb11_plat_resume();
	return 0;
#endif
#endif

#ifdef MTK_USB_RUNTIME_SUPPORT
	mt_eint_mask(CUST_EINT_MT6280_USB_WAKEUP_NUM);
#endif
	power = musbfsh_readb(musbfsh->mregs, MUSBFSH_POWER);
	DBG(0, "power=0x%x\r\n", power);

	/*
	 * wx, let child port do the job;
	 * joson,runtime suspend not ready now,
	 * set resume signal here
	 */
	/* resume bus */
#ifdef MUSBFSH_ENABLE_BUS_SUSPEND
	power |= MUSBFSH_POWER_RESUME;
	power &= ~MUSBFSH_POWER_SUSPENDM;
	musbfsh_writeb(musbfsh->mregs, MUSBFSH_POWER, power);
	mdelay(30);
	power &= ~MUSBFSH_POWER_RESUME;
	musbfsh_writeb(musbfsh->mregs, MUSBFSH_POWER, power);

	power = musbfsh_readb(musbfsh->mregs, MUSBFSH_POWER);
	DBG(1, "after set, power:0x%x\n", power);
#endif

	return 0;
}

struct hc_driver musbfsh_hc_driver = {
	.description = "musbfsh-hcd",
	.product_desc = "MUSBFSH HDRC host driver",
	.hcd_priv_size = sizeof(struct musbfsh),
	.flags = HCD_USB2 | HCD_MEMORY,

	/*
	 * not using irq handler or reset hooks from usbcore, since
	 * those must be shared with peripheral code for OTG configs
	 */

	.start = musbfsh_h_start,
	.stop = musbfsh_h_stop,

	.get_frame_number = musbfsh_h_get_frame_number,

	.urb_enqueue = musbfsh_urb_enqueue,
	.urb_dequeue = musbfsh_urb_dequeue,
	.endpoint_disable = musbfsh_h_disable,

	.hub_status_data = musbfsh_hub_status_data,
	.hub_control = musbfsh_hub_control,
	.bus_suspend = musbfsh_bus_suspend,
	.bus_resume = musbfsh_bus_resume,
	/* .start_port_reset    = NULL, */
	/* .hub_irq_enable      = NULL, */
};