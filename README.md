# videostream
a videostreamer with capabilities: capture yuv/mjpeg, encode by openamx il on raspberrypi, containers mp4/flv, streamer: rtmp 

libuserv4l2:
通过v4l2的ioctl调用实现的一个用户空间采集yuv或者mjpeg格式（或者其他格式，需要camera isp fw和driver的支持，在我的笔记本上支持yuyv422和mjpeg格式）的image数据lib。
encoder:
采用openmax il实现的基于raspberrypi的gpu加速的硬件h264编码。
containers:
mp4是对mp4v2库的wrap；
flv根据协议手册编写，实现了简单的封装功能。
streamer:
rtmp是对rtmpdump的wrap。
