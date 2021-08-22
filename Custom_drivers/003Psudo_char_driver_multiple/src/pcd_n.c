#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>



#include<linux/kernel.h>
#include<linux/list.h>

#define NO_OF_DEVICES  4


/*permission codes */
#define RDONLY 0x01
#define WRONLY 0X10
#define RDWR   0x11

#define DEV_MEM_SIZE_PCDEV1   1024
#define DEV_MEM_SIZE_PCDEV2   500
#define DEV_MEM_SIZE_PCDEV3   1024
#define DEV_MEM_SIZE_PCDEV4   700
 
#undef pr_fmat 
#define pr_fmt(fmt)  "%s :" fmt,__func__

/*pseudo deice memory*/
char device_buffer_pcdev1[DEV_MEM_SIZE_PCDEV1];
char device_buffer_pcdev2[DEV_MEM_SIZE_PCDEV2];
char device_buffer_pcdev3[DEV_MEM_SIZE_PCDEV3];
char device_buffer_pcdev4[DEV_MEM_SIZE_PCDEV4];



/*device private structure*/
struct pcdev_private_data{

	char *buffer;
	unsigned size;
	const char *serial_number;
	int perm;
	struct cdev cdev;

};

/*Driver private data sturecture */
struct pcdrv_private_data{

	int total_devices;
	
 	/*this hold the device number*/
        dev_t device_number;
        
        struct class *class_pcd;
	struct device *device_pcd;
        
	struct pcdev_private_data  pcdev_data[NO_OF_DEVICES];
};


struct pcdrv_private_data pcdrv_data = {

	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		
		[0] = {
			.buffer = device_buffer_pcdev1,
			.size = DEV_MEM_SIZE_PCDEV1,
			.serial_number = "PCDEV111111111111111!",
			.perm = RDONLY  /*Read only*/
		},
		[1] = {
			.buffer = device_buffer_pcdev2,
			.size = DEV_MEM_SIZE_PCDEV2,
			.serial_number = "PCDEV222222222222!",
			.perm = WRONLY  /*Write only*/
		},
		[2] = {
			.buffer = device_buffer_pcdev3,
			.size = DEV_MEM_SIZE_PCDEV3,
			.serial_number = "PCDEV3333333333!",
			.perm = RDWR  /*Read Write only*/
		},
		[3] = {
			.buffer = device_buffer_pcdev4,
			.size = DEV_MEM_SIZE_PCDEV4,
			.serial_number = "PCDEV44444444444444!",
			.perm = RDWR  /*Read Write only*/
		}	
		},
		



};





off_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
       struct pcdev_private_data *pcdev_data =  (struct pcdev_private_data *)filp->private_data;
       
       int max_size = pcdev_data->size;
       
        loff_t temp;

	pr_info("lseek requested \n");
	pr_info("Current value of the file position = %lld\n",filp->f_pos);

	switch(whence)
	{
		case SEEK_SET:
			if((offset > max_size) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if((temp > max_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = max_size + offset;
			if((temp > max_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}
	
	pr_info("New value of the file position = %lld\n",filp->f_pos);

	return filp->f_pos;

    
    return 0;
}
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){
 
       struct pcdev_private_data *pcdev_data =  (struct pcdev_private_data *)filp->private_data;
       
       int max_size = pcdev_data->size;
      
        pr_info("😀️read request for %zu byte \n",count);
        pr_info ("current file position  = %lld \n", *f_pos);
        
        /*adjust the count*/
        if((*f_pos + count) > max_size){
        	count =  max_size - *f_pos;
        }
        
        /*copy to user*/
	if(copy_to_user(buff, &pcdev_data->buffer[*f_pos], count)){
	
		return -EFAULT;
        }
        
        /*update the current file position*/
        *f_pos += count;
        
        pr_info ("Numbervof byte succesfully read = %zu \n", count);
        pr_info ("Update the file position  = %lld \n", *f_pos);
        /*Return the number of byte*/
        
	return count;
	

  return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
    
     
       struct pcdev_private_data *pcdev_data =  (struct pcdev_private_data *)filp->private_data;
       
       int max_size = pcdev_data->size;
       	
        pr_info("write request for %zu byte \n",count);
        pr_info ("current file position  = %lld \n", *f_pos);
        
        /*adjust the count*/
        if((*f_pos + count) > max_size){
        	count =  max_size - *f_pos;
        }
        
        if(!count)
        	return -ENOMEM;
        /*copy from user*/
	if(copy_from_user(&pcdev_data->buffer[*f_pos], buff,count)){
	
		return -EFAULT;
        }
        
        /*update the current file position*/
        *f_pos += count;
        
        pr_info ("Numbervof byte succesfully written = %zu \n", count);
        pr_info ("Update the file position  = %lld \n", *f_pos);
        
        /*Return the number of byte succesfully written*/        
	return count;	
  
 
       return 0;
}   


int check_permission(int dev_perm, int acc_mode){

	if(dev_perm == RDWR)
		return 0;
		
	//ensures readonly access
	if( (dev_perm == RDONLY) && ( (acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE) ) )
		return 0;
	
	//ensures writeonly access
	if( (dev_perm == WRONLY) && ( (acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ) ) )
		return 0;

	return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp){
        
       int ret;
       int minor_n;
       struct pcdev_private_data *pcdev_data;
       
       /*Find out on which devices file open was attempted by the user space*/
       minor_n = MINOR(inode->i_rdev);
       pr_info("minor access = %d\n",minor_n);
       
       /*Get device private data structure*/
       pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);

       /*to supply device private data to other method of driver*/
       filp->private_data = pcdev_data;
       
       /*check the permiision*/
       ret= check_permission(pcdev_data->perm,filp->f_mode);
       
       (!ret)?pr_info("Open Was successful\n"):pr_info("open was unsuccessfull\n");
       return ret;
}

int pcd_release (struct inode *inode, struct file *filp){
        
        pr_info("closed was succesfull");
        return 0;
}
       

/*file operation of the driver*/
struct file_operations pcd_fops= {

	.open= pcd_open,
	.write= pcd_write,
	.read=pcd_read,
	.release=pcd_release,
	
	.owner = THIS_MODULE


};


static int __init pcd_driver_init(void){

	int ret;
	int i;	
	
	/*1.dynamic alocate the device number*/
	ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcd_devices");
	if(ret<0)
	{
		pr_info("Alloc char failed\n");
		goto out;
	}
	
	/*4. create device class under /sys/class/*/
	pcdrv_data.class_pcd= class_create(THIS_MODULE, "pcd_class");
	if(IS_ERR(pcdrv_data.class_pcd))
	{
		pr_err("Class creation failed\n");
		ret = PTR_ERR(pcdrv_data.class_pcd);
		goto unreg_chrdev;
	}
	
	for(i=0;i<NO_OF_DEVICES;i++){
	
		pr_info("Device number <majar>:<minor> = %d:%d \n", MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));
		

	
		/*initialize the cdev struture with fops*/
		cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);
	
		/*3. Register a device (cdev structure) with VFS*/
		pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
		ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number + i,1);
		if(ret<0)
		{
			pr_info("cdev add failed\n");
			
			goto cdev_del;
		}
	
	
		/* populate the sysfs with device information*/
	
		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number + i ,NULL, "pcdev-%d",i+1);
		if(IS_ERR(pcdrv_data.device_pcd))
		{
			pr_err("device creation failed\n");
			ret = PTR_ERR(pcdrv_data.device_pcd);
			goto class_del;
		}

	}
	
	pr_info("module init was succesfull\n");

	return 0;
	
cdev_del:
	
class_del:
	for(;i>=0;i--){
	device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + 1);
	cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	
	class_destroy(pcdrv_data.class_pcd);
	
unreg_chrdev:
	unregister_chrdev_region(pcdrv_data.device_number + 1,NO_OF_DEVICES);	
out: 
	pr_info("Module insertion failed");
	return ret;
}

static void __exit pcd_driver_cleanup(void){

	int i;
	for(i=0; i< NO_OF_DEVICES;i++){
		device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + 1);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);		
		
	}
	class_destroy(pcdrv_data.class_pcd);
	unregister_chrdev_region(pcdrv_data.device_number + 1,NO_OF_DEVICES);
	
	pr_info("module unloaded.........\n");


}


module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MITHLESH");
MODULE_DESCRIPTION("A pseudo char driver for 4 devices");






















