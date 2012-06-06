/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __CMD_ONENAND_H__
#define __CMD_ONENAND_H__


extern int onenand_block_read(loff_t from, size_t len, size_t *retlen, u_char *buf, int oob);
extern int onenand_block_write(loff_t to, size_t len, size_t *retlen, const u_char * buf);
extern int onenand_write_yaffs2(loff_t to, size_t len, size_t *retlen, const u_char * buf);
extern int onenand_block_erase(u32 start, u32 size, int force);


#endif
