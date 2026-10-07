#pragma once

#include <string>

namespace rich4 {

// [NEW M4-F] rich4.ini 配置系统。
// 优先级：CLI 覆盖 > rich4.ini > RICH4.CFG > 内置默认。
// 红线：不往 RICH4.CFG（原版 72B 定长，exe 同读写）塞新字段——扩展配置一律本文件。
// 位置：writableDataDir(rich4.ini)（游戏目录可写则游戏目录，否则 SDL_GetPrefPath）；
//   缺失时自动生成带注释的默认文件（createIfMissing）。
struct GameConfig {
    // [display]
    // native|wide|free
    // [PORT 安卓] 默认 free：画布跟随屏幕 drawable 并按比例放大，横屏手机才铺满
    // （native 是 640x480 固定小窗，在手机上只是屏幕中央一小块）。
    // 用户可在 rich4.ini [display] preset 里显式改回。
#ifdef __ANDROID__
    std::string preset = "free";
#else
    std::string preset = "native";
#endif
    int canvasW = 0;               // 显式画布（仅 preset=native 生效；0=auto）
    int canvasH = 0;
    float uiScale = 1.0f;
    std::string filter = "linear"; // nearest|linear（呈现放大过滤）
    int vsync = 1;                 // 1 自适应 vsync / 0 关
    int fullscreen = 0;            // 1 全屏 / 0 窗口
    // [text]
    std::string fontRegular = "resources/Fonts/HarmonyOS_Sans_SC_Regular.ttf";
    std::string fontBold = "resources/Fonts/HarmonyOS_Sans_SC_Bold.ttf";
    float sizeScale = 1.30f; // 字号全局缩放（实机对照微调）
    // [PORT 手机适配①] 1.09 → 1.30：原 640x480 桌面字号在手机上偏小，统一放大
    // [paths]
    std::string mediaDir; // 空 = <gameDir>/Media → <gameDir>/../Media
    std::string logFile;  // 空 = rich4.log（cwd → fallbackDataDir）
    // [debug]
    int stats = 0; // 每 120 帧性能统计（同 --stats）
};

// 全局配置实例（默认构造 = 内置默认；loadConfig/setConfigValue 就地修改）
GameConfig& config();

// 载入 ini：解析 `[section]` + `key = value`（`#`/`;` 注释）；
//   createIfMissing 时文件缺失则写入带注释的默认模板。
//   返回 false 仅当"文件存在但无法读取/写入模板"。
bool loadConfig(const std::string& iniPath, bool createIfMissing);

// 设置单个键（`section.key` 或裸 key，大小写不敏感）；未知键返回 false。
bool setConfigValue(const std::string& key, const std::string& value);

// 默认 ini 模板文本（带注释）
std::string defaultIniText();

} // namespace rich4
