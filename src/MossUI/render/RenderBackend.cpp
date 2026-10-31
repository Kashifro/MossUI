#include "MossUI/render/RenderBackend.hpp"

namespace MossUI {

void RenderBackend::BeginFrame() {
    clips.clear();
    api.SetClip(nullptr);
    ++frame;
}

void RenderBackend::EndFrame() {
    clips.clear();
    api.SetClip(nullptr);
}

void RenderBackend::ResetTextures() {
    cache.clear();
}

void RenderBackend::PushClip(const Rect& r) {
    clips.push_back(r);
    api.SetClip(&clips.back());
}

void RenderBackend::PopClip() {
    if (!clips.empty()) clips.pop_back();
    api.SetClip(clips.empty() ? nullptr : &clips.back());
}

bool RenderBackend::Quad(const Rect& r, const std::string& path, const Uv& uv) {
    const void* tex = Resolve(path);
    if (!tex) return false;
    api.DrawQuad(tex, r, uv);
    return true;
}

const void* RenderBackend::Resolve(const std::string& path) {
    auto it = cache.find(path);
    std::string why;
    if (it == cache.end()) {
        Entry e;
        e.born = e.lastTry = frame;
        e.tex = api.LoadTexture(path, why);
        it = cache.emplace(path, std::move(e)).first;
        if (it->second.tex) return it->second.tex.get();
        return nullptr;   // just asked for: give the game a few frames before judging it missing
    }
    Entry& e = it->second;
    if (e.tex) return e.tex.get();

    const int age = frame - e.born;
    if (age < SETTLE_FRAMES) {
        e.tex = api.LoadTexture(path, why);
    } else if (frame - e.lastTry >= RETRY_FRAMES) {
        e.lastTry = frame;
        e.tex = api.LoadTexture(path, why);
    }
    if (e.tex) return e.tex.get();
    if (age >= SETTLE_FRAMES && reported.insert(path).second) api.OnTextureMissing(path, why);
    return nullptr;
}

}
