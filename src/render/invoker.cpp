#include "invoker.h"

#include <iostream>

using namespace std;


namespace RenderSpace {

    void Command::invoke() {
        switch (m_type) {
        case AddDrawable:
            cout << "AddDrawable" << endl;
            break;
        case RemoveDrawable:
            cout << "RemoveDrawable" << endl;
            break;
        case UpdateDrawable:
            cout << "UpdateDrawable" << endl;
            break;
        case HideOrShowDrawable:
            cout << "HideOrShowDrawable" << endl;
            break;
        case Pick:
            cout << "Pick" << endl;
            break;
        default:
            break;
        }
    }

    void CommandQueue::push(Command&& cmd) {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push(std::move(cmd));
    }

    void CommandQueue::invoke() {
        lock_guard<mutex> lock(m_mutex);
        
        while (!m_queue.empty()) {
            auto cmd = m_queue.pop();
            if (cmd != nullptr) {
                cmd->invoke();
            }
        }
    }

}