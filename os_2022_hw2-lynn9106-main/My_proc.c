
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <stdarg.h>
#include <uapi/linux/string.h>
#include <asm/string.h>

MODULE_DESCRIPTION("my_proc");
MODULE_AUTHOR("Lynn");
MODULE_LICENSE("GPL");

char total_threadtime[1024];	//store all the final information of the threads
static struct proc_dir_entry *proc_file;

/* 	Function is called when proc file is read
	(user needs to print the utime and context switch time of the threads)*/
static ssize_t procfile_read(struct file *filePointer, char *buffer,size_t buffer_length, loff_t *offset)
{
int slen=strlen(total_threadtime);
char s[slen];
strcpy(s,total_threadtime);
    int len = sizeof(s);
    ssize_t ret = len;
    if (*offset >= len || copy_to_user(buffer, s, len)) {		//copy total_threadtime to user buffer
        pr_info("copy_to_user failed\n");
        ret = 0;
    } else {
        *offset += len;
    }
    strcpy(total_threadtime,"");		//clear char* for next time
    return ret;
}

char procfs_buffer[20];		//store the string read from user buffer
static unsigned long procfs_buffer_size = 0;

/*	Function is called when the procfile is written
	(user write the pid and tid of a thread into /proc/thread_info)*/
static ssize_t procfile_write(struct file *File, const char *user_buffer, size_t len, loff_t *offs) {
procfs_buffer_size = len;
    if (copy_from_user(procfs_buffer, user_buffer, procfs_buffer_size))	//copy user buffer to procfs_buffer
        return -EFAULT;
    procfs_buffer[procfs_buffer_size] = '\0';
    *offs += procfs_buffer_size;

	char *token;			//convert string into long int
	long tid_int=0,pid_int=0;
	long *tgid=&tid_int;long *pid=&pid_int;
	char* procptr=procfs_buffer;
	token = strsep((&procptr)," ");
	kstrtol(token,10,tgid);
	kstrtol(procptr,10,pid);

   struct task_struct *the_process;		//search the task_struct of the thread to get utime, nvcsw and nivcsw
   struct task_struct *the_thread;
	for_each_process(the_process) {
		if(task_tgid_nr(the_process)== *tgid){
			for_each_thread(the_process,the_thread){
				if(task_pid_nr(the_thread)== *pid){
						long long t_time = (the_thread->utime/1000000);
						unsigned long scount = the_thread->nvcsw+the_thread->nivcsw;
						strcpy(procfs_buffer,"");
						sprintf(procfs_buffer,"%ld %lld %lu ",*pid,t_time,scount);		//store the information of the thread into procfs_buffer
						strcat(total_threadtime,procfs_buffer);							//and append to total_threadtime
						return procfs_buffer_size;
				}
			}
		}
	}
    return procfs_buffer_size;
}


static const struct proc_ops fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};

/*
 Function is called when the module is loaded into the kernel
 */
static int __init my_init(void) {
	proc_file = proc_create("thread_info", 0777, NULL, &fops);
	if(proc_file == NULL) {
		printk("procfs_test - Error creating /proc/thread_info\n");
		return -ENOMEM;
	}

	printk("procfs_test - Created /proc/thread_info\n");
	return 0;
}

/*
 Function is called when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("procfs_test - Removing /proc/thread_info\n");
	proc_remove(proc_file);
}

module_init(my_init);
module_exit(my_exit);
