
// https://mpitutorial.com/tutorials/point-to-point-communication-application-random-walk/zh_cn/
// 随机游走问题的并行化处理
// 为什么不是多个球??

#include <mpi.h>
#include <time.h>

#include <cstdlib>
#include <iostream>
#include <vector>

using std::endl, std::cout, std::cerr;

/// @brief 考虑域的总大小，并为 MPI
/// 进程找到合适的子域,将域的其余部分交给最终的进程
/// @param domain_size 待分解的一维计算域的总大小
/// @param world_rank 当前进程的全局编号
/// @param world_size 参与计算的 MPI 进程总数
/// @param subdomain_start 当前进程负责的子域起始索引
/// @param subdomain_size 当前进程负责的子域大小
void decompose_domain(int domain_size, int world_rank, int world_size,
                      int* subdomain_start, int* subdomain_size) {
    if (world_size > domain_size) {
        // 总进程数（world_size）超过了域的总大小（domain_size）
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    *subdomain_start = domain_size / world_size * world_rank;
    *subdomain_size = domain_size / world_size;

    // 余数处理
    if (world_rank == world_size - 1) {
        *subdomain_size += domain_size % world_size;
    }
}

struct Walker {
    int location;
    int num_steps_left_in_walk;
};

/// @brief 初始化 walkers,采用子域边界，并将 walker 添加到 incoming_walkers中
/// @param num_walkers_per_proc
/// @param max_walk_size
/// @param subdomain_start
/// @param incoming_walkers
void init_walkers(int num_walkers_per_proc, int max_walk_size,
                  int subdomain_start, std::vector<Walker>* incoming_walkers) {
    Walker walker;
    for (int i = 0; i < num_walkers_per_proc; ++i) {
        walker.location = subdomain_start;
        walker.num_steps_left_in_walk =
            (rand() / (float)RAND_MAX) * max_walk_size;
        incoming_walkers->push_back(walker);
    }
}

void walk(Walker* walker, int subdomain_start, int subdomain_size,
          int domain_size, std::vector<Walker>* outgoing_walkers) {
    while (walker->num_steps_left_in_walk > 0) {
        if (walker->location == subdomain_start + subdomain_size) {
            if (walker->location == domain_size) {
                walker->location = 0;
            }
            outgoing_walkers->push_back(*walker);
            break;
        } else {
            walker->num_steps_left_in_walk--;
            walker->location++;
        }
    }
}

// 发送待传出的 walker 的函数和接收待传入的 walker 的函数
void send_outgoing_walkers(std::vector<Walker>* outgoing_walkers,
                           int world_rank, int world_size) {
    MPI_Send((void*)outgoing_walkers->data(),
             outgoing_walkers->size() * sizeof(Walker), MPI_BYTE,
             (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);

    outgoing_walkers->clear();
}

void receive_incoming_walkers(std::vector<Walker>* incoming_walkers,
                              int world_rank, int world_size) {
    int incoming_rank = (world_rank == 0) ? world_size - 1 : world_rank - 1;

    MPI_Status status;
    MPI_Probe(incoming_rank, 0, MPI_COMM_WORLD, &status);

    int incoming_walkers_size;
    MPI_Get_count(&status, MPI_BYTE, &incoming_walkers_size);
    incoming_walkers->resize(incoming_walkers_size / sizeof(Walker));
    MPI_Recv((void*)incoming_walkers->data(), incoming_walkers_size, MPI_BYTE,
             incoming_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

int main(int argc, char** argv) {
    int domain_size;
    int max_walk_size;
    int num_walkers_per_proc;

    if (argc < 4) {
        cerr << "Usage: random_walk domain_size max_walk_size "
             << "num_walkers_per_proc" << endl;
        std::exit(1);
    }
    domain_size = atoi(argv[1]);
    max_walk_size = atoi(argv[2]);
    num_walkers_per_proc = atoi(argv[3]);

    MPI_Init(NULL, NULL);

    // 写的串行，if分支实现并行
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);  // !!! 打错字了！！！

    srand(time(NULL) * world_rank);
    int subdomain_start, subdomain_size;
    std::vector<Walker> incoming_walkers, outgoing_walkers;

    decompose_domain(domain_size, world_rank, world_size, &subdomain_start,
                     &subdomain_size);

    init_walkers(num_walkers_per_proc, max_walk_size, subdomain_start,
                 &incoming_walkers);

    cout << "Process " << world_rank << " initiated " << num_walkers_per_proc
         << " walkers in subdomain " << subdomain_start << " - "
         << subdomain_start + subdomain_size - 1 << endl;

    int maximum_sends_recvs = max_walk_size / (domain_size / world_size) + 1;

    for (int m = 0; m < maximum_sends_recvs; ++m) {
        for (int i = 0, len = incoming_walkers.size(); i < len; ++i) {
            walk(&incoming_walkers[i], subdomain_start, subdomain_size,
                 domain_size, &outgoing_walkers);
        }
        cout << "Process " << world_rank << " sending "
             << outgoing_walkers.size() << " outgoing walkers to process "
             << (world_rank + 1) % world_size << endl;

        if (world_rank % 2 == 0) {
            send_outgoing_walkers(&outgoing_walkers, world_rank, world_size);
            receive_incoming_walkers(&incoming_walkers, world_rank, world_size);
        } else {
            receive_incoming_walkers(&incoming_walkers, world_rank, world_size);
            send_outgoing_walkers(&outgoing_walkers, world_rank, world_size);
        }
        cout << "Process " << world_rank << " received "
             << incoming_walkers.size() << " incoming walkers" << endl;
    }

    cout << "Process " << world_rank << " done" << endl;
    MPI_Finalize();

    return 0;
}