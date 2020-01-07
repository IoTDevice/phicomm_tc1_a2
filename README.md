# phicomm_tc1_a2

#### 使用[汉枫SDK](http://www.hi-flying.com/download-center-1/software-development-kit-1/download-item-hf-lpx30-hsf-sdk-1mb-2mb)来开发的斐讯插排TC1 A2的第三方固件，基于https://github.com/linlyv/TC1_A2 和 https://github.com/a2633063/zTC1
#### 使用SDK版本为LPx30-HSF_v4.13.24

## 使用方法(摘录自https://github.com/linlyv/TC1_A2)

### 1.接线：

把第10号引脚（复位引脚）接跟线出来，串口接第5（tx）和第6引脚(rx)，电源供电引线最好接5V（测试点）,烧录固件时不要接220V交流电

通过串口工具连接到电脑，打开secureCRT软件，设置参数230400,8,none,1 ，连接上模块。

<img src="./doc/lpt230.jpg" width="540" >
> 图1 要接线的点（用到的点）


### 2.烧录固件

按住板上的按键，另一只手把复位引线碰一下GND(测试点)，接着在1秒内按电脑上的空格键（鼠标光标要在接收窗口上）。

看到有打印出Bootloader的信息，接着按照输入‘S’，再输入‘Y’,  点击菜单栏的"传输"(Transfer)，选择“发送Xmodem”，然后就选择[XXX_gcc.bin](https://github.com/IoTDevice/phicomm_tc1_a2/releases)文件，打开。

等待传输完成即可。

<img src="./doc/step1.png" width="540">
<img src="./doc/step2.png" width="540">
<img src="./doc/step3.png" width="540">

### 3.配网

使用[云易连](https://github.com/OpenIoTHub/OpenIoTHub)app配网

### 4.app控制
使用云易连APP（OPenIoTHub），直接云易连添加设备，app配网自动发现[OpenIoTHub](https://github.com/OpenIoTHub/OpenIoTHub)。

### 5.固件升级

串口升级和web网页升级

web网页升级:只需要访问模块的ip地址+iweb.html(例如：http://192.168.123.184/iweb.html),就可以打开升级页面，选择第一项Upgrade application，浏览文件选择“[XXX_upgrade.bin](https://github.com/IoTDevice/phicomm_tc1_a2/releases)”文件，点击Upload
即可，传输成功会返回：Update successful !

<img src="./doc/webup.jpg" width="540">
