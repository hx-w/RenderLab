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

void text_preset(shared_ptr<RenderService> rservice) {
    // text preset
    {
        RenderLine rline;
        rline.frame_remain = -1;
        rline.Segments.emplace_back(TextSegment{"[data generating] ", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rline.Segments.emplace_back(TextSegment{"0.00%", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rservice->add_text(BoxRegion::BOX_RIGHT_TOP, move(rline));
    }
    {
        RenderLine rline;
        rline.frame_remain = -1;
        rline.Segments.emplace_back(TextSegment{"[ml training] ", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rline.Segments.emplace_back(TextSegment{"0.00%", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rservice->add_text(BoxRegion::BOX_RIGHT_TOP, move(rline));
    }
    {
        RenderLine rline;
        rline.frame_remain = -1;
        rline.Segments.emplace_back(TextSegment{"[server deploying] ", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rline.Segments.emplace_back(TextSegment{"0.00%", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rservice->add_text(BoxRegion::BOX_RIGHT_TOP, move(rline));
    }
    {
        RenderLine rline;
        rline.frame_remain = -1;
        rline.Segments.emplace_back(TextSegment{"[mesh reconstruction] ", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rline.Segments.emplace_back(TextSegment{"0.00%", glm::vec3(0.7f, 0.7f, 0.7f), 20});
        rservice->add_text(BoxRegion::BOX_RIGHT_TOP, move(rline));
    }
}

void text_update(shared_ptr<RenderService> rservice, int index, double ratio) {
    string header;
    glm::vec3 color;
    switch (index) {
        case 0:
            header = "[data generating] ";
            break;
        case 1:
            header = "[ml training] ";
            break;
        case 2:
            header = "[server deploying] ";
            break;
        case 3:
            header = "[mesh reconstruction] ";
            break;
        default:
            break;
    }
    char buffer[32] = {0};
    if (ratio < 0.5) {
        color = glm::vec3(1.0, 0.7, 0.7);
        snprintf(buffer, sizeof(buffer), "%2.2f%%", ratio * 100);
    }
    else if (ratio < 0.98) {
        color = glm::vec3(0.7, 1.0, 0.7);
        snprintf(buffer, sizeof(buffer), "%2.2f%%", ratio * 100);
    }
    else {
        color = glm::vec3(0.0, 1.0, 0.0);
        snprintf(buffer, sizeof(buffer), "%s", "success");
    }
    RenderLine rline;
    rline.frame_remain = -1;
    rline.Segments.emplace_back(TextSegment{header, glm::vec3(0.7, 0.7, 0.7), 20});
    rline.Segments.emplace_back(TextSegment{buffer, color, 20});
    rservice->update_text(BoxRegion::BOX_RIGHT_TOP, index, move(rline));
}

void train_and_test(int scale, shared_ptr<RenderService> rservice) {
    vector<string> train_set;
    vector<string> test_set;
    train_test_split(train_set, test_set, 0.2);

    text_preset(rservice);

    // 创建单独线程进行训练
    remove("static/dataset/edge_line.csv");
    remove("static/dataset/uv_point.csv");


    thread ml_train_thread([&](){
        // gen data
        auto train_size = train_set.size();
        for (auto id = 0; id < train_size; ++id) {
            text_update(rservice, 0, id * 1.0 / train_size);
            auto service = ToothSpace::make_service(train_set[id], scale);
            service->retag_point();
            text_update(rservice, 0, (id + 0.5) * 1.0 / train_size);
            service->simulate();
            text_update(rservice, 0, (id + 1.0) * 1.0 / train_size);
        }
        // train
        cout << "[INFO] train ml model..." << endl;
        text_update(rservice, 1, 0.0);
        execute("python3 scripts/ml_train.py");
        text_update(rservice, 1, 1.0);
    });
    ml_train_thread.join();

    thread ml_predict_thread([&](){
        this_thread::sleep_for(1000ms);
        cout << "[INFO] ml predict..." << endl;
        auto test_size = test_set.size();
        for (auto id = 0; id < test_size; ++id) {
            text_update(rservice, 3, id * 1.0 / test_size);
            auto service = ToothSpace::make_service(test_set[id], scale);
            service->retag_point_by_ml();
            text_update(rservice, 3, (id + 0.5) * 1.0 / test_size);
            service->simulate_by_ml();
            text_update(rservice, 3, (id + 1.0) * 1.0 / test_size);
        }
    });
    ml_predict_thread.detach();

    thread ml_server_thread([&](){
        cout << "[INFO] start ml server..." << endl;
        text_update(rservice, 2, 1.0);
        execute("python3 scripts/ml_server.py");
    });
    ml_server_thread.join(); // background
}


#endif
