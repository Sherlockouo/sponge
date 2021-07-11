# Lab3 TCP Sender

![image-20210711084956428](https://gitee.com/coder-wdf/picgo/raw/master/img/image-20210711084956428.png)

##TCP Sender’s duty.

 	1. 读取字节流
 	2. 把字节流转换为一系列的TCP 段

## TCP Sender 和 Receiver

> Sender 和 Receiver 都负责一系列的TCP 段

**Sender **：

 	1. 写入所有与Receiver相关的域——序列号，SYN标记，负载，FIN标记
 	2. 读取TCP 信息段中Receiver写的域。ACKNO 和 window size

![image-20210711090339111](https://gitee.com/coder-wdf/picgo/raw/master/img/image-20210711090339111.png)

It will be your TCPSender’s responsibility to:

- Keep track of the receiver’s window (processing incoming acknos and window sizes)
- Fill the window when possible, by reading from the ByteStream, creating new TCP segments (including SYN and FIN flags if needed), and sending them. The sender should keep sending segments until either the window is full or the ByteStream is empty.
- Keep track of which segments have been sent but not yet acknowledged by the receiver— we call these “outstanding” segments 
- Re-send outstanding segments if enough time passes since they were sent, and they haven’t been acknowledged yet

