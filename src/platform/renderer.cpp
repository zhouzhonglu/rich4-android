#include "game/platform/renderer.h"

#include "game/core/log.h"

namespace rich4 {

Renderer::~Renderer() { destroy(); }

bool Renderer::create(SDL_Window* window, int logicalWidth, int logicalHeight, bool vsync) {
    m_renderer = SDL_CreateRenderer(window, nullptr);
    if (!m_renderer) {
        RICH4_LOGE("SDL_CreateRenderer failed: %s", SDL_GetError());
        return false;
    }
    m_logicalWidth = logicalWidth;
    m_logicalHeight = logicalHeight;
    setWidescreen(true);
    if (vsync) {
        // [NEW M4-D 实机] 自适应 vsync：绘制超过一帧（16.7ms）时允许立即呈现（不锁 30fps）。
        //   严格 vsync 下"绘制 17ms"会等下一个 vblank → 半速抖动（放大窗口后动画观感变慢的主因之一）。
        if (!SDL_SetRenderVSync(m_renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
            SDL_SetRenderVSync(m_renderer, 1); // 驱动不支持自适应 → 回退严格 vsync
        }
    }
    return true;
}

void Renderer::destroy() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
}

void Renderer::setLogicalSize(int width, int height) {
    // [NEW M4-A2] 逻辑呈现随画布尺寸（free preset resize）
    if (width <= 0 || height <= 0) {
        return;
    }
    m_logicalWidth = width;
    m_logicalHeight = height;
    setWidescreen(true);
}

void Renderer::setWidescreen(bool enabled) {
    // [PORT 触屏实机] LETTERBOX 会保持画布比例并在**上下/左右留黑边**（实机截图：
    //   选人界面四周都是黑边，看起来"没全屏"）。STRETCH 让画布铺满整个窗口，
    //   宽屏手机上界面随之加宽，无黑边。原版 640x480 窗口比例一致时两者等价。
    SDL_SetRenderLogicalPresentation(
        m_renderer, m_logicalWidth, m_logicalHeight,
        enabled ? SDL_LOGICAL_PRESENTATION_STRETCH : SDL_LOGICAL_PRESENTATION_DISABLED);
}

void Renderer::beginFrame() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
}

void Renderer::present() { SDL_RenderPresent(m_renderer); }

} // namespace rich4
