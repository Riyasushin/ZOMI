
# MPI 关键词

通信

点对点通信
send 
receive

message envelop

In/Out buf
IN count
IN datatype
IN dest
IN source
IN tag
IN comm
OUT status


问题：死锁

非阻塞通信
Cancel
Test
Wait

集合通信(接口)

reduce
allreduce
gather
allgather
broadcast
scatter
就是通信的概念:
通信术语：
- All-reduce： 收集起来，搓一发，发给所有
- All-Gather：互相广播，每个节点都收集所有节点的数据
- Broadcast：分发操作
- Reduce：搓一发，发给特定节点
- Scatter：一个节点分发给其他节点
- Gather：收集到一个节点上

Error

消息，通信方式

进程组

跨节点起进程

# MPI 的通讯， MPI的通讯库