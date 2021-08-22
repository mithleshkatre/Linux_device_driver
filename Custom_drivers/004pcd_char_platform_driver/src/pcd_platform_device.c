#include<linux/module.h>
#include<linux/platform_device.h>


#include"platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

void pcdev_release(struct device *dev){

	pr_info("Device Release\n");
}

//1. create 2. platform data

struct pcdev_platform_data pcdev_pdata[4]=
{
	[0]= {.size = 512, .perm= RDWR, .serial_number = "PCDEV11111111111!"},
	[1]= {.size = 1024, .perm= RDWR, .serial_number = "PCDEV22222222222!"},
	[2]= {.size = 128, .perm= RDONLY, .serial_number = "PCDEV33333333333!"},
	[3]= {.size = 32, .perm= WRONLY, .serial_number = "PCDEV444444444444!"},

};


//1. create 2. platform device



struct platform_device platform_pcdev_1 = {

	.name = "pcdev-A1x",
	.id= 0,
	.dev = {
		.platform_data = &pcdev_pdata[0],
		.release = pcdev_release
		}
};


struct platform_device platform_pcdev_2 = {

        .name = "pcdev-B1x",
        .id= 1,
	.dev = {
                .platform_data = &pcdev_pdata[1],
		.release = pcdev_release
               }

};

struct platform_device platform_pcdev_3 = {

        .name = "pcdev-C1x",
        .id= 2,
	.dev = {
                .platform_data = &pcdev_pdata[2],
		.release = pcdev_release
               }

};

struct platform_device platform_pcdev_4 = {

        .name = "pcdev-D1x",
        .id= 3,
	.dev = {
                .platform_data = &pcdev_pdata[3],
		.release = pcdev_release
               }

};

struct platform_device *platform_pcdevs[]={

	&platform_pcdev_1,
	&platform_pcdev_2,
	&platform_pcdev_3,
	&platform_pcdev_4
};


static int __init pcdev_platform_init(void){

	//register platform drvice
	//platform_device_register(&platform_pcdev_1);
	//platform_device_register(&platform_pcdev_2);
	
	platform_add_devices(platform_pcdevs, ARRAY_SIZE(platform_pcdevs));
	pr_info("Device Setup module Loaded\n");


	return 0;
}


static void  __exit pcdev_platform_exit(void){

	 //unregister platform drvice
        platform_device_unregister(&platform_pcdev_1);
        platform_device_unregister(&platform_pcdev_2);
        platform_device_unregister(&platform_pcdev_3);
        platform_device_unregister(&platform_pcdev_4);

	 pr_info("Device Setup module UnloadedS\n");

       
}


module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("moduel which register the platform devices");


































