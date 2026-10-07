package xyz.a078868.rich4;

import android.content.pm.ActivityInfo;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

/**
 * 大富翁4 安卓入口：继承 SDLActivity，补上「沉浸式全屏」。
 *
 * 为什么需要这个类：
 * 1) 主题 NoTitleBar.Fullscreen 只隐藏状态栏，隐藏不了底部导航条（手势条）；
 * 2) 只在 onResume/onWindowFocusChanged 隐藏一次**不够** —— 游戏内打开对话框
 *    （彩票/股票/财神等）会触发窗口/焦点变化，系统把状态栏和导航条加回来，
 *    之后就一直不全屏（实机反馈"很多页面突然就不全屏了"）。
 *
 * 改为「守护式」：定时兜底重隐藏 + 监听系统栏可见性变化，任何时刻被加回来都
 * 能在几百毫秒内压回去。BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE 保留边缘滑动
 * 临时呼出（游戏常用）。
 */
public class Rich4Activity extends SDLActivity {

    private static final long HIDE_RETRY_MS = 700;
    private final Handler mHideHandler = new Handler(Looper.getMainLooper());

    private final Runnable mHideTick = new Runnable() {
        @Override
        public void run() {
            // [PORT 实机三轮] SDL 会按窗口/传感器自行 setRequestedOrientation，把方向改回
            //   竖屏 → 游戏画面被拉成竖长条（实机截图：主菜单整个竖着变形）。
            //   守护循环里每次一并重新锁定横屏，确保任何时候都锁住。
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
            hideSystemBars();
            mHideHandler.postDelayed(this, HIDE_RETRY_MS);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 游戏画面常亮（跟随原版 PC 版一直亮屏游玩的习惯）
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // [PORT 实机] 强制横屏，且不依赖系统"自动旋转"开关。
        //   manifest 的 sensorLandscape 在部分 ROM 上仍受自动旋转开关影响（实机反馈：
        //   必须手动打开自动旋转才全屏）。LANDSCAPE 为固定横屏，任何设置下都生效。
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        // [PORT 实机] 系统栏被加回来时立刻压回去（安卓 10 及以下的主通道）
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(
                    new View.OnSystemUiVisibilityChangeListener() {
                        @Override
                        public void onSystemUiVisibilityChange(int visibility) {
                            if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                                hideSystemBars();
                            }
                        }
                    });
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        // [PORT 实机] 守护式重隐藏：对话框/弹窗会把系统栏加回来，只靠 onResume 不够
        mHideHandler.removeCallbacks(mHideTick);
        mHideHandler.post(mHideTick);
    }

    @Override
    protected void onStop() {
        mHideHandler.removeCallbacks(mHideTick);
        super.onStop();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        // 焦点回来（下拉通知栏收起、切回应用、对话框开关）时重新进入沉浸式
        if (hasFocus) {
            hideSystemBars();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemBars();
    }

    private void hideSystemBars() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // 安卓 11+：WindowInsetsController 隐藏系统栏；
            // BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE = 从边缘滑动临时显示（游戏常用）
            WindowInsetsController c = getWindow().getInsetsController();
            if (c != null) {
                c.hide(WindowInsets.Type.systemBars());
                c.setSystemBarsBehavior(
                        WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            // 安卓 5.0 ~ 10：IMMERSIVE_STICKY 走老 API
            View v = getWindow().getDecorView();
            v.setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        }
    }
}
