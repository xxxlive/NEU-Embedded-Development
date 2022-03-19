接口参数

## ShowServer

Port： 9999

receive: 

1. recipe=xxx 菜谱需要展示的信息
2. clear=xxx 退出菜谱时展示的信息,也用于回到时间状态必须调用

## MenuServer

Port: 9997

receive:

1. fun=xxx 只接受'switch_left','switch_right','shut_down'三个参数
2. search = xxx 传入菜谱名字

## GasServer

Port:1000

Receive:

1. face=xxx 只接受‘YES','NO'