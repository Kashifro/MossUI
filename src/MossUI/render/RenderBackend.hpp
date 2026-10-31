#pragma once

#include "MossUI/core/Backend.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace MossUI {

// The few calls a game renderer has to provide. RenderBackend turns them into a full MossUI::Backend: texture cache with
// settle frames and retry, clip stack, missing-texture reporting. Quads are batched by the game until FlushQuads.
class RenderApi {
public:
    using Texture = std::shared_ptr<void>;   // opaque to MossUI; DrawQuad gets the raw pointer back

    virtual ~RenderApi() = default;

    virtual void FillRect(const Rect& r, const Color& c, float alpha) = 0;

    // Null = the texture is missing or not ready yet. `why` is only used for the one-time log line.
    virtual Texture LoadTexture(const std::string& path, std::string& why) = 0;
    virtual void DrawQuad(const void* texture, const Rect& dst, const Uv& uv) = 0;
    virtual void FlushQuads(const Color& tint) = 0;

    virtual void SetClip(const Rect* clip) = 0;   // null = whole screen, scissor off

    virtual void DrawText(const TextRun& run) = 0;
    virtual void FlushText() = 0;
    virtual float TextWidth(const std::string& s) = 0;

    virtual void OnTextureMissing(const std::string& path, const std::string& why) { (void)path; (void)why; }
    virtual void OnAccessibility(const AccessibilityInfo&) {}
};

class RenderBackend final : public Backend {
public:
    explicit RenderBackend(RenderApi& api) : api(api) {}

    // Per render pass. The first frames after a texture is first asked for are "settling": the game may still be
    // loading it, so loads are retried instead of cached as missing.
    void BeginFrame();
    void EndFrame();
    void ResetTextures();   // call when a screen opens (resource packs may have changed)

    void Fill(const Rect& r, const Color& c, float alpha) override { api.FillRect(r, c, alpha); }
    bool Quad(const Rect& r, const std::string& path, const Uv& uv) override;
    void FlushQuads(const Color& tint) override { api.FlushQuads(tint); }
    void Preload(const std::string& path) override { Resolve(path); }
    void PushClip(const Rect& r) override;
    void PopClip() override;
    void Text(const TextRun& run) override { api.DrawText(run); }
    void FlushText() override { api.FlushText(); }
    float TextWidth(const std::string& s) override { return api.TextWidth(s); }
    void Accessibility(const AccessibilityInfo& info) override { api.OnAccessibility(info); }

    static constexpr int SETTLE_FRAMES = 4;
    static constexpr int RETRY_FRAMES = 30;    // a texture still missing after settling is retried this often

private:
    struct Entry { RenderApi::Texture tex; int born = 0, lastTry = 0; };
    const void* Resolve(const std::string& path);

    RenderApi& api;
    std::vector<Rect> clips;
    std::map<std::string, Entry> cache;
    std::set<std::string> reported;
    int frame = 0;
};

}
