#include "game/platform/touch_gestures.h"

namespace rich4::platform {

void TouchGestureTracker::handle(const SDL_Event& ev) {
    switch (ev.type) {
        case SDL_EVENT_FINGER_DOWN:
            m_active = true;
            m_fired = false;
            m_downAt = SDL_GetTicks();
            m_startX = ev.tfinger.x;
            m_startY = ev.tfinger.y;
            break;

        case SDL_EVENT_FINGER_MOTION:
            if (!m_active) break;
            {
                const float dx = ev.tfinger.x - m_startX;
                const float dy = ev.tfinger.y - m_startY;
                // 移动超过容差视为拖动/滑动，不是长按
                if (dx * dx + dy * dy > m_moveTol * m_moveTol) {
                    m_active = false;
                }
            }
            break;

        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_CANCELED:
            m_active = false;
            break;

        default:
            break;
    }
}

bool TouchGestureTracker::pollFired() {
    if (!m_active || m_fired) return false;
    if (SDL_GetTicks() - m_downAt < m_holdMs) return false;
    m_fired = true;
    m_active = false;
    return true;
}

}  // namespace rich4::platform
