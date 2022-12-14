/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _MDDP_F_DESC_H
#define _MDDP_F_DESC_H

enum {
	DESC_FLAG_FASTPATH = 0x00000001,
	DESC_FLAG_TRACK_BRIDGE = 0x00000002,
	DESC_FLAG_TRACK_NAT = 0x00000004,
	DESC_FLAG_TRACK_ROUTER = 0x00000008,
	DESC_FLAG_IPV4 = 0x00000010,
	DESC_FLAG_IPV6 = 0x00000020,
	DESC_FLAG_ESP_DECODE = 0x00000100,
	DESC_FLAG_ESP_ENCODE = 0x00000200,
	DESC_FLAG_UNKNOWN_ETYPE = 0x01000000,
	DESC_FLAG_UNKNOWN_PROTOCOL = 0x02000000,
	DESC_FLAG_IPFRAG = 0x04000000,	/* ip fragment */
	DESC_FLAG_MTU = 0x08000000,	/* too large packet to handle */
	DESC_FLAG_NOMEM = 0x10000000,
	DESC_FLAG_IPCSUM_ERROR = 0x20000000,
	DESC_FLAG_FASTSKB = 0x40000000,	/* For fastskb */
};

struct mddp_f_desc {
	u_int32_t flag;
	u_int16_t l3_off;
	u_int16_t l4_off;
	u_int32_t l3_len;
};

#endif /* _MDDP_F_DESC_H */
