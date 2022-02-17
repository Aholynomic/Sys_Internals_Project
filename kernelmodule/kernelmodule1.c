/*
 * This is a kernel module designed to be inserted into a Linux system
 * and hijack the system call table and replace with our own function
 * that monitors network system calls.
 *
 * 	kernelmodule1.c
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/string.h>
#include <linux/tty.h>
#include <linux/unistd.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/ip.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <asm/uaccess.h>

#define LOG_FILENAME "/test.log"
#define LOG_FILEPATH "/home/pi/Desktop/Assessment"

static unsigned long *sctable;

asmlinkage int (*getuid_call)(void);
asmlinkage int (*orig_getsockname)(int, struct sockaddr *, unsigned int);
asmlinkage int (*orig_getpeername)(int, struct sockaddr *, unsigned int);
asmlinkage int (*orig_connect)(int, const struct sockaddr *, unsigned int);

/**
 * implementing inet_ntoa(**args) manually because of kernel space
 */
static char *my_inet_ntoa(struct in_addr in) {
	unsigned char *bytes = (unsigned char *) &in;
	char buffer[18];
	snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d",
			bytes[0], bytes[1], bytes[2], bytes[3]);

	return buffer;
}

/**
 * function to write the network logs
 */
int write_logfile(char *buff) {

	char f_path[120];
	strcpy(f_path, LOG_FILEPATH);
	strcat(f_path, LOG_FILENAME);

	struct file *fp_w;
	loff_t pos;
	int ret, count;
	mm_segment_t fs;

	// either create or append to file
	fp_w = filp_open(f_path, O_RDWR|O_CREAT|O_APPEND, 0644);
	if (IS_ERR(fp_w)) {
		printk(KERN_INFO "failed to open %s\n", LOG_FILENAME);
		ret = -ENODEV;
		goto out1;
	}

	fs = get_fs();

	set_fs(KERNEL_DS);

	pos = 0;

	// write the buffer to file
	count = kernel_write(fp_w, buff, strlen(buff), &pos);
	if (count < 0) {
		printk(KERN_INFO "failed write: %d\n", count);
		goto out2;
	}

	pr_info("write %s to %s\n", buff, LOG_FILENAME);

	// close file
	filp_close(fp_w, NULL);

	set_fs(fs);

	out2:
		set_fs(fs);
		filp_close(fp_w, NULL);
	out1:
		filp_close(fp_w, NULL);
}

/**
 * this will be the modified connect() system call
 * note: also runs the original
 */
asmlinkage int new_connect(int fd, struct sockaddr __user *buff1, int flag) {

	int socklen, ret1, ret2, uid;
	int tmp1, tmp2, tmp3, time, sec, hr, min;
	struct sockaddr_in getsock, getpeer;
	struct sockaddr_in *getsock_p, *getpeer_p;
	char netinfo_buff[200], path[120], buff[100], uid_str[10], act_time[12];

	socklen = sizeof(getsock);

	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);

	// source IP
	ret1 = orig_getsockname(fd, (struct sockaddr *)&getsock, &socklen);
	if (ret1 == -1){
		printk(KERN_INFO "getsockname failed");
	}
	getsock_p = &getsock;

	// destination IP
	ret2 = orig_getpeername(fd, (struct sockaddr *)&getpeer, &socklen);
	if (ret2 == -1){
			printk(KERN_INFO "getpeername failed");
		}
	getpeer_p = &getpeer;

	set_fs(old_fs);

	printk(KERN_INFO "Time: %d", ktime_get_real());

	// get uid of process
	uid = getuid_call();
	snprintf(uid_str, sizeof(uid_str), "*%d", uid);
	strcpy(netinfo_buff, uid_str);

	printk(KERN_INFO "writing new entry\n");

	// write Connect indicator to buffer
	snprintf(buff, 9, "*%s", "Connect");
	strcat(netinfo_buff, buff);

	// write source IP to buffer
	snprintf(buff, 18, "*%s", my_inet_ntoa(getsock.sin_addr));
	strcat(netinfo_buff, buff);

	// write source port to buffer
	snprintf(buff, 10, "*%d", getsock.sin_port);
	strcat(netinfo_buff, buff);

	// write destination address to buffer
	snprintf(buff, 18, "*%s", my_inet_ntoa(getpeer.sin_addr));
	strcat(netinfo_buff, buff);

	// write destination port to buffer
	snprintf(buff, 10, "*%d\n", getpeer.sin_port);
	strcat(netinfo_buff, buff);

	// write the contents of the buffer to the given path file
	write_logfile(netinfo_buff);

	// run original system call
	return orig_connect(fd, buff1,flag);
}

/**
 * overwrites the system call table numbers with our spy module/s
 */
static int __init module1_init(void) {
	// informational - see dmesg
	printk(KERN_INFO "entry of spy module\n");

	// get the base address of the system call table
    printk(KERN_INFO "grabbing system call table address\n");
	sctable = kallsyms_lookup_name("sys_call_table");

	printk(KERN_EMERG "Unload the module and Results will be Written to File");

	// print to dmesg
	printk(KERN_EMERG "Address of sys_call_table : %lx\n", sctable);

	// store the original addresses of the system calls - for safety and
	// invoking them with new functions
	printk(KERN_INFO "storing original addresses of system calls\n");
	orig_getsockname = (void *)sctable[__NR_getsockname];
	orig_getpeername = (void *)sctable[__NR_getpeername];
	orig_connect = (void *)sctable[__NR_connect];
	getuid_call = (void *)sctable[__NR_getuid];

	// overwrite the addresses of the system calls we plan to modify
	printk(KERN_INFO "overwriting addresses\n");
	sctable[__NR_connect] = new_connect;

	// see dmesg
	printk(KERN_EMERG "Connect Old Address: %x\n", orig_connect);
	printk(KERN_EMERG "Connect New Address: %x\n", new_connect);
	printk(KERN_EMERG "Unchanged getsockname() Address: %x\n", orig_getsockname);
	printk(KERN_EMERG "Unchanged getpeername() Address: %x\n", orig_getpeername);

	return 0;
}

/**
 * set the system call table numbers back to original state
 */
static void __exit module1_exit(void){
	// informational - see dmesg
	printk(KERN_INFO "exit of spy module\n");

	// modify the system call table to original addresses that
	// we took in module1_init()
	printk(KERN_EMERG "Modifying System Call Table To Original Addresses\n");
	sctable[__NR_getsockname] = orig_getsockname;
	sctable[__NR_getpeername] = orig_getpeername;
	sctable[__NR_connect] = orig_connect;
	sctable[__NR_getuid] = getuid_call;

	printk(KERN_EMERG "Results written to %s%s.\n", LOG_FILEPATH, LOG_FILENAME);
}

///// Info ///////////////////////////////////
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ethan Hastie, 1801853");
MODULE_DESCRIPTION("Spy System Call");
MODULE_VERSION ("0.1");
module_init(module1_init);
module_exit(module1_exit);
/////////////////////////////////////////////
