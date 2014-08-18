#include <linux/semaphore.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/wait.h>		/* wait_queue_head_t */
#include <linux/interrupt.h>
#include <linux/version.h>

#include <linux/types.h>
#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/inet.h>
#include <linux/errno.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <net/wext.h>
#include <linux/if_packet.h>

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/spinlock.h>

#include "get_pte.c"


//#define	DEBUG

#ifndef	DRV_NAME
#define	DRV_NAME	"genpipe"
#endif
#ifndef	DRV_IDX
#define	DRV_IDX		(0)
#endif
#ifndef	IF_NAME
#define	IF_NAME		"eth0"
#endif
#define	DRV_VERSION	"0.0.2"
#define	genpipe_DRIVER_NAME	DRV_NAME " Generic Etherpipe driver " DRV_VERSION

#ifndef	PACKET_BUF_MAX
#define	PACKET_BUF_MAX	(1024*1024)
#endif
#ifndef	MTU
#define	MTU		(9014)
#endif

// macchan (see /proc/kallsyms or /boot/System.map-$(uname -r)
//struct list_head ptype_all __read_mostly;
//struct list_head ptype_base[PTYPE_HASH_SIZE] __read_mostly;
struct list_head *ptypeall   = 0LL;
struct list_head *ptypebase[PTYPE_HASH_SIZE];

int (*netifreceiveskb)(struct sk_buff *);
int (*ixgbecleanrxirq)(struct ixgbe_q_vector *, struct ixgbe_ring *, const int );
int (*arprcv)(struct sk_buff *, struct net_device *, struct packet_type *, struct net_device *);
int (*iprcv)(struct sk_buff *, struct net_device *, struct packet_type *, struct net_device *);
int (*ipv6rcv)(struct sk_buff *, struct net_device *, struct packet_type *, struct net_device *);
int (*backup_func)(struct sk_buff *, struct net_device *, struct packet_type *, struct net_device *);

static char *interface = IF_NAME;
module_param( interface , charp , S_IRUGO);
MODULE_PARM_DESC( interface, "interface" );

#define	INFO_SKB(X) \
printk( "len=%u,", X->len); \
printk( "data_len=%u,", X->data_len); \
printk( "mac_header=%x,", (unsigned int)X->mac_header); \
printk( "network_header=%x,", (unsigned int)X->network_header); \
printk( "transport_header=%x,", (unsigned int)X->transport_header); \
printk( "*head=%p,", X->head); \
printk( "*data=%p,", X->data); \
printk( "tail=%x,", (unsigned int)X->tail); \
printk( "end=%x\n", (unsigned int)X->end);

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)
#define	__devinit
#define	__devexit
#define	__devexit_p
#endif

static spinlock_t rx_lock;
static wait_queue_head_t write_q;
static wait_queue_head_t read_q;

/* receive and transmitte buffer */
struct _pbuf_dma {
	unsigned char   *rx_start_ptr;		/* rx buf start */
	unsigned char   *rx_end_ptr;		/* rx buf end */
	unsigned char   *rx_write_ptr;		/* rx write ptr */
	unsigned char   *rx_read_ptr;		/* rx read ptr */
	unsigned char   *tx_start_ptr;		/* tx buf start */
	unsigned char   *tx_end_ptr;		/* tx buf end */
	unsigned char   *tx_write_ptr;		/* tx write ptr */
	unsigned char   *tx_read_ptr;		/* tx read ptr */
} static pbuf0={0,0,0,0,0,0,0,0};

unsigned long rx_count[NR_CPUS];
struct net_device* device = NULL; 

int genpipe_pack_rcv(struct sk_buff *, struct net_device *, struct packet_type *, struct net_device *);

static struct packet_type genpipe_pack =
{
	__constant_htons(ETH_P_ALL),
	NULL,
	genpipe_pack_rcv,

	(void *) 1,
	NULL
};

struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

void file_close(struct file* file) {
	filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
} 

int get_table_entry(void)
{
	struct file *fp;
	int pos, size, i;
	char buf[128], *ptr;

	netifreceiveskb = ixgbecleanrxirq = arprcv = iprcv = ipv6rcv = ptypeall = 0LL;
	for (i=0;i<PTYPE_HASH_SIZE;++i)
		ptypebase[i] = 0LL;

	fp = file_open("/proc/kallsyms", O_RDONLY, 0);
	if (fp) {
		printk( "open /proc/kalsyms\n");
		pos=0;
		while ( (size = file_read(fp, pos, buf+(sizeof(buf)/2), sizeof(buf)/2)) > 0) {
			pos += size;
			ptr = strnstr(buf, " netif_receive_skb\n", sizeof(buf));
			if (ptr && !netifreceiveskb)
				sscanf(ptr-19, "%llx", &netifreceiveskb);
			ptr = strnstr(buf, "ixgbe_clean_rx_irq", sizeof(buf));
			if (ptr && !ixgbecleanrxirq)
				sscanf(ptr-19, "%llx", &ixgbecleanrxirq);
			ptr = strnstr(buf, " arp_rcv\n", sizeof(buf));
			if (ptr && !arprcv)
				sscanf(ptr-19, "%llx", &arprcv);
			ptr = strnstr(buf, " ip_rcv\n", sizeof(buf));
			if (ptr && !iprcv)
				sscanf(ptr-19, "%llx", &iprcv);
			ptr = strnstr(buf, " ipv6_rcv\n", sizeof(buf));
			if (ptr && !ipv6rcv)
				sscanf(ptr-19, "%llx", &ipv6rcv);
			ptr = strnstr(buf, " ptype_all\n", sizeof(buf));
			if (ptr && !ptypeall)
				sscanf(ptr-19, "%llx", &ptypeall);
			ptr = strnstr(buf, " ptype_base\n", sizeof(buf));
			if (ptr && !ptypebase[0]) {
				sscanf(ptr-19, "%llx", &ptypebase[0]);
				for (i=1;i<PTYPE_HASH_SIZE;++i)
					ptypebase[i] = (unsigned long long)ptypebase[0] + i*0x10;
			}
			memcpy(&buf[0], &buf[sizeof(buf)/2], sizeof(buf)/2);
			if (netifreceiveskb != 0LL && ixgbecleanrxirq != 0LL &&  arprcv != 0LL && iprcv != 0LL && ipv6rcv != 0LL && ptypeall != 0LL && ptypebase[0] != 0LL)
				break;
		}
		printk("netifreceiveskb=%p\n", netifreceiveskb);
		printk("ixgbecleanrxirq=%p\n", ixgbecleanrxirq);
		printk("arprcv=%p\n", arprcv);
		printk("iprcv=%p\n", iprcv);
		printk("ipv6rcv=%p\n", ipv6rcv);
		printk("ptype_all=%p\n", ptypeall);
		for (i=0; i<PTYPE_HASH_SIZE; ++i)
			printk("ptype_base[%d]=%p\n", i, ptypebase[i]);

		file_close(fp);
		return 0;
	}
	return -1;	
}


int genpipe_netif_receive_skb(struct sk_buff *skb)
{
		return netifreceiveskb(skb);
}

int hook_ixgbe(void)
{
	unsigned char *ptr;
	unsigned int dest;
	pte_t *pte;
	int i;

	if (netifreceiveskb == 0LL || ixgbecleanrxirq == 0LL)
		return -1;

	ptr = ixgbecleanrxirq;
	for (i=0; i<0x10000; ++ptr, ++i) {
		// check callq (0xe8 + 4byte)
		if (*ptr == 0xe8) {
			dest = *(unsigned int *)(ptr+1);
			if (((unsigned int)ptr + 5 + dest) == (unsigned int)netifreceiveskb) {
				printk( "%p: callq netif_receive_skb()\n", ptr);
				if ( (pte = get_pte((unsigned long long)ptr)) ) {
					pte->pte |= (_PAGE_RW);
					*(unsigned int *)(ptr+1) = (unsigned int)genpipe_netif_receive_skb - ((unsigned int)ptr + 5);
					pte->pte &= ~(_PAGE_RW);
				}
				return 0;
			}
		}
	}

	return -1;
}

int genpipe_arprcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *dev2)
{
//	printk("genpipe_arprcv dev=%p,dev2=%p,genpipe_pack.dev=%p\n", dev, dev2,genpipe_pack.dev);
	if (dev != genpipe_pack.dev)
		return arprcv(skb, dev, pt, dev2);

	/* Don't mangle buffer if shared */
	if (!(skb = skb_share_check(skb, GFP_ATOMIC)))
		return 0;

	kfree_skb(skb);
	return skb->len;
}

int genpipe_iprcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *dev2)
{
//	printk("genpipe_iprcv dev=%p,dev2=%p,genpipe_pack.dev=%p\n", dev, dev2,genpipe_pack.dev);
	if (unlikely(dev != genpipe_pack.dev))
		return iprcv(skb, dev, pt, dev2);

	if (unlikely(skb->pkt_type == PACKET_OUTGOING))	 // DROP loopback PACKET
		goto lend;

//	spin_lock(&rx_lock);
	++rx_count[smp_processor_id()];
//	spin_unlock(&rx_lock);

lend:
	/* Don't mangle buffer if shared */
	if (!(skb = skb_share_check(skb, GFP_ATOMIC)))
		return 0;

	kfree_skb(skb);
	return skb->len;
}

int genpipe_ipv6rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *dev2)
{
//	printk("genpipe_ipv6rcv dev=%p,dev2=%p,genpipe_pack.dev=%p\n", dev, dev2,genpipe_pack.dev);
	if (dev != genpipe_pack.dev)
		return ipv6rcv(skb, dev, pt, dev2);

	/* Don't mangle buffer if shared */
	if (!(skb = skb_share_check(skb, GFP_ATOMIC)))
		return 0;

	kfree_skb(skb);
	return skb->len;
}

int genpipe_nop(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *dev2)
{
//	printk("genpipe_nop dev=%p,dev2=%p,genpipe_pack.dev=%p\n", dev, dev2,genpipe_pack.dev);
//	if (dev != genpipe_pack.dev)
//		return nop(skb, dev, pt, dev2);

	/* Don't mangle buffer if shared */
	if (!(skb = skb_share_check(skb, GFP_ATOMIC)))
		return 0;

	kfree_skb(skb);
	return skb->len;
}

int genpipe_pack_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *dev2)
{
	int i, frame_len;
	unsigned char *p;

	if (skb->pkt_type == PACKET_OUTGOING)	 // DROP loopback PACKET
		goto lend;

#ifdef DEBUG
	printk(KERN_DEBUG "Test protocol: Packet Received with length: %u\n", skb->len+18);
#endif

//	spin_lock(&rx_lock);
	++rx_count[smp_processor_id()];
//	spin_unlock(&rx_lock);
//	frame_len = skb->len;
//	p = skb_mac_header(skb);
//	p = skb->data;

lend:
	/* Don't mangle buffer if shared */
	if (!(skb = skb_share_check(skb, GFP_ATOMIC)))
		return 0;

	kfree_skb(skb);
	return skb->len;
}

static int genpipe_open(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);
//	rtnl_lock();
//	dev_set_promiscuity(device, 1);
//	rtnl_unlock();

	return 0;
}

static ssize_t genpipe_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *ppos)
{
	char tmp[16];
	int copy_len, available_read_len, i, rxcounts;
#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	rxcounts=0;
	for (i=0;i<NR_CPUS;++i)
		rxcounts += rx_count[i];

	sprintf( tmp, "%11lu\r\n", rxcounts);
	if ( copy_to_user( buf, tmp, 13 ) ) {
		printk( KERN_INFO "copy_to_user failed\n" );
		return -EFAULT;
	}

	return 13;
}

static ssize_t genpipe_write(struct file *filp, const char __user *buf,
	size_t count, loff_t *ppos)

{
	int i, copy_len, pos, ret, frame_len;
	struct sk_buff *tx_skb;
	unsigned char *cr;
	static unsigned char tmp_pkt[MTU+14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	copy_len = 0;
	tx_skb = NULL;

	if ( (pbuf0.tx_write_ptr +  count) > pbuf0.tx_end_ptr ) {
		memcpy( pbuf0.tx_start_ptr, pbuf0.tx_read_ptr, (pbuf0.tx_write_ptr - pbuf0.tx_start_ptr ));
		pbuf0.tx_write_ptr -= (pbuf0.tx_write_ptr - pbuf0.tx_start_ptr );
		pbuf0.tx_read_ptr = pbuf0.tx_start_ptr;
		if ( pbuf0.tx_read_ptr < pbuf0.tx_start_ptr )
			pbuf0.tx_read_ptr = pbuf0.tx_start_ptr;
	}

	if ( count > (pbuf0.tx_end_ptr - pbuf0.tx_write_ptr))
		count = (pbuf0.tx_end_ptr - pbuf0.tx_write_ptr);

#ifdef DEBUG
	printk("%s count=%d\n", __func__, count);
#endif

	if ( copy_from_user( pbuf0.tx_write_ptr, buf, count ) ) {
		printk( KERN_INFO "copy_from_user failed\n" );
		return -EFAULT;
	}

	pbuf0.tx_write_ptr += count;
	copy_len = count;

genpipe_write_loop:
	for ( cr = pbuf0.tx_read_ptr; cr < pbuf0.tx_write_ptr && *cr != '\n'; ++cr );
	if ( cr == pbuf0.tx_write_ptr )	/* not found CR */
		goto genpipe_write_exit;

#ifdef DEBUG
	printk( "pbuf0.tx_read_ptr=%s\n", pbuf0.tx_read_ptr);
#endif
	frame_len = 0;
	pos = 0;

	for ( ; pbuf0.tx_read_ptr < cr && frame_len < MTU ; ++pbuf0.tx_read_ptr ) {
		// skip space
		if (*pbuf0.tx_read_ptr == ' ')
			continue;
		// conver to upper char
		if (*pbuf0.tx_read_ptr >= 'a' && *pbuf0.tx_read_ptr <= 'z')
			*pbuf0.tx_read_ptr -= 0x20;
		// is hexdigit?
		if (*pbuf0.tx_read_ptr >= '0' && *pbuf0.tx_read_ptr <= '9') {
			if ( pos == 0 ) {
				tmp_pkt[frame_len+14] = (*pbuf0.tx_read_ptr - '0') << 4;
				pos = 1;
			} else {
				tmp_pkt[frame_len+14] |= (*pbuf0.tx_read_ptr - '0');
				++frame_len;
				pos = 0;
			}
		} else if (*pbuf0.tx_read_ptr >= 'A' && *pbuf0.tx_read_ptr <= 'Z') {
			if ( pos == 0 ) {
				tmp_pkt[frame_len+14] = (*pbuf0.tx_read_ptr - 'A' + 10) << 4;
				pos = 1;
			} else {
				tmp_pkt[frame_len+14] |= (*pbuf0.tx_read_ptr - 'A' + 10);
				++frame_len;
				pos = 0;
			}
		}
	}

#ifdef DEBUG
printk( "%02x%02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x %02x%02x %02x %02x\n", 
	tmp_pkt[14], tmp_pkt[15], tmp_pkt[16], tmp_pkt[17], tmp_pkt[18], tmp_pkt[19],
	tmp_pkt[20], tmp_pkt[21], tmp_pkt[22], tmp_pkt[23], tmp_pkt[24], tmp_pkt[25],
	tmp_pkt[26], tmp_pkt[27],
	tmp_pkt[27], tmp_pkt[28]);
#endif

	tx_skb = netdev_alloc_skb_ip_align(device, frame_len+14);
	skb_reserve(tx_skb, 2);	/* align IP on 16B boundary */
	if (likely(tx_skb)) {
#ifdef DEBUG
INFO_SKB(tx_skb);
#endif

		skb_reset_mac_header(tx_skb);
		skb_reset_transport_header(tx_skb);
		skb_reset_network_header(tx_skb);
		memcpy(skb_put(tx_skb, frame_len+14), tmp_pkt, frame_len+14);
//		skb_set_mac_header(tx_skb,14);
//		skb_set_transport_header(tx_skb,20);
		skb_set_network_header(tx_skb,38);
#ifdef DEBUG
INFO_SKB(tx_skb);
#endif
		tx_skb->dev = device;
		tx_skb->protocol = eth_type_trans(tx_skb, device);
		ret = dev_queue_xmit(tx_skb);
		if (ret) {
			printk("fail to dev_queue_xmit=%d\n", ret);
		}
	}

	pbuf0.tx_read_ptr = cr + 1;

	i = (pbuf0.tx_read_ptr - pbuf0.tx_start_ptr );
	if (i > 0) {
		memcpy( pbuf0.tx_start_ptr, pbuf0.tx_read_ptr, ( pbuf0.tx_write_ptr - pbuf0.tx_read_ptr ) );
		pbuf0.tx_read_ptr -= i;
		pbuf0.tx_write_ptr -= i;
	}

	goto genpipe_write_loop;

genpipe_write_exit:

	return copy_len;
}

static int genpipe_release(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);
	rtnl_lock();
	dev_set_promiscuity(device, -1);
	rtnl_unlock();

	return 0;
}

static unsigned int genpipe_poll( struct file* filp, poll_table* wait )
{
	unsigned int retmask = 0;

#ifdef DEBUG
	printk("%s\n", __func__);
#endif

	poll_wait( filp, &read_q,  wait );
//	poll_wait( filp, &write_q, wait );

	if ( pbuf0.rx_read_ptr != pbuf0.rx_write_ptr ) {
		retmask |= ( POLLIN  | POLLRDNORM );
//		log_format( "POLLIN  | POLLRDNORM" );
	}
/* 
   読み込みデバイスが EOF の場合は retmask に POLLHUP を設定
   デバイスがエラー状態である場合は POLLERR を設定
   out-of-band データが読み出せる場合は POLLPRI を設定
 */

	return retmask;
}


static long genpipe_ioctl(struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	printk("%s\n", __func__);
	return  -ENOTTY;
}

static struct file_operations genpipe_fops = {
	.owner		= THIS_MODULE,
	.read		= genpipe_read,
	.write		= genpipe_write,
	.poll		= genpipe_poll,
	.compat_ioctl	= genpipe_ioctl,
	.open		= genpipe_open,
	.release	= genpipe_release,
};

static struct miscdevice genpipe_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DRV_NAME,
	.fops = &genpipe_fops,
};

static int __init genpipe_init(void)
{
	int ret, i;
	static char name[16];
	struct packet_type *ptype = NULL;


#ifdef MODULE
	pr_info(genpipe_DRIVER_NAME "\n");
#endif
	printk("%s\n", __func__);

	device = dev_get_by_name(&init_net, interface); 
	if ( !device ) {
		printk(KERN_WARNING "Could not find %s\n", interface);
		ret = -1;
		goto error;
	}

	/* Set receive buffer */
	if ( ( pbuf0.rx_start_ptr = kmalloc(PACKET_BUF_MAX, GFP_KERNEL) ) == 0 ) {
		printk("fail to kmalloc\n");
		ret = -1;
		goto error;
	}
	pbuf0.rx_end_ptr = (pbuf0.rx_start_ptr + PACKET_BUF_MAX - 1);
	pbuf0.rx_write_ptr = pbuf0.rx_start_ptr;
	pbuf0.rx_read_ptr  = pbuf0.rx_start_ptr;

	/* Set transmitte buffer */
	if ( ( pbuf0.tx_start_ptr = kmalloc(PACKET_BUF_MAX, GFP_KERNEL) ) == 0 ) {
		printk("fail to kmalloc\n");
		ret = -1;
		goto error;
	}
	pbuf0.tx_end_ptr = (pbuf0.tx_start_ptr + PACKET_BUF_MAX - 1);
	pbuf0.tx_write_ptr = pbuf0.tx_start_ptr;
	pbuf0.tx_read_ptr  = pbuf0.tx_start_ptr;
	
	if (get_table_entry() < 0) {
		printk("fail to open /peoc/ka\n");
		ret = -1;
		goto error;
	}

	if (hook_ixgbe() < 0) {
		printk("error in hook_ixgbe()\n");
		ret = -1;
		goto error;
	}

	/* register character device */
	sprintf( name, "%s/%d", DRV_NAME, DRV_IDX );
	genpipe_dev.name = name;
	ret = misc_register(&genpipe_dev);
	if (ret) {
		printk("fail to misc_register (MISC_DYNAMIC_MINOR)\n");
		goto error;
	}

	init_waitqueue_head( &read_q );
	init_waitqueue_head( &write_q );

//macchan
	printk("*device=%p\n", device);
	printk("*genpipe_arprcv=%p\n", genpipe_arprcv);
	printk("*genpipe_nop=%p\n", genpipe_nop);
	printk("*genpipe_pack_rcv=%p\n", genpipe_pack.func);

	// macchan
	rcu_read_lock();
	for (i=0;i<PTYPE_HASH_SIZE;++i) {
		printk("*ptype_base[%x]*\n", i);
		list_for_each_entry_rcu(ptype, ptypebase[i], list) {
			if (ptype->func == arprcv)
				ptype->func = genpipe_arprcv;
			else if (ptype->func == iprcv)
				ptype->func = genpipe_iprcv;
			else if (ptype->func == ipv6rcv)
				ptype->func = genpipe_ipv6rcv;
			printk("dev=%p,type=%04x, func=%p\n", ptype->dev, ntohs(ptype->type), ptype->func);
		}
	}
	printk("*ptype*\n");
	list_for_each_entry_rcu(ptype, ptypeall, list) {
		backup_func = ptype->func;
		ptype->func = genpipe_nop;
		printk("dev=%p,type=%04x, func=%p\n", ptype->dev, ntohs(ptype->type), ptype->func);
	}
	rcu_read_unlock();

	genpipe_pack.dev = device;
//	dev_add_pack(&genpipe_pack);

	for (i=0; i<NR_CPUS;++i)
		rx_count[i] = 0;

	return 0;

error:
	if ( pbuf0.rx_start_ptr ) {
		kfree( pbuf0.rx_start_ptr );
		pbuf0.rx_start_ptr = NULL;
	}

	if ( pbuf0.tx_start_ptr ) {
		kfree( pbuf0.tx_start_ptr );
		pbuf0.tx_start_ptr = NULL;
	}

	return ret;
}

static void __exit genpipe_cleanup(void)
{
	int i;
	struct packet_type *ptype = NULL;
	misc_deregister(&genpipe_dev);

//	dev_remove_pack(&genpipe_pack);

//macchan
	rcu_read_lock();
	for (i=0;i<PTYPE_HASH_SIZE;++i) {
		list_for_each_entry_rcu(ptype, ptypebase[i], list) {
			if (ptype->func == genpipe_arprcv)
				ptype->func = arprcv;
			else if (ptype->func == genpipe_iprcv)
				ptype->func = iprcv;
			else if (ptype->func == genpipe_ipv6rcv)
				ptype->func = ipv6rcv;
		}
	}
	rcu_read_unlock();
	rcu_read_lock();
	list_for_each_entry_rcu(ptype, ptypeall, list) {
		if (ptype->func == genpipe_nop);
			ptype->func = backup_func; 
	}
	rcu_read_unlock();

	if ( pbuf0.rx_start_ptr ) {
		kfree( pbuf0.rx_start_ptr );
		pbuf0.rx_start_ptr = NULL;
	}

	if ( pbuf0.tx_start_ptr ) {
		kfree( pbuf0.tx_start_ptr );
		pbuf0.tx_start_ptr = NULL;
	}

	printk("%s\n", __func__);
}

MODULE_LICENSE("GPL");
module_init(genpipe_init);
module_exit(genpipe_cleanup);

