#include "service.h"
#include "printer.h"

using namespace std;

namespace ToothSpace {
    ToothService::ToothService(
        ToothEngine& engine,
        const string& dir, int scale
    ) noexcept : m_engine(engine) {
        // preset
        _init(dir, scale);
    }

    ToothService::ToothService(ToothEngine& engine) noexcept
        : m_engine(engine) {}

    ToothService::~ToothService() {
        _reset();
    }

    void ToothService::_init(const string& dir, int scale) {
        _reset();
        m_faces.emplace_back(NURBSFace(dir + "/face-1.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + "/face-2.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + "/face-3.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + "/face-4.txt", scale, true));
    }

    void ToothService::_reset() {
        m_faces.clear();
        FaceList().swap(m_faces);
    }
}