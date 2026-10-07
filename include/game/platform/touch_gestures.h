// 触屏手势：长按 = Esc（通用「取消/关闭」）。
//
// 动机：大地图/卡片栏/道具栏等对话框靠右键或 Esc 关闭，触屏没有右键，会关不掉。
// 但 SDL3 已把触摸自动转成左键鼠标事件（左键按下在 finger down 时就发出了），
// 此时再注入右键会与已开始的左键动作打架。改成长按注入 Esc —— 这些对话框
// 一律支持 Esc 退出，语义正好是「取消」，且不影响左键的正常点击流程。
//
// 用法：事件循环里 handle() 喂事件，每次 pumpEvents 末尾 tick() 检查是否到时。
#ifndef RICH4_PLATFORM_TOUCH_GESTURES_H
#define RICH4_PLATFORM_TOUCH_GESTURES_H

#include <SDL3/SDL.h>

namespace rich4::platform {

class TouchGestureTracker {
public:
    // 设定长按时长（毫秒）与移动容差（归一化坐标，约等于屏幕宽/高的百分比）。
    // [PORT 触屏实机] 550ms 太容易在瞄准（飞弹选目标/射气球）时误触发取消：
    //   提高到 750ms 并收紧容差，要求手指几乎完全静止才算长按。
    void configure(Uint64 holdMs = 750, float moveTolerance = 0.012f) {
        m_holdMs = holdMs;
        m_moveTol = moveTolerance;
    }

    // 喂入事件；只关心 FINGER_DOWN/MOTION/UP，其余原样不影响
    void handle(const SDL_Event& ev);

    // 每帧调用：长按到时返回 true（调用方注入一次 Esc）
    bool pollFired();

    bool active() const { return m_active; }

private:
    bool m_active = false;
    bool m_fired = false;
    Uint64 m_downAt = 0;
    float m_startX = 0.0f, m_startY = 0.0f;
    Uint64 m_holdMs = 550;
    float m_moveTol = 0.02f;
};

}  // namespace rich4::platform

#endif
