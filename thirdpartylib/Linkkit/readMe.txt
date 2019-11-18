阿里云工业互联网平台

>>>>>>使用方法（Instructions）：
1.链接库libLPBx30Linkkit.a；
2.添加以下头文件路径(可能需要根据工程文件所在位置不同而修改)：
  ..\..\thirdpartylib\Linkkit\include
  ..\..\thirdpartylib\Linkkit\include\exports
  ..\..\thirdpartylib\Linkkit\include\imports
3.编译时定义以下宏：
  MQTT_DIRECT MQTT_COMM_ENABLED DEVICE_MODEL_ENABLED
4.参考linkkittest.c使用。


>>>>>>相关资料：
https://help.aliyun.com/product/93051.html