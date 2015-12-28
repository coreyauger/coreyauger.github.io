/*
 *  module_kill.c
 *  
 *  This module allows a common user to create a 
 *  device with Major number 255 and then any user 
 *  that knows it exists can pass in a pid of a 
 *  process and the module will kill it.  This can 
 *  be any process on the system ... mu ha ha ha!
 */

/*  
 *  Basic ideas taken from Peter Jay Salzman 
 *  paper on writing kernel modules. Very good.
 *  you can check it out @
 *  http://gnu.kookel.org/ftp/LDP/LDP/lkmpg/2.6/html/lkmpg.html
 */

/*
 * Kernel Header Files 
 */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module, */
#include <linux/fs.h>		/* inode struct */
#include <asm/uaccess.h>	/* for put_user */
#include <linux/sched.h>	/* for task info and pid */

#include "module_killpid.h"	/* include my stuff */

/* 
 * Initialize the module 
 */
int init_module()
{
	/*
	 * Ask the kernel nicly to give us an unused Device Major number
	 */
	Major = register_chrdev(0, DEVICE_NAME, &fops); 
	if (Major < 0) {
		printk("Registering character device failed: %d\n",Major);
		return Major;
	}

	/*
	 * Print out the Major Dev number that we got to syslg
	 */
	printk( KERN_ALERT "module_killpid assigned major: %d.\n", Major );
	printk( KERN_ALERT "Once the device is created with\n" );
	printk( "'mknod /dev/killpid c %d 0' pass in any", Major ); 
	printk( "pid to have the module kill the process" );
	printk( "mu ha ha ha ha!!!\n");
	
	return 0;
	
}

/* 
 *  Cleanup - unregister the char dev 
 */
void cleanup_module()
{
	/*
	 * Unregister the device major number with the kernel
	 */
	int ret = unregister_chrdev( Major, DEVICE_NAME );
	if( ret < 0 ){
		printk("Error in unregister_chrdev: %d\n", ret );
	}
}


/* 
 *  * Called when a process tries to open the device file, like
 *   * "cat /dev/mycharfile"
 *    */
static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open){
		return -EBUSY;
	}
	Device_Open++;
	sprintf(msg, "Send me a PID to kill\n");
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
	
	return SUCCESS;
}


/* 
 *  Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

/* 
*  Decrement the usage count, or else once you opened the file, you'll
*  never get get rid of the module. 
*/
	module_put(THIS_MODULE);
	
	return 0;
}

/* 
 *  Called when a process, which already opened the dev file, attempts to
 *  read from it.
 */
static ssize_t device_read( struct file *filp, char *buffer,
			    size_t length, loff_t * offset)
{
/*
 * Number of bytes actually written to the buffer 
 */	int bytes_read = 0;

/*
 * If we're at the end of the message, 
 * return 0 signifying end of file 
 */
	if (*msg_Ptr == 0){
		return 0;
	}

/* 
 * Actually put the data into the buffer 
 */
 	while (length && *msg_Ptr) {
/* 
 * The buffer is in the user data segment, not the kernel 
 * segment so "*" assignment won't work.  We have to use 
 * put_user which copies data from the kernel data segment to
 * the user data segment. 
 */
		put_user(*(msg_Ptr++), buffer++);
		length--;
		bytes_read++;
	}

/* 
 * Most read functions return the number of bytes put into the buffer
 */
	return bytes_read;
}


/*  
 * Called when a process writes to dev file: echo "1040" > /dev/killpid 
 */
static ssize_t
device_write(struct file *file,
	const char __user * buffer, size_t length, loff_t * offset)
{
	int i, pid;
	char msg2[ BUF_LEN ];
	
	for (i = 0; i < length && i < BUF_LEN; i++){
		get_user(msg2[i], buffer + i);
	}
	/* 
	 * Here is where we convert the ascii to int
	*  with our hacked atoi function :)
	*/
	pid = atoi( msg2, i );
	task = find_task_by_pid(pid);	/* Use the kernel function to get a task struct */
	/* print a msg to the log that we are killing the pid */
	printk("Killing process: %d\n", task->pid );
	/* kill the mofo */
	kill_proc(task->pid, SIGKILL, 1);

	return i;		/*  Again, return the number of input characters used  */
}

/*
 * atoi function since I can not find one in kernel code/
 * there has got to be one somwhere but I just can't find it
 * ... this is a hack function that should be replaced when 
 * I find the right one to use :)   
 */ 
static int atoi( char* convert, int len )
{
	int ret = 0;		/* atoi return value */
	int i, j, tens;		/* local storage */
	len-=2;			/* hack to get the len in right spot */
	for( i=len; i>=0; i-- ){
		if( convert[i] >= '0' && convert[i] <= '9' ){
			tens = 1;
			for( j=0; j<(len-i); j++) /* C coule use a built in pow operator */
				tens*=10;	  /* since they dont .. i will hack my own */
			/* add the value to the return */
			ret += (tens>9) ? tens*(int)(convert[i]-48):(int)(convert[i]-48);
		}
	}
	/* return the atoi value */
	return ret;
}

//

