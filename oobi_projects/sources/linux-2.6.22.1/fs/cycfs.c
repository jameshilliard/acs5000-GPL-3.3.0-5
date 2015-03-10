/*
 * Cyclades File System - cycfs 
 *
 * Copyright (C) 2004 
 *
 * This file system is used to access the internal flash
 *
 * Based on lwfs - demonstrate a trivial filesystem using libfs
 *               - author Jonathan Corbet
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pagemap.h> 	/* PAGE_CACHE_SIZE */
#include <linux/fs.h>     	/* This is where libfs stuff is declared */
#include <asm/atomic.h>
#include <asm/uaccess.h>	/* copy_to_user */
#include <asm/flash.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Regina Kodato");

#define CYCFS_MAGIC 0x20041029

#define FLASH_TYPE_RAM 0
#define FLASH_TYPE_CF 1

//[RK]Jun/13/05 - boot_ori RO
extern int flag_single_mode;

static int n_shareds = 0;
struct SHARED_SECTORS {
	int sector;
	int state;
};
static struct SHARED_SECTORS sh_sectors[10] = {{0}};

#define MAX_SH_ITEMS (sizeof(sh_sectors)/sizeof(struct SHARED_SECTORS))

#define FLAGS_OPENNED           1
#define FLAGS_CHANGED           2

#define CRC_FLAG_DoCRC          1
#define CRC_FLAG_DoSize         2
#define CRC_FLAG_Shared         4
#define CRC_FLAG_Protected      8

#define FLASH_DIV(x) ((x) >> cf->flash_shift)
#define FLASH_MOD(x) ((x) & (cf->flash_sector_size-1))

struct CycFlashPartition {
	int	flash_type;
	int	sector_start;
	int	sector_end;
	int	current_sector;
	int	flags;
	int 	size;
	unsigned char *buf;
	unsigned short crc_flag;
	unsigned short crc_calc;
	//[RK]Jun/13/05 - boot_ori RO
	int     offset_start;
	int     offset_end;
};

//[RK]Jun/13/05 - boot_ori RO
struct CycFlashCtr {
	int flash_sector_size;
	int flash_shift;
	int flash_length;
	int FlashNblkChip;
	int FlashNblkTotal;
};

static struct CycFlashCtr FlashControl;
static struct CycFlashCtr CF_Control;

#define FLASH_BOOT_ORI	0
#define FLASH_BOOT_INFO	1
#define FLASH_BOOT_ALT	2
#define FLASH_BOOT_CONF	3
#define FLASH_zIMAGE	4
#define FLASH_CONFIG	5

static struct CycFlashPartition cyc_flash[] = {
	{ .flash_type = FLASH_TYPE_RAM } , 
	{ .flash_type = FLASH_TYPE_RAM },
	{ .flash_type = FLASH_TYPE_RAM },
	{ .flash_type = FLASH_TYPE_RAM },
	{ .flash_type = FLASH_TYPE_CF },
	{ .flash_type = FLASH_TYPE_CF },
};

static int flash_locked;

static int cycfs_open(struct inode *inode, struct file *filp);
static ssize_t cycfs_read_file(struct file *filp, char *buf,
	size_t count, loff_t *offset);
static ssize_t cycfs_write_file(struct file *filp, const char *buf,
		size_t count, loff_t *offset);
static int cycfs_release_file(struct inode * inode, struct file * filp);

static int fill_sector_flash(int , int , char *, int , struct CycFlashPartition *, int );

// Cyclades Flash operations
static struct file_operations cycfs_file_ops = {
	.open	= cycfs_open,
	.read 	= cycfs_read_file,
	.write  = cycfs_write_file,
	.release = cycfs_release_file,
};

#if 0
static struct super_operations cycfs_s_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
};
#endif

static struct tree_descr ACSFiles[] = {
	{ NULL, 0, 0 },		// Skipped
	{ .name = "boot_ori",
	  .ops = &cycfs_file_ops,   
	  .mode = S_IRUSR },   // .mode = S_IWUSR | S_IRUSR },
	{ .name = ".info",
	  .ops = &cycfs_file_ops,
	  .mode = S_IWUSR | S_IRUSR },
	{ .name = "boot_alt",
	  .ops = &cycfs_file_ops,
	  .mode = S_IWUSR | S_IRUSR },
	{ .name = "boot_conf",
	  .ops = &cycfs_file_ops,
	  .mode = S_IWUSR | S_IRUSR },
	{ .name = "zImage",
	  .ops = &cycfs_file_ops,
	  .mode = S_IWUSR | S_IRUSR },
	{ .name = "config.tgz",
	  .ops = &cycfs_file_ops,
	  .mode = S_IWUSR | S_IRUSR },
	{ "", NULL, 0 }
};

// Shared Flash Sectors
static void add_shared_sector(int sector)
{
	int i;

	for(i=0; i < n_shareds; i++) {
		if (sh_sectors[i].sector == sector) return;
	}

	if (n_shareds >= MAX_SH_ITEMS) {
		panic("Flash - Shared sectors overflow");
	}

	sh_sectors[n_shareds++].sector = sector;
}

static int check_shared_sector(struct CycFlashPartition *fp, int start, int end)
{
	int i;

	if ((fp->crc_flag & CRC_FLAG_Shared)) {
		return(0);
	}
	for (i=0; i < n_shareds; i++) {
		if ((sh_sectors[i].sector >= start) &&
		    (sh_sectors[i].sector <= end)) {
			return(1);
		}
	}
	return(0);
}

static int lock_shared_sector(int start, int end)
{
	int i;

	for (i=0; i < n_shareds; i++) {
		if ((sh_sectors[i].sector >= start) &&
		    (sh_sectors[i].sector <= end)) {
			if (sh_sectors[i].state) {
				return(1);
			}
			sh_sectors[i].state = 1;
		}
	}
	return(0);
}

static void unlock_shared_sector(int start, int end)
{
	int i;

	for (i=0; i < n_shareds; i++) {
		if ((sh_sectors[i].sector >= start) &&
		    (sh_sectors[i].sector <= end)) {
			sh_sectors[i].state = 0;
		}
	}
}

// FLASH Operations

// * Returns the correct FLASH chip address (1st, 2nd or 3rd)
static char *FlashChipAddr(int sector)
{
	int div;
	struct CycFlashCtr *cf;

	cf = &FlashControl;

	if ((div = (sector / cf->FlashNblkChip)) == 0) {
		return(flash_addr);
	}
	return((div * cf->FlashNblkChip * cf->flash_sector_size) + flash_addr);
}

// * This routine generates the 16 bits CRC (X^16+X^12+X^5+1) for a
// *  stream of bytes.
// *	byte_n	   number of bytes
// *	crc	  initial value of crc
// *	buffer	   first address of buffer
static unsigned short calc_crc(unsigned long byte_n, unsigned short crc, char *buffer)
{
	int j;

	while (byte_n--) {
		crc = (((crc >> 8) ^ *buffer++) << 8) | (0xFF & crc) ;
		for(j = 0; j < 8; j++) {
			if (crc & 0x8000)  {
				crc <<= 1;
				crc ^= 0x1021;
			} else {
				crc <<= 1;
			}
		}
	}
	return(crc);
}

// * Provides 2 write cycles in order to "unlock" the flash
static void unlock_sequence(volatile unsigned char *pflash)
{
	*(pflash + CFG_ADDR) = UNLOCK_VALUE_A;
	*(pflash + (CFG_ADDR >> 1)) = UNLOCK_VALUE_B;
}

// * data_polling
// * Provides the necessary command sequence demanded by the Flash (Am29f400AT)
// * in order to check if the command (program/erase) over an address given as
// * argument was completed.  Does the necessary switching on the window register
// * to guarantee that the interrupts (when enabled) will be pointing to I/O 
// * window space.
// * Returns -	PASS - completed with succes

static unsigned char data_polling(volatile unsigned char *verify_addr, unsigned char data)
{
	unsigned char aux;
	int	flash_retries;

	for (flash_retries=1; flash_retries < 36000000; flash_retries++) {
		if (((*verify_addr) & DQ7) == (data & DQ7)) {
			return(PASS);
		}
		aux = ((*(verify_addr) & DQ5) == DQ5);
		if (aux) {
			/* flash ready*/
			if ((*(verify_addr) & DQ7) == (data & DQ7)) {
				return(PASS);
			} else {
				return(FAIL);
			}
		}
		if (!(flash_retries & 3)) {
			//current->state = TASK_INTERRUPTIBLE;
			schedule();
		}
	}
	return(FAIL);
}

// * do_flash_erase_sector
// * Provides the necessary command sequence demanded by the Flash (?)
// * in order to erase a sector given as argument. Does the necessary switching
// * on the window register to guarantee that the interrupts (when enabled)
// * will be pointing to I/O window space.

static int do_flash_erase_sector(struct CycFlashPartition *fp, int sector)
{
	volatile unsigned char 	*pflash;
	volatile unsigned char 	*sector_addr;
	int	sub_sector;
	struct CycFlashCtr *cf;

	cf = &FlashControl;

	for (sub_sector=0; sub_sector < 1; sub_sector++) {
		/* spots the correct FLASH chip (1st, 2nd or 3rd) */
		pflash = FlashChipAddr(sector);
		sector_addr = flash_addr + sector * cf->flash_sector_size;
		*(pflash + 0x0000) = RESET_READ_VALUE;
		unlock_sequence(pflash);
		*(pflash + CFG_ADDR) = SETUP_VALUE;	/* setup command */
		unlock_sequence(pflash);
		*(sector_addr) = SECTOR_ERASE_VALUE;

		/* waits for the sector erased confirmation */
		if (data_polling(sector_addr, DQ7) != PASS) {
			return(-EIO);
		}
		*(pflash + 0x0000) = RESET_READ_VALUE;
	}
	return(0);
}

// * returns the size of the file in the partition
static int do_flash_file_size(struct CycFlashPartition *fp)
{
	unsigned char *p; 
	int i, j;
	struct CycFlashCtr *cf;

	cf = &FlashControl;

	if (fp->crc_flag & CRC_FLAG_Shared) {
		return((fp->sector_end - fp->sector_start) * cf->flash_sector_size);
        }

	if (fp->crc_flag & CRC_FLAG_DoSize) {
		i = ((fp->sector_end - fp->sector_start) * cf->flash_sector_size) - 
			fp->offset_start - fp->offset_end;
		p = flash_addr + (fp->sector_end * cf->flash_sector_size) - fp->offset_end;
		while ((i -= 4) >= 0 && *(int *)((char *)p -= 4) == -1);

		for (j=0; j < 4 && *p != 0xff; j++, p++, i++);

		if (i) --i;

		if (i >= 2 && fp->crc_flag & CRC_FLAG_DoCRC) i -= 2;

		if (i < 0) {
			return(((fp->sector_end - fp->sector_start) * cf->flash_sector_size) -
				fp->offset_start - fp->offset_end);
		}
		return(i);
	}
	return(0);
}

// * erase the sector and copy the buffer from user to flash
static int do_flash_write(struct CycFlashPartition *fp)
{
	volatile unsigned char	*pflash, *pchip;
	char *p , *buf;
	unsigned int i,sector;
	struct CycFlashCtr *cf;

	cf = & FlashControl;
	p = buf = fp->buf;
	sector = fp->current_sector;

	if (do_flash_erase_sector(fp,sector)) {
		return(-EIO);
	}

	/* spots the correct FLASH chip (1st, 2nd or 3rd) */
	pchip = FlashChipAddr(sector);
	pflash = flash_addr + sector * cf->flash_sector_size;

	for (i = 0; i < cf->flash_sector_size; i++, buf++, pflash++) {
		/* write one byte */
		unlock_sequence(pchip);
		*(pchip + CFG_ADDR) = PROGRAM_VALUE; /* setup command */
		*pflash = *buf;

		/* waits until program sequence completion */
		if  (data_polling(pflash, *buf) != PASS) {
			/* something went wrong : return with fail status */
			return(-EIO);
		}
	}
	if (memcmp(flash_addr+sector*cf->flash_sector_size, p, cf->flash_sector_size)) {
		return(-EIO);
	}

	return (0);
}

// Compact Flash Operations

#include "cycCF.c"

static int lba_sector_sizes = 0;

static int do_cflash_read(struct CycFlashPartition *fp, int sector)
{
	struct CycFlashCtr *cf;
	cf = &CF_Control;

	if (!fp->buf) {
		if ((fp->buf = kmalloc(cf->flash_sector_size, GFP_KERNEL)) == NULL) {
			return(-ENOMEM);
		}
		fp->current_sector = -1;
	}

	//[RK]Aug/22/05 - changed the sector_size from 512 to 64KB
	// if (!cyc_cf_read (fp->buf, sector, cf->flash_sector_size)) 
	if (!cyc_cf_read (fp->buf, sector * 128, cf->flash_sector_size)) {
		return(-EIO);
	} 
	fp->current_sector = sector;
	return(0);
}

static int do_cflash_file_size(struct CycFlashPartition *fp, int wr, int val)
{
	int max, size, ix_word_size;
	int buf[256];
	struct CycFlashCtr *cf;

	cf = &CF_Control;

	if (!(fp->crc_flag & CRC_FLAG_DoSize)) {
		return(0);
	}

	if (!cyc_cf_read((char *)buf, lba_sector_sizes, 1024)) {
		return(0);
	}

	ix_word_size = (fp->sector_start == 0) ? 0 : 1;

	if (wr) {
		buf[ix_word_size] = val;
		cyc_cf_write((char *)buf, lba_sector_sizes, 1024);
	}

	size = buf[ix_word_size];
	max = (fp->sector_end - fp->sector_start) * cf->flash_sector_size;

	if (size < 0 || size > max) size = max;

	return(size);
}

static int do_cflash_write(struct CycFlashPartition *fp)
{
	struct CycFlashCtr *cf;
	cf = & CF_Control;

	//[RK]Aug/22/05 - changed the sector_size from 512 to 64KB
	//  if (!cyc_cf_write (fp->buf, fp->current_sector , cf->flash_sector_size)) 
	if (!cyc_cf_write (fp->buf, fp->current_sector * 128 , cf->flash_sector_size)) {
		return(-EIO);
	} 
	return(0);
}

// * 
static int cycfs_release_file(struct inode * inode, struct file * filp)
{
	struct CycFlashPartition *fp;
	unsigned char	buf[4];
	int i, i_max;
	struct CycFlashCtr *cf;

	fp = &cyc_flash[inode->i_ino -1] ;

	if (!fp->flags) {
		return(-EIO);
	}

	if (fp->flash_type == FLASH_TYPE_RAM) {
		cf = &FlashControl;
	} else {
		cf = &CF_Control;
	}

	if (fp->flags & FLAGS_CHANGED) {

		while (test_and_set_bit(0, (void *)&flash_locked)) {
			schedule();
		}

		fp->size = filp->f_pos;
		inode->i_size = fp->size;
		if (fp->crc_flag & CRC_FLAG_DoCRC) {
			buf[0] = fp->crc_calc / 256;
			buf[1] = fp->crc_calc % 256;
			buf[2] = 0x7e;
			fill_sector_flash(
				fp->sector_start + FLASH_DIV(filp->f_pos+fp->offset_start) ,
				FLASH_MOD(filp->f_pos+fp->offset_start), buf, 2, fp, 1);
			filp->f_pos += 2;
		}
		if (fp->crc_flag & CRC_FLAG_DoSize) {
			buf[0] = 0x7e;
			fill_sector_flash(
				fp->sector_start + FLASH_DIV(filp->f_pos+fp->offset_start),
				FLASH_MOD(filp->f_pos+fp->offset_start) , buf, 1, fp, 1);
			filp->f_pos++;
			i = FLASH_MOD(filp->f_pos+fp->offset_start);
			i_max = (fp->current_sector+1 != fp->sector_end) ?
				cf->flash_sector_size :
				cf->flash_sector_size - fp->offset_end;

			for (; i < i_max; ) {
				fp->buf[i++] = 0xff;
			}

			if (fp->flash_type == FLASH_TYPE_RAM) {
				do_flash_write(fp);
				for (i=fp->current_sector+1; i < fp->sector_end; i++) {
					do_flash_erase_sector(fp, i);
				}
			} else { // CF 
				do_cflash_write(fp);
				do_cflash_file_size(fp,1,fp->size);
			}
		} else {
			if (fp->flash_type == FLASH_TYPE_RAM) {
				do_flash_write(fp);
			} else {
				do_cflash_write(fp);
			}
		}
	}
	
	unlock_shared_sector(fp->sector_start, fp->sector_end);

	if (fp->buf) {
		kfree(fp->buf);
	}

	fp->flags = 0;
	fp->buf = NULL;
	clear_bit(0, (void *)&flash_locked); 
	return 0;
}

static int fill_sector_flash(int sector, int pos, char *buf, int count, struct CycFlashPartition *fp, int flag)
{
	int result;
	struct CycFlashCtr *cf;

	if (fp->flash_type == FLASH_TYPE_RAM) {
		cf = &FlashControl;
	} else {
		cf = &CF_Control;
	}

	if (fp->buf && (sector != fp->current_sector) && (fp->flags & FLAGS_CHANGED)) {
		fp->flags &= ~FLAGS_CHANGED; 
		if (fp->flash_type == FLASH_TYPE_RAM) {
			if ((result = do_flash_write(fp))) {
				return(result);
			}
		} else {
			if ((result = do_cflash_write(fp))) {
				return(result);
			}
		}
	}

	if (!fp->buf) {
		if ((fp->buf = kmalloc(cf->flash_sector_size, GFP_KERNEL)) == NULL) {
			return(-ENOMEM);
		}
		fp->current_sector = -1;
	}
	if (sector != fp->current_sector) {
		if (check_shared_sector(fp, sector, sector)) {
			if (fp->flash_type == FLASH_TYPE_CF) {
				if ((result = do_cflash_read(fp, sector))) { //????? CF
					return(result);
				}
			} else {
				memcpy(fp->buf, flash_addr + sector * cf->flash_sector_size, cf->flash_sector_size);
			}
		}
	}

	if (count + pos > cf->flash_sector_size) {
		count = cf->flash_sector_size - pos;
	}

	if (flag) {
		memcpy(fp->buf+pos, buf, count);
	} else {
		copy_from_user(fp->buf+pos, buf, count);
	}

	fp->current_sector = sector;
	fp->flags |= FLAGS_CHANGED ;
	return(count);
}

// * Open a file.  All we have to do here is to copy over a
// * copy of the counter pointer so it's easier to get at.
static int cycfs_open(struct inode *inode, struct file *filp)
{
	struct CycFlashPartition *fp;

	fp = &cyc_flash[inode->i_ino -1] ;

	if (fp->flags) {
		return(-EIO);
	}

	if ((filp->f_mode & FMODE_WRITE) &&
	    (fp->crc_flag & CRC_FLAG_Protected) &&
	    (flag_single_mode == 0)) {
		return(-EIO);
	}

	if (check_shared_sector(fp, fp->sector_start, fp->sector_end)) {
		if (lock_shared_sector(fp->sector_start, fp->sector_end)) {
			return(-EIO);
		}
	}

	// the inode defines the index in the cyclades partition table
	filp->private_data = (void *) fp;
	fp->flags = FLAGS_OPENNED;
	fp->buf = NULL;
	fp->crc_calc = 0;
	
	return 0;
}

// * Read a file.  Here we increment and read the counter, then pass it
// * back to the caller.  The increment only happens if the read is done
// * at the beginning of the file (offset = 0); otherwise we end up counting
// * by twos.
static ssize_t cycfs_read_file(struct file *filp, char *buf,
	size_t count, loff_t *ppos)
{
	struct CycFlashPartition *fp = (struct CycFlashPartition *)filp->private_data;
	unsigned char *p;
	int offset, max_ppos, block, ix_block, count_aux, ret;
	int save_count = 0, read_count = 0;
	struct CycFlashCtr *cf;
	
	if (fp->flash_type == FLASH_TYPE_RAM) {
		cf = &FlashControl;
	} else {
		cf = &CF_Control;
	}

	offset = (fp->sector_start * cf->flash_sector_size) + fp->offset_start;
	if (fp->crc_flag & CRC_FLAG_DoSize) {
		max_ppos = fp->size;
	} else {
		max_ppos = (fp->sector_end * cf->flash_sector_size) - fp->offset_end - offset;
	}

	if ((max_ppos -= *ppos) <= 0) {
		return(0);
	}

	if (count > max_ppos) {
		count = max_ppos;
	}

get_from_mem:
	block = FLASH_DIV(offset + *ppos);

	if (fp->flash_type == FLASH_TYPE_CF) {
		if (!fp->buf || (block != fp->current_sector)) {
			if ((ret = do_cflash_read(fp,block))) {
				return(ret);
			}
		}
	}

	if (fp->buf && block == fp->current_sector) {
		ix_block =  FLASH_MOD(*ppos);
		count_aux = cf->flash_sector_size - ix_block;
		if (count_aux > count) {
			count_aux = count;
		}
		copy_to_user(buf, fp->buf+ix_block, count_aux);
		read_count += count_aux;
		*ppos += count_aux;
		if ((count -= count_aux) == 0) {
			return(read_count);
		}
		buf += count_aux;
	}

	if (fp->flash_type == FLASH_TYPE_CF) {
		goto get_from_mem;
	}

	if (fp->buf &&
		block < fp->current_sector &&
		fp->current_sector <= FLASH_DIV(offset + *ppos + count)) {
		save_count = count;
		count = cf->flash_sector_size * fp->current_sector - *ppos;
		save_count -= count;
	}

	p = flash_addr + *ppos + offset;
	copy_to_user(buf, p, count);
	*ppos += count;
	read_count += count;

	if (save_count) {
		count = save_count;
		save_count = 0;
		goto get_from_mem;
	}

	return(read_count);
}

// * Write a file.
static ssize_t cycfs_write_file(struct file *filp, const char *buf,
		size_t count, loff_t *ppos)
{
	struct CycFlashPartition *fp = (struct CycFlashPartition *)filp->private_data;
	char * p = buf;
	int sector, max_ppos, count_aux;
	struct CycFlashCtr *cf;

	if (fp->flash_type == FLASH_TYPE_RAM) {
		cf = &FlashControl;
	} else {
		cf = &CF_Control;
	}

	if ((fp->crc_flag & CRC_FLAG_Protected) && (flag_single_mode == 0)) {
		return(-EIO);
	}

	max_ppos = (fp->sector_end - fp->sector_start) * cf->flash_sector_size -
			fp->offset_start - fp->offset_end;
	
	if ((max_ppos -= *ppos) <= 0) {
		return(-EINVAL);
	}

	if (count > max_ppos) {
		count = max_ppos;
	}

	max_ppos = 0;

	while (test_and_set_bit(0, (void *)&flash_locked)) {
		schedule();
	}

	while (count > 0) {
		sector = fp->sector_start + FLASH_DIV(*ppos+fp->offset_start);
		if ((count_aux = fill_sector_flash(sector, FLASH_MOD(*ppos+fp->offset_start), p, count, fp, 0)) <= 0) {
			clear_bit(0, (void *)&flash_locked); 
			return(count_aux);
		}
		if (fp->crc_flag & CRC_FLAG_DoCRC) {
			fp->crc_calc = calc_crc(count_aux, fp->crc_calc, p);
		}
		count -= count_aux;
		*ppos += count_aux;
		p += count_aux;
		max_ppos += count_aux;
	}

	clear_bit(0, (void *)&flash_locked); 
	return(max_ppos);
}

// * fill the size of file
int cycfs_simple_fill_super(struct super_block *s, int magic, struct tree_descr *files)
{
	static struct super_operations s_ops = {.statfs = simple_statfs};
	struct inode *inode;
	struct dentry *root;
	struct dentry *dentry;
	int i;

	s->s_blocksize = PAGE_CACHE_SIZE;
	s->s_blocksize_bits = PAGE_CACHE_SHIFT;
	s->s_magic = magic;
	s->s_op = &s_ops;

	inode = new_inode(s);
	if (!inode)
		return -ENOMEM;
	inode->i_mode = S_IFDIR | 0755;
	inode->i_uid = inode->i_gid = 0;
	inode->i_blkbits = 10;
	inode->i_blocks = 0;
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop = &simple_dir_operations;
	root = d_alloc_root(inode);
	if (!root) {
		iput(inode);
		return -ENOMEM;
	}
	for (i = 0; !files->name || files->name[0]; i++, files++) {
		struct qstr name;
		if (!files->name)
			continue;
		name.name = files->name;
		name.len = strlen(name.name);
		name.hash = full_name_hash(name.name, name.len);
		dentry = d_alloc(root, &name);
		if (!dentry)
			goto out;
		inode = new_inode(s);
		if (!inode)
			goto out;
		inode->i_mode = S_IFREG | files->mode;
		inode->i_uid = inode->i_gid = 0;
		inode->i_blkbits = 10;
		inode->i_blocks = 0;
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
		inode->i_fop = files->ops;
		inode->i_ino = i;
		if (cyc_flash[i-1].flash_type == FLASH_TYPE_RAM) {
			inode->i_size = do_flash_file_size(&cyc_flash[i-1]);
		} else {
			inode->i_size = do_cflash_file_size(&cyc_flash[i-1],0,0);
		}
		cyc_flash[i-1].size = inode->i_size;
		d_add(dentry, inode);
	}
	s->s_root = root;
	return 0;
out:
	d_genocide(root);
	dput(root);
	return -ENOMEM;
}

// * "Fill" a superblock with the files .
static int cycfs_fill_super (struct super_block *sb, void *data, int silent)
{
	return cycfs_simple_fill_super(sb, CYCFS_MAGIC, ACSFiles);
}

// * Stuff to pass in when registering the filesystem.
static int cycfs_get_super(struct file_system_type *fst, int flags,
			   const char *devname, void *data, struct vfsmount *mnt)
{
	return get_sb_single(fst, flags, data, cycfs_fill_super, mnt);
}

static struct file_system_type cycfs_type = {
	.owner 		= THIS_MODULE,
	.name		= "cycfs",
	.get_sb		= cycfs_get_super,
	.kill_sb	= kill_litter_super,
	.next		= NULL,
};

static int SetFlashSectors(int index, int st_off, int size,
        int flags, struct CycFlashCtr *cf)
{
	int ini_sector,i;
	int end_sector;
	struct CycFlashPartition *pf;

	pf = &cyc_flash[index];

	ini_sector = FLASH_DIV(st_off);
	end_sector = FLASH_DIV(st_off + size + cf->flash_sector_size - 1);

	if ((ini_sector < 0 || ini_sector >= cf->FlashNblkTotal) ||
	    (end_sector < 0 || end_sector >  cf->FlashNblkTotal)) {
		printk("Flash partition invalid sector range %d - %d total %d\n",
			ini_sector, end_sector, cf->FlashNblkTotal);
		return(-1);
	}

	pf->crc_flag = flags;
	pf->sector_start = ini_sector;
	pf->sector_end   = end_sector;
	pf->offset_start = FLASH_MOD(st_off);
	pf->offset_end = FLASH_MOD(st_off + size) ?
			 cf->flash_sector_size - FLASH_MOD(st_off + size) : 0;
	for (i=0; i < index ; i++) {
		if (cyc_flash[i].sector_end == (ini_sector+1)) {
			add_shared_sector(ini_sector);
		}
	}

	return(end_sector);
}

#define K(n) (1024*(n))
#define HK(n) (512*(n))

// * Get things set up.
static int __init cycfs_init(void)
{
	int length = 0; // CF length
        bd_t    *bd = (bd_t *)__res;

	if (cyc_cf_detect()) {  // get information about CF and fill CF_Control
		length = max_lba_addr * 256 * 2;
	}

#ifndef CONFIG_UBOOT
	FlashControl.flash_length = bd->hw_info.flash_size;
#else
	FlashControl.flash_length = bd->bi_flashsize;
#endif

	switch (bd->hw_info.board_type) {
		case BOARD_ONS441:
		case BOARD_ONS481:
		case BOARD_ONS442:
		case BOARD_ONS482:
		case BOARD_ONS841:
		case BOARD_ONS881:
		case BOARD_ONS842:
		case BOARD_ONS882:
		case BOARD_KVMP16:
			FlashControl.flash_shift = 12;  // div by shift right
			FlashControl.flash_sector_size = K(4);  // blocks of 4k
			FlashControl.FlashNblkChip = 64;        // number of 4kblock per chip
			FlashControl.FlashNblkTotal = FlashControl.flash_length / FlashControl.flash_sector_size;
			SetFlashSectors(FLASH_BOOT_ORI,K(0),
					K(252),CRC_FLAG_Protected,&FlashControl);
			SetFlashSectors(FLASH_BOOT_CONF,K(252),
					K(4),0,&FlashControl);
			break;
#ifdef CONFIG_TS1H
		case BOARD_TS1H:
			//???? FlashControl.FlashWrite  = do_cfiflash_write;
			//???? FlashControl.EraseSector = do_cfiflash_erase_sector;
			FlashControl.FlashNblkChip = 32;                // number of 128k block per chip
			FlashControl.flash_sector_size = K(128);        // blocks of 128k
			FlashControl.flash_shift = 17;  // div by shift right
			FlashControl.FlashNblkTotal = FlashControl.flash_length / FlashControl.flash_sector_size;
			SetFlashSectors(FLASH_BOOT_ORI, K(0), 
				K(127),CRC_FLAG_Protected|CRC_FLAG_DoSize,&FlashControl);
			SetFlashSectors(FLASH_BOOT_INFO,K(127),
				K(1),0,&FlashControl);
			SetFlashSectors(FLASH_BOOT_ALT,K(128),
				K(64),CRC_FLAG_DoSize|CRC_FLAG_DoCRC,&FlashControl);
			SetFlashSectors(FLASH_BOOT_CONF,K(192),
				HK(1),CRC_FLAG_DoSize,&FlashControl);
			SetFlashSectors(FLASH_CONFIG,K(192)+HK(1),
				K(191)+HK(1),CRC_FLAG_DoSize,&FlashControl);
			SetFlashSectors(FLASH_zIMAGE,K(384),
				FlashControl.flash_length-K(384),CRC_FLAG_DoSize|CRC_FLAG_DoCRC,&FlashControl);
			break;
#endif
		default : //ACS
			//[RK]Aug/22/05 - changed the sector_size from 512 to 64KB
			//CF_Control.flash_shift = 9;     // div by shift right
			//CF_Control.flash_sector_size = 512; // blocks of 0.5k
			CF_Control.flash_shift = 16;     // div by shift right
			CF_Control.flash_sector_size = K(64); // blocks of 64k
			CF_Control.FlashNblkTotal = length / CF_Control.flash_sector_size;
			CF_Control.FlashNblkChip  = CF_Control.FlashNblkTotal;
			FlashControl.flash_sector_size = K(4);  // blocks of 4k
			FlashControl.flash_shift = 12;  // div by shift right
			FlashControl.FlashNblkChip = 64;    // number of 4kblock per chip
			FlashControl.FlashNblkTotal = FlashControl.flash_length / FlashControl.flash_sector_size;
			SetFlashSectors(FLASH_BOOT_ORI, K(0), 
					K(127),CRC_FLAG_Protected|CRC_FLAG_DoSize,&FlashControl);
			SetFlashSectors(FLASH_BOOT_INFO,K(127),
					K(1),0,&FlashControl);
			SetFlashSectors(FLASH_BOOT_ALT,K(128),
					K(64),CRC_FLAG_DoSize|CRC_FLAG_DoCRC,&FlashControl);
			SetFlashSectors(FLASH_BOOT_CONF,K(192),
					K(64),CRC_FLAG_DoSize,&FlashControl);
#if !(defined(CONFIG_TS1H) || defined(CONFIG_UBOOT))
			//[RK]Aug/22/05 - changed the sector_size from 512 to 64KB
			lba_sector_sizes = SetFlashSectors(FLASH_zIMAGE, 
						HK(0),HK((CF_Control.FlashNblkTotal-9)*128),
						CRC_FLAG_DoSize|CRC_FLAG_DoCRC,&CF_Control);
			lba_sector_sizes *= 128;
			SetFlashSectors(FLASH_CONFIG,
					HK((CF_Control.FlashNblkTotal-8)*128),
					HK(8*128),CRC_FLAG_DoSize,&CF_Control);
#endif

			break;
	}

	return register_filesystem(&cycfs_type);
}

static void __exit cycfs_exit(void)
{
	unregister_filesystem(&cycfs_type);
}

module_init(cycfs_init);
module_exit(cycfs_exit);
