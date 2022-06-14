#ifndef TRAIN_AND_TEST_HPP
#define TRAIN_AND_TEST_HPP

#include <string>
#include <vector>
#include <random>
#include <memory>
#include "tooth/execute.h"
#include "render/service.h"

using namespace std;
using namespace RenderSpace;

// 获取指定目录下的子目录
#ifdef _WIN32
#include <io.h>

void getSubdirs(const string& path, vector<string>& subdirs) {
    //文件句柄
    long   hFile   =   0;
    //文件信息
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1) {
        do {
            if ((fileinfo.attrib &  _A_SUBDIR)) {
                if (fileinfo.name[0] == 'N') {
                    subdirs.push_back(p.assign(path).append("\\").append(fileinfo.name) );
                }
            }
        } while(_findnext(hFile, &fileinfo)  == 0);
        _findclose(hFile);
    }
}

#else
#include <dirent.h>

void getSubdirs(const string& path, vector<string>& subdirs) {
    DIR* dir;
    struct dirent* ptr;

    if ((dir = opendir(path.c_str())) == NULL) {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (ptr->d_type == 4 && ptr->d_name[0] == 'N') {
            subdirs.push_back(path + "/" + ptr->d_name);
        }
    }
    closedir(dir);
}

#endif

void train_test_split(vector<string>& train_set, vector<string>& test_set, double test_ratio) {
    vector<string> subdirs;
    getSubdirs("static/nurbs", subdirs);
    auto rd = random_device{}; 
    auto rng = default_random_engine{ rd() };
    shuffle(begin(subdirs), end(subdirs), rng);
    int train_size = subdirs.size() * (1 - test_ratio);
    for (int i = 0; i < train_size; ++i) {
        train_set.push_back(subdirs[i]);
    }
    for (int i = train_size; i < subdirs.size(); ++i) {
        test_set.push_back(subdirs[i]);
    }
}

void train_and_test(int scale, shared_ptr<RenderService> rservice) {
    vector<string> train_set;
    vector<string> test_set;
    train_test_split(train_set, test_set, 0.2);

    // 创建单独线程进行训练
    // remove("static/dataset/edge_line.csv");
    // remove("static/dataset/uv_point.csv");


    thread ml_train_thread([&](){
        // gen data
        auto train_size = train_set.size();
        for (auto id = 0; id < train_size; ++id) {
            auto service = ToothSpace::make_service(train_set[id], scale);
            service->retag_point();
            service->simulate();
        }
    });
    ml_train_thread.join();

//     thread ml_predict_thread([&](){
//         this_thread::sleep_for(1000ms);
//         cout << "[INFO] ml predict..." << endl;
//         auto test_size = test_set.size();
//         for (auto id = 0; id < test_size; ++id) {
//             auto service = ToothSpace::make_service(test_set[id], scale);
//             service->retag_point_by_ml();
//             service->simulate_by_ml();
//         }
//     });

//     thread ml_server_thread([&](){
//         cout << "[INFO] start ml server..." << endl;
//         execute("python3 scripts/ml_server.py");
//     });

// #ifdef _WIN32
//     ml_server_thread.detach(); // background
//     ml_predict_thread.join();
// #else
//     ml_predict_thread.detach();
//     ml_server_thread.join(); // background
// #endif
}


#endif
