// 安卓：APK assets 不是文件系统，游戏到处用 fopen/ifstream 读文件。
// 这里在 SDL_main 起手把 assets 提取到内部存储并 chdir，让原有相对路径照常工作：
//   <internal>/resources/MultiverseJourney/*.mkf|*.mid|*.CFG   ← assets/game/*
//   <internal>/resources/Fonts/*.ttf                           ← assets/Fonts/*
// 首次启动提取（约 270MB，之后跳过）。
#ifndef RICH4_PLATFORM_ANDROID_ASSETS_H
#define RICH4_PLATFORM_ANDROID_ASSETS_H

namespace rich4::platform {

// 幂等：已提取则直接返回 true；失败返回 false（游戏仍可启动，但会缺素材）。
// 完成后 CWD 为内部存储根，resources/... 相对路径可正常解析。
bool ensureAndroidAssets();

}  // namespace rich4::platform

#endif
