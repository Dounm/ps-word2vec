/**
* @file main.cpp
* @author Dounm <niuchong893184@gmail.com>
* @date 2017-02-07
*/

#include <cstdlib>
#include <iostream>
#include <string>
#include "ps/ps.h"
#include "word2vec.h"

#include <thread>
#include <chrono>

void StartServer() {
    if (!ps::IsServer()) return;
    auto server = new ps::KVServer<float>(0);
    server->set_request_handle(ps::KVServerDefaultHandle<float>());     // float, not double
    ps::RegisterExitCallback([server](){ delete server; });
}

int main(int argc, char **argv) {
    StartServer();
    ps::Start();

    int rank = ps::MyRank();
    if (ps::IsWorker()) {
        // std::this_thread::sleep_for(std::chrono::seconds(2));
        srand(time(NULL));
        psw2v::Word2Vec model(50, 200, 128, 5, 0.005);
        model.load_dict("ig.data/text8.dict");
        model.train("ig.data/train_data/text8.p" + std::to_string(rank));
    }

    ps::Finalize();

    std::cout << "all done" << std::endl;
}
