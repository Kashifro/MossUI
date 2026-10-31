#pragma once

#include "MossUI/core/Types.hpp"

#include <algorithm>
#include <array>

namespace MossUI {

// Texture sub-rectangle in 0..1 texture space.
struct Uv { float u0 = 0, v0 = 0, u1 = 1, v1 = 1; };

// Symmetric nine-slice: a TEX_W x TEX_H texture whose borders are BORDER_W / BORDER_H wide.
struct Slice { float texW, borderW, texH, borderH; };

struct QuadSpec { Rect dst; Uv uv; };

// The up-to-nine quads of a nine-slice drawn into `r`. Returns how many were written to `out`.
// Pure maths: every backend (and the nine-patch of any mod) shares it.
inline int NineSliceQuads(const Rect& r, const Slice& s, std::array<QuadSpec, 9>& out) {
    const float w = std::max(0.0f, r.x1 - r.x0), h = std::max(0.0f, r.y1 - r.y0);
    const float midW = std::max(0.0f, w - 2 * s.borderW), midH = std::max(0.0f, h - 2 * s.borderH);
    const float srcW = std::max(0.0f, s.texW - 2 * s.borderW), srcH = std::max(0.0f, s.texH - 2 * s.borderH);

    const float px[3] = {r.x0, r.x0 + s.borderW, r.x0 + w - s.borderW}, sx[3] = {s.borderW, midW, s.borderW};
    const float py[3] = {r.y0, r.y0 + s.borderH, r.y0 + h - s.borderH}, sy[3] = {s.borderH, midH, s.borderH};
    const float u[3] = {0, s.borderW / s.texW, (s.texW - s.borderW) / s.texW};
    const float uw[3] = {s.borderW / s.texW, srcW / s.texW, s.borderW / s.texW};
    const float v[3] = {0, s.borderH / s.texH, (s.texH - s.borderH) / s.texH};
    const float vh[3] = {s.borderH / s.texH, srcH / s.texH, s.borderH / s.texH};

    int n = 0;
    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 3; ++i) {
            if (sx[i] <= 0 || sy[j] <= 0) continue;
            out[static_cast<size_t>(n++)] = {{px[i], px[i] + sx[i], py[j], py[j] + sy[j]},
                                             {u[i], v[j], u[i] + uw[i], v[j] + vh[j]}};
        }
    return n;
}

}
