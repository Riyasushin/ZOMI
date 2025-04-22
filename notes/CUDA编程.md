

# 硬件和软件的概念对应

Grid   --- Device(设置的)
Block  --- SM
Thread --- 一个SM中的线程

每个 GPC 中包含 TPC（Texture processing cluster）表示纹理处理簇，
每个处理簇被分为多个 SM（Streaming Multiprocessors）流处理器，
SM 中包含多个 CUDA Core 和 Tensor Core，用于处理图形和 AI 张量计算。

一个 block 上线程放在同一个 SM 上执行，一个 SM 有限的 Cache 制约了每个 block 的线程数量。