// 安卓 assets 提取到内部存储（实现见 android_assets.h 注释）。
// 非安卓平台为空操作，保持桌面构建不受影响。
#include <cstddef>
#include "game/platform/android_assets.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifdef __ANDROID__
#include <SDL3/SDL.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace rich4::platform {

#ifndef __ANDROID__

bool ensureAndroidAssets() {
    return true;  // 桌面：素材本就在文件系统
}

#else

namespace {

// assets 下要提取的清单（相对 assets/ 根）。与 android/app/src/main/assets/ 对应。
// 走清单而不是目录遍历：SDL3 无列出 APK assets 的跨版本 API，清单可随包生成。
const char* const kManifest[] = {
    // —— 游戏数据（正版 Rich4 包的 mkf）——
    "game/Data.mkf",
    "game/Effect.mkf",
    "game/help.mkf",
    "game/jump.mkf",
    "game/map.mkf",
    "game/Panel.mkf",
    "game/Speaking.mkf",
    // —— 背景音乐（MIDI，实际文件名两种前缀并存）——
    "game/midi01.mid", "game/midi02.mid", "game/midi03.mid", "game/midi04.mid",
    "game/midi05.mid", "game/midi06.mid", "game/midi07.mid", "game/midi08.mid",
    "game/midi09.mid", "game/midi10.mid", "game/midi11.mid", "game/midi12.mid",
    "game/midi13.mid", "game/midi14-1.mid", "game/midi14-2.mid",
    "game/midi15.mid", "game/midi16.mid",
    "game/Rich08.mid", "game/Rich16.mid", "game/Rich17.mid", "game/Rich18.mid",
    "game/Rich19.mid", "game/Rich20.mid", "game/Rich21.mid", "game/Rich22.mid",
    // —— 配置模板（游戏本体会按需生成 rich4.ini）——
    "game/RICH4.CFG",
    // —— 字体（文本渲染）——
    "Fonts/HarmonyOS_Sans_SC_Regular.ttf",
    "Fonts/HarmonyOS_Sans_SC_Bold.ttf",
    // —— 背景/场景音乐（OGG；解码器只认 Vorbis，由 MIDI 合成）——
    //   track02..09 = 可选背景曲 8 首（playMusic）
    //   track10..26 = 场景音乐 17 首（playSceneMusic：入狱/住院/节日等）
    "Music/track02.ogg", "Music/track03.ogg", "Music/track04.ogg", "Music/track05.ogg",
    "Music/track06.ogg", "Music/track07.ogg", "Music/track08.ogg", "Music/track09.ogg",
    "Music/track10.ogg", "Music/track11.ogg", "Music/track12.ogg", "Music/track13.ogg",
    "Music/track14.ogg", "Music/track15.ogg", "Music/track16.ogg", "Music/track17.ogg",
    "Music/track18.ogg", "Music/track19.ogg", "Music/track20.ogg", "Music/track21.ogg",
    "Music/track22.ogg", "Music/track23.ogg", "Music/track24.ogg", "Music/track25.ogg",
    "Music/track26.ogg",
};

bool makeDirs(const std::string& path) {
    std::string acc;
    for (size_t i = 0; i < path.size(); ++i) {
        acc.push_back(path[i]);
        if (path[i] == '/' && acc.size() > 1) {
            ::mkdir(acc.c_str(), 0755);
        }
    }
    return ::mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

bool extractOne(const char* assetPath, const std::string& dstPath) {
    // SDL3 在安卓上：SDL_IOFromFile 先试文件系统，失败则回退 AssetManager
    // （src/io/SDL_iostream.c → Android_JNI_FileOpen → AAssetManager_open），
    // 因此用它读 APK 内的 assets；桌面平台读本地同名文件，行为一致。
    SDL_IOStream* in = SDL_IOFromFile(assetPath, "rb");
    if (!in) {
        SDL_Log("[rich4] assets 缺失: %s", assetPath);
        return false;
    }
    std::FILE* out = std::fopen(dstPath.c_str(), "wb");
    if (!out) {
        SDL_Log("[rich4] 无法写入 %s", dstPath.c_str());
        SDL_CloseIO(in);
        return false;
    }
    char buf[256 * 1024];
    size_t n;
    while ((n = SDL_ReadIO(in, buf, sizeof(buf))) > 0) {
        if (std::fwrite(buf, 1, n, out) != n) {
            SDL_Log("[rich4] 写入失败 %s", dstPath.c_str());
            std::fclose(out);
            SDL_CloseIO(in);
            return false;
        }
    }
    std::fclose(out);
    SDL_CloseIO(in);
    return true;
}

}  // namespace

bool ensureAndroidAssets() {
    const char* base = SDL_GetAndroidInternalStoragePath();
    if (!base) {
        SDL_Log("[rich4] 取不到内部存储路径");
        return false;
    }
    const std::string root(base);

    // 目标结构与桌面一致，让 resources/... 相对路径照常解析
    const std::string gameDir = root + "/resources/MultiverseJourney";
    const std::string fontDir = root + "/resources/Fonts";
    const std::string musicDir = gameDir + "/Media/Music";  // setMusicDir(mediaDir + "/Music")
    makeDirs(gameDir);
    makeDirs(fontDir);
    makeDirs(musicDir);

    // 逐文件增量提取：缺哪个补哪个。
    // 不能只看 Data.mkf 就跳过——覆盖安装升级时新版本新增的素材（如音乐）
    // 会永远解不出来（4.0 装在 3.0 上没音乐就是这个原因）。
    SDL_Log("[rich4] 提取素材到 %s（增量，缺则补）", root.c_str());
    int ok = 0, skip = 0, fail = 0;
    for (const char* rel : kManifest) {
        // 目标结构与桌面一致：assets/game/* → resources/MultiverseJourney/*，
        // assets/Music/* → resources/MultiverseJourney/Media/Music/*（setMusicDir 期望的位置），
        // 其余（Fonts/*）保持 resources/Fonts/*，让原有相对路径照常解析。
        std::string dst;
        if (std::strncmp(rel, "game/", 5) == 0) {
            dst = gameDir + "/" + (rel + 5);
        } else if (std::strncmp(rel, "Music/", 6) == 0) {
            dst = musicDir + "/" + (rel + 6);
        } else {
            dst = root + "/resources/" + rel;
        }
        // 已存在且非空 → 跳过（升级只补新增的，不重拷几百 MB）
        struct stat st{};
        if (::stat(dst.c_str(), &st) == 0 && st.st_size > 0) {
            ++skip;
            continue;
        }
        if (extractOne(rel, dst)) {
            ++ok;
        } else {
            ++fail;
        }
    }
    SDL_Log("[rich4] 提取完成：新增 %d，已有 %d，失败 %d", ok, skip, fail);
    ::chdir(root.c_str());
    return fail == 0;
}

#endif

}  // namespace rich4::platform
