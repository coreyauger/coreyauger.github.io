#ifndef _MODULE_KILLPID_H_
#define _MODULE_KILLPID_H_

/*
 * Function prototypes go here:
 */
	int init_module(void);
	void cleanup_module(void);
	static int device_open(struct inode *, struct file *);
	static int device_release(struct inode *, struct file *);
	static ssize_t device_read(struct file *, char *, size_t, loff_t *);
	static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
	static int atoi( char* convert, int len ); /* My lame atoi function */
	
	#define SUCCESS 0
	#define DEVICE_NAME "killpid"	/* Dev name as it appears in /proc/devices   */
	#define BUF_LEN 32		/* Max length of the message from the device */
	
	/* 
	* Global variables are declared as static, so are global within the file. 
	*/
	
	static int Major;		/* Major number assigned to our device driver */
	static int Device_Open = 0;	/* Is device open?  
					Used to prevent multiple access to device */
	static char 	msg[BUF_LEN];	/* The msg the device will give when asked */
	static char 	*msg_Ptr;
	static task_t 	*task;		/* The task structure that we use to print 
					out info of the task before we kill it */
/* 
 * This next structure tells the compiler which operations 
 * we are going to support in our module.  Anything that is 
 * not listed here gets a NULL value and is not implemented
 */
	static struct file_operations fops = {
		.read = device_read,
		.write = device_write,
		.open = device_open,
		.release = device_release
	};

#endif

//
