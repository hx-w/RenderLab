#include "service.h"

#include <iostream>
#include <communication/ContextHub.h>

using namespace std;
using namespace fundamental;

namespace ToothSpace {
    ToothService::ToothService(ToothEngine& engine) noexcept
        : m_engine(engine), m_autobus(make_unique<AutoBus>()) {
        _subscribe();
    }

    ToothService::~ToothService() {
        m_autobus.reset();
    }

    void ToothService::_subscribe() {
        m_autobus->subscribe<void(const string&)>(SignalPolicy::Sync, "GUI/filepath_selected",
            [this](const string& path) {
                cout << path.c_str() << endl;
            });
    }
}
