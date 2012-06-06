/* u-boot/board/samsung/smdkc110/ide.h
 *
 * Copyright (C) 2009 Samsung Electronics
 *      http://samsungsemi.com/
 *
 * Header file for IDE support on SMDKV210
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __V210_IDE_H
#define __V210_IDE_H

void cf_outb(unsigned char val, volatile unsigned char *addr)
{
        *(addr) = val;
}

unsigned char cf_inb(volatile unsigned char *addr)
{
        volatile unsigned char c;

        c = *(addr);
        return c;
}

void cf_insw(unsigned short *sect_buf, unsigned short *addr, int words)
{
        int i;

        for (i = 0; i < words; i++)
                *(sect_buf + i) = *(addr);
}

void cf_outsw(unsigned short *addr, unsigned short *sect_buf, int words)
{
        int i;

        for (i = 0; i < words; i++)
                *(addr) = *(sect_buf + i);
}
#define inb(addr)               cf_inb((volatile unsigned char *)(addr))
#define outb(x, addr)           cf_outb((unsigned char)(x), (volatile unsigned char *)(addr))
#define insw(port, addr, count) cf_insw((unsigned short *)addr, (unsigned short *)(port), (count))
#define outsw(port, addr, count)        cf_outsw((unsigned short *)(port), (unsigned short *)addr, (count))

#endif
