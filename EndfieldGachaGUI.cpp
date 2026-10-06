/*
终末地抽卡模拟器 图形界面版
制作者：爱写作业的好汉（https://space.bilibili.com/3546375050496178）
说明：本文件复用 EndfieldGacha.cpp 中的抽卡内核（卡池数据、保底逻辑、批量寻访接口），
      在其之上提供 Win32 图形界面；控制台版仍可单独编译使用。
编译：g++ -O2 -std=gnu++17 -mwindows -o EndfieldGachaGUI.exe EndfieldGachaGUI.cpp -static -lgdiplus -lshell32 -lgdi32 -luser32
注：更新请搜索注释“//更新修改”
*/
#define main EndfieldGachaConsoleMain
#include "EndfieldGacha.cpp"
#undef main

#include <windows.h>
#include <gdiplus.h>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

//控件ID
#define IDC_VERSION    1001
#define IDC_BANNER     1002
#define IDC_INFO       1003
#define IDC_BIG        1004
#define IDC_SMALL      1005
#define IDC_FIVE       1006
#define IDC_COUNT      1007
#define IDC_DRAW1      1008
#define IDC_DRAW10     1009
#define IDC_DRAWCOUNT  1010
#define IDC_DECLARE    1011
#define IDC_STORE      1012
#define IDC_ART        1013
#define IDC_RESULT     1014
#define IDC_STATUS     1015
#define IDC_ARTLIST    1016
#define IDC_ARTOPEN    1017
#define IDC_ARTWIKI    1018
#define IDC_ARTCLOSE   1019
#define IDC_TEXTTEXT   1020
#define IDC_TEXTCLOSE  1021
#define IDC_ARTNAME    1022

//窗口尺寸（逻辑像素）
#define WIN_W   1170
#define WIN_H   750
#define ART_W   1000
#define ART_H   640
#define TEXT_W  780
#define TEXT_H  560

HINSTANCE g_hInst;
HWND g_hMain, g_hArt, g_hText;
HWND hVersion, hBanner, hInfo, hBig, hSmall, hFive, hCount, hResult, hStatus;
HFONT g_hFont;
double g_scale = 1.0;
int g_bannerIdx[32];    //当前版本下的卡池（对应BANNERS下标）
int g_bannerCount = 0;
int g_curBanner = 0;    //当前卡池在BANNERS中的下标
int g_artSel = -1;      //图鉴当前选中角色
Gdiplus::Image* g_artImage = NULL;
ULONG_PTR g_gdiToken = 0;

//更新修改
//版本/卡池表（与控制台版AskSection/AskVersion保持一致）
struct BannerRow
{
    int version;
    int section;
    const wchar_t* vname;
    const wchar_t* name;
    const wchar_t* up;
    const wchar_t* date;
};
const BannerRow BANNERS[] = {
    {1,   1, L"1.0 零号委托",       L"熔火灼痕",         L"莱万汀",                        L"2026.1.22~2026.2.7"},
    {1,   2, L"1.0 零号委托",       L"轻飘飘的信使",     L"洁尔佩塔",                      L"2026.2.7~2026.2.24"},
    {1,   3, L"1.0 零号委托",       L"热烈色彩",         L"伊冯",                          L"2026.2.24~2026.3.12"},
    {2,   4, L"1.1 新潮起，故渊离", L"河流的女儿",       L"汤汤",                          L"2026.3.12~2026.3.29"},
    {2,   5, L"1.1 新潮起，故渊离", L"狼珀",             L"洛茜",                          L"2026.3.29~2026.4.17"},
    {3,   6, L"1.2 春晓时",         L"春雷动，万物生",   L"庄方宜",                        L"2026.4.17~2026.5.22"},
    {3,   7, L"1.2 春晓时",         L"辉光庆典（特殊）", L"莱万汀/洁尔佩塔/艾尔黛拉/骏卫", L"2026.5.14~2026.6.5"},
    {4,   8, L"1.3 寻遗散记",       L"拳出无悔",         L"弭弗",                          L"2026.6.5~2026.6.26"},
    {4,   9, L"1.3 寻遗散记",       L"逐罪者",           L"卡缪",                          L"2026.6.26~2026.7.16"},
    {5,  10, L"1.4 向渊行",         L"临渊望北",         L"诀",                            L"2026.7.16~2026.8.9"},
    {5,  11, L"1.4 向渊行",         L"晨星于此闪耀",     L"梨诺",                          L"2026.8.9~2026.9.2"},
    {6,  12, L"1.5 雪凇幽梦",       L"冬猎",             L"提弗洛斯",                      L"2026.9.2~2026.9.30"},
    {6,  13, L"1.5 雪凇幽梦",       L"绚丽异彩（重构）", L"伊冯",                          L"2026.9.24~2026.10.14"},
    {7, 999, L"常驻池",             L"基础寻访",         L"（无UP角色）",                  L"永久开放"}
};
const int BANNER_COUNT = (int)(sizeof(BANNERS) / sizeof(BANNERS[0]));

//更新修改
//按DPI缩放逻辑坐标
int S(int v)
{
    return (int)(v * g_scale + 0.5);
}

//内核字符串为UTF-8，转成宽字符显示
wstring U8(const string& s)
{
    if (s.empty())
        return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (n <= 0)
        return L"";
    wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

//百分比文本
wstring Pct(long long part, long long total)
{
    wchar_t buf[32];
    if (total <= 0)
        return L"0.00%";
    swprintf(buf, 32, L"%.2f%%", (double)part * 100.0 / (double)total);
    return buf;
}

//创建子控件
HWND Mk(const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id, HWND parent)
{
    HWND c = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, S(x), S(y), S(w), S(h),
        parent, (HMENU)(INT_PTR)id, g_hInst, NULL);
    if (g_hFont)
        SendMessageW(c, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    return c;
}

//设置编辑框整数
void SetEditInt(HWND h, int v)
{
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", v);
    SetWindowTextW(h, buf);
}

//取编辑框整数（非法输入返回-1）
int GetEditInt(HWND h)
{
    wchar_t buf[32] = {0};
    GetWindowTextW(h, buf, 32);
    if (buf[0] == 0)
        return -1;
    for (int i = 0; buf[i]; i++)
    {
        if (buf[i] < L'0' || buf[i] > L'9')
            return -1;
    }
    return _wtoi(buf);
}

//已获得各星级总数
void CountOwned(int& c6, int& c5, int& c4)
{
    c6 = c5 = c4 = 0;
    for (const auto& name : ALL_STARS6)
    {
        auto it = owned.find(name);
        if (it != owned.end())
            c6 += it->second;
    }
    for (const auto& name : ALL_STARS5)
    {
        auto it = owned.find(name);
        if (it != owned.end())
            c5 += it->second;
    }
    for (const auto& name : ALL_STARS4)
    {
        auto it = owned.find(name);
        if (it != owned.end())
            c4 += it->second;
    }
}

//嵌晶玉折算的衍质源石（1抽500嵌晶玉，1衍质源石=75嵌晶玉，向上取整）
long long CalcOriginium()
{
    if (g_count % 3 == 0)
        return (long long)g_count * 20 / 3;
    return (long long)g_count * 20 / 3 + 1;
}

//更新修改
//刷新左侧状态栏
void UpdateStatus()
{
    int c6, c5, c4;
    CountOwned(c6, c5, c4);
    long long quota = CalcWeaponQuota();
    wstringstream ss;
    if (stars6_big_baseline > 0)
        ss << L"剩余大保底：" << stars6_big_baseline << L" 抽必出当期UP\n";
    else
        ss << L"剩余大保底：本期角色井已消耗（或无角色井）\n";
    ss << L"剩余小保底：" << stars6_small_baseline << L" 抽必出6星\n";
    ss << L"剩余5星保底：" << stars5_baseline << L" 抽必出5星及以上\n";
    ss << L"─────────────────────\n";
    ss << L"本池累计寻访：" << banner_pulls << L" 抽\n";
    ss << L"累计寻访：" << g_count << L" 抽\n";
    ss << L"6星：" << c6 << L"（" << Pct(c6, g_count) << L"）  5星：" << c5 << L"（" << Pct(c5, g_count) << L"）  4星：" << c4 << L"（" << Pct(c4, g_count) << L"）\n";
    ss << L"嵌晶玉：" << (long long)g_count * 500 << L"    衍质源石：" << CalcOriginium() << L"\n";
    ss << L"武库配额：" << quota << L"（约 " << quota / 1980 << L" 次申领 / " << quota / 1980 * 10 << L" 抽）\n";
    ss << L"寻访情报书：" << intel_book << L"    流光庆时调用凭证：" << select_voucher;
    SetWindowTextW(hStatus, ss.str().c_str());
}

//卡池信息文本
wstring BuildBannerInfo()
{
    const BannerRow& b = BANNERS[g_curBanner];
    wstringstream ss;
    ss << L"卡池：" << b.name << L"（" << b.date << L"）\r\n";
    ss << L"UP角色：" << b.up << L"\r\n";
    ss << L"6星池：";
    for (int i = 0; i < stars6_count; i++)
    {
        if (i == up_stars6_count && up_stars6_count > 0)
            ss << L"｜非UP：";
        ss << U8(stars6[i]) << L" ";
    }
    ss << L"\r\n5星池：";
    for (int i = 0; i < stars5_count; i++)
        ss << U8(stars5[i]) << L" ";
    return ss.str();
}

//更新修改
//切换卡池：重置本池累计与里程碑，按规则设置保底
void RefreshBanner()
{
    const BannerRow& b = BANNERS[g_curBanner];
    SetSection(b.section);
    banner_pulls = 0;
    token_special = 0;
    milestone_30 = milestone_60 = milestone_90 = milestone_120 = false;
    memset(token, 0, sizeof(token));

    bool hasBig = (up_stars6_count > 0 && banner_type != 2);
    if (hasBig)
    {
        SetEditInt(hBig, 120);
        EnableWindow(hBig, TRUE);
        stars6_big_baseline = 120;
        up_first = true;
    }
    else
    {
        SetEditInt(hBig, 0);
        EnableWindow(hBig, FALSE);
        stars6_big_baseline = 0;
        up_first = false;
    }
    //小保底与5星保底跨池继承，无有效值时用默认值
    int v = GetEditInt(hSmall);
    if (v < 1 || v > 80)
    {
        SetEditInt(hSmall, 80);
        stars6_small_baseline = 80;
    }
    else
        stars6_small_baseline = v;
    v = GetEditInt(hFive);
    if (v < 1 || v > 10)
    {
        SetEditInt(hFive, 10);
        stars5_baseline = 10;
    }
    else
        stars5_baseline = v;
    v = GetEditInt(hCount);
    if (v < 1 || v > 10000)
        SetEditInt(hCount, 10);

    SetWindowTextW(hInfo, BuildBannerInfo().c_str());
    UpdateStatus();
}

//更新修改
//把一次寻访的输出写入结果列表
void AddResultLine(const GachaItem& it)
{
    wstring line = U8(it.text);
    while (!line.empty() && (line[0] == L'\n' || line[0] == L'\r'))
        line.erase(line.begin());
    int idx = (int)SendMessageW(hResult, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    if (idx >= 0)
        SendMessageW(hResult, LB_SETITEMDATA, idx, (it.type == 2) ? 0 : it.stars);
    //最多保留最近2000行，避免长时间抽取占用过多内存
    while (SendMessageW(hResult, LB_GETCOUNT, 0, 0) > 2000)
        SendMessageW(hResult, LB_DELETESTRING, 0, 0);
    int cnt = (int)SendMessageW(hResult, LB_GETCOUNT, 0, 0);
    if (cnt > 0)
        SendMessageW(hResult, LB_SETTOPINDEX, cnt - 1, 0);
}

//更新修改
//执行一次寻访
void DoPull(int times)
{
    bool hasBig = (up_stars6_count > 0 && banner_type != 2);
    int big = 0;
    if (hasBig)
    {
        big = GetEditInt(hBig);
        if (big < 0 || big > 120)
        {
            MessageBoxW(g_hMain, L"剩余大保底需填写 0~120 的整数（0 表示本期角色井已消耗）。", L"输入有误", MB_ICONWARNING);
            return;
        }
    }
    int small = GetEditInt(hSmall);
    if (small < 1 || small > 80)
    {
        MessageBoxW(g_hMain, L"剩余小保底需填写 1~80 的整数。", L"输入有误", MB_ICONWARNING);
        return;
    }
    int five = GetEditInt(hFive);
    if (five < 1 || five > 10)
    {
        MessageBoxW(g_hMain, L"剩余5星保底需填写 1~10 的整数。", L"输入有误", MB_ICONWARNING);
        return;
    }
    stars6_big_baseline = big;
    up_first = (big > 0);
    stars6_small_baseline = small;
    stars5_baseline = five;

    vector<GachaItem> items = DoBatchPull(times);
    for (size_t i = 0; i < items.size(); i++)
        AddResultLine(items[i]);

    //回填保底剩余，便于连续抽取
    if (hasBig)
        SetEditInt(hBig, stars6_big_baseline);
    SetEditInt(hSmall, stars6_small_baseline);
    SetEditInt(hFive, stars5_baseline);
    UpdateStatus();
}

//更新修改
//仓库统计文本
wstring BuildStoreText()
{
    int c6, c5, c4;
    CountOwned(c6, c5, c4);
    long long quota = CalcWeaponQuota();
    wstringstream ss;
    if (owned.empty())
        ss << L"仓库为空，快去抽卡吧！\r\n";
    else
    {
        ss << L"当前角色获取统计（累计寻访 " << g_count << L" 抽）：\r\n";
        ss << L"【6星】\r\n";
        for (const auto& name : ALL_STARS6)
        {
            auto it = owned.find(name);
            if (it != owned.end())
                ss << L"  " << U8(name) << L" × " << it->second << L"    出率：" << Pct(it->second, g_count) << L"\r\n";
        }
        ss << L"【5星】\r\n";
        for (const auto& name : ALL_STARS5)
        {
            auto it = owned.find(name);
            if (it != owned.end())
                ss << L"  " << U8(name) << L" × " << it->second << L"    出率：" << Pct(it->second, g_count) << L"\r\n";
        }
        ss << L"【4星】\r\n";
        for (const auto& name : ALL_STARS4)
        {
            auto it = owned.find(name);
            if (it != owned.end())
                ss << L"  " << U8(name) << L" × " << it->second << L"    出率：" << Pct(it->second, g_count) << L"\r\n";
        }
        ss << L"\r\n综合出率：6星 " << Pct(c6, g_count) << L"，5星 " << Pct(c5, g_count) << L"，4星 " << Pct(c4, g_count) << L"\r\n";
    }
    ss << L"\r\n─────────────────────────\r\n";
    ss << L"目前已抽：" << g_count << L" 抽\r\n";
    ss << L"相当于嵌晶玉 × " << (long long)g_count * 500 << L"\r\n";
    ss << L"或相当于衍质源石 × " << CalcOriginium() << L"\r\n";
    ss << L"共计可获得武库配额 × " << quota << L"（相当于 " << quota / 1980 << L" 次申领，武器池 " << quota / 1980 * 10 << L" 抽）\r\n";
    ss << L"【寻访情报书】 × " << intel_book << L"（下一次特许寻访开启后自动转化为10张专有寻访凭证）\r\n";
    ss << L"【流光庆时调用凭证】 × " << select_voucher << L"（可从莱万汀/洁尔佩塔/艾尔黛拉/骏卫中自选一名干员获取）\r\n";
    return ss.str();
}

//声明文本
const wchar_t* DECLARE_TEXT =
    L"本软件制作者：爱写作业的好汉（https://space.bilibili.com/3546375050496178）\r\n"
    L"关于《明日方舟：终末地》的一切权利归鹰角网络所有，侵权必删！\r\n"
    L"本工具：卡池信息来源于 https://end.canmoe.com/zh-CN/banner-calendar\r\n"
    L"干员介绍连接至 https://www.fz.wiki/wiki/干员\r\n"
    L"本工具无任何版权，侵权必删！\r\n"
    L"\r\n"
    L"【抽卡规则摘要】\r\n"
    L"· 基础出率：6星 0.8%（其中UP占50%）、5星 8%、4星 91.2%\r\n"
    L"· 6星保底：80抽必得；距上次6星满66抽起每抽+5%（第66抽5.8%，第79抽70.8%）\r\n"
    L"· 5星保底：10抽内必得5星及以上，出5星或6星均清空计数（跨池继承）\r\n"
    L"· 角色井：特许/重构寻访单池120抽内必得当期UP，获得后即消耗，不跨池继承\r\n"
    L"· 信物赠礼：每累计寻访240次获得当期UP干员信物×1\r\n"
    L"· 里程碑：特许寻访30抽送加急十连、60抽送寻访情报书；重构寻访30/60/90抽各送加急十连\r\n"
    L"· 特殊寻访（辉光庆典）：6星池仅4名UP角色，保底独立；60抽基础寻访凭证、120抽自选调用凭证\r\n"
    L"· 武库配额：每获得1名干员得6星2000、5星200、4星20；1980配额可申领10把武器\r\n"
    L"· 货币：1抽=500嵌晶玉，1衍质源石=75嵌晶玉";

//更新修改
//加载立绘
void LoadArtImage(int idx)
{
    if (g_artImage)
    {
        delete g_artImage;
        g_artImage = NULL;
    }
    if (idx < 0 || idx >= (int)ALL_CHARACTERS.size())
        return;
    wstring path = GetImagePathW(U8(ENGLISH[idx]));
    Gdiplus::Image* img = Gdiplus::Image::FromFile(path.c_str());
    if (img && img->GetLastStatus() == Gdiplus::Ok)
        g_artImage = img;
    else
    {
        delete img;
        g_artImage = NULL;
    }
}

//在指定区域绘制立绘（等比缩放居中）
void PaintArtImage(HDC hdc)
{
    RECT rc;
    rc.left = S(324); rc.top = S(40); rc.right = S(984); rc.bottom = S(556);
    HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);
    FrameRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
    if (!g_artImage)
    {
        RECT t = rc;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(120, 120, 120));
        DrawTextW(hdc, L"（未找到该角色的立绘图片，请确认 CharacterImages 文件夹与 exe 放在同一目录）", -1, &t,
            DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        return;
    }
    int iw = (int)g_artImage->GetWidth();
    int ih = (int)g_artImage->GetHeight();
    int bw = (rc.right - rc.left) - 8;
    int bh = (rc.bottom - rc.top) - 8;
    if (iw <= 0 || ih <= 0 || bw <= 0 || bh <= 0)
        return;
    double k1 = (double)bw / (double)iw;
    double k2 = (double)bh / (double)ih;
    double k = (k1 < k2) ? k1 : k2;
    int dw = (int)(iw * k);
    int dh = (int)(ih * k);
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.DrawImage(g_artImage, rc.left + 4 + (bw - dw) / 2, rc.top + 4 + (bh - dh) / 2, dw, dh);
}

//更新修改
//绘制自绘列表框的条目（按星级着色）
void DrawOwnerItem(const DRAWITEMSTRUCT* dis)
{
    if (dis->itemID == (UINT)-1)
        return;
    wchar_t text[512] = {0};
    SendMessageW(dis->hwndItem, LB_GETTEXT, dis->itemID, (LPARAM)text);
    int stars = (int)SendMessageW(dis->hwndItem, LB_GETITEMDATA, dis->itemID, 0);
    COLORREF col;
    if (stars == 6)
        col = RGB(200, 110, 0);       // 暗黄色(橙色)
    else if (stars == 5)
        col = RGB(180, 140, 0);       // 亮黄色(金色)
    else if (stars == 4)
        col = RGB(130, 60, 190);      // 亮紫色
    else
        col = RGB(60, 90, 160);       // 文字消息
    if (dis->itemState & ODS_SELECTED)
        FillRect(dis->hDC, &dis->rcItem, (HBRUSH)(COLOR_HIGHLIGHT + 1));
    else
        FillRect(dis->hDC, &dis->rcItem, (HBRUSH)(COLOR_WINDOW + 1));
    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, (dis->itemState & ODS_SELECTED) ? RGB(255, 255, 255) : col);
    RECT r = dis->rcItem;
    r.left += S(4);
    DrawTextW(dis->hDC, text, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
}

//显示文本窗口（声明、仓库统计）
void ShowTextWindow(const wchar_t* title, const wstring& text);
//显示角色图鉴窗口
void ShowArtWindow();

//文本窗口过程
LRESULT CALLBACK TextProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static wstring* buf = NULL;
    switch (msg)
    {
    case WM_CREATE:
        Mk(L"EDIT", L"", WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_BORDER,
            12, 12, TEXT_W - 36, TEXT_H - 76, IDC_TEXTTEXT, hwnd);
        Mk(L"BUTTON", L"关闭", BS_PUSHBUTTON, 12, TEXT_H - 54, 110, 32, IDC_TEXTCLOSE, hwnd);
        if (buf)
        {
            SetWindowTextW(GetDlgItem(hwnd, IDC_TEXTTEXT), buf->c_str());
            delete buf;
            buf = NULL;
        }
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_TEXTCLOSE)
            DestroyWindow(hwnd);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_hText = NULL;
        return 0;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

//角色图鉴窗口过程
LRESULT CALLBACK ArtProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        int n6 = (int)ALL_STARS6.size() + 2;         //含管理员-男/女
        int n5 = n6 + (int)ALL_STARS5.size();
        HWND list = Mk(L"LISTBOX", L"", LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_BORDER,
            12, 12, 300, ART_H - 76, IDC_ARTLIST, hwnd);
        for (int i = 0; i < (int)ALL_CHARACTERS.size(); i++)
        {
            int stars = (i < n6) ? 6 : ((i < n5) ? 5 : 4);
            wstringstream ss;
            ss << (i + 1) << L". " << U8(ALL_CHARACTERS[i]) << L"（" << stars << L"星）";
            int idx = (int)SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
            if (idx >= 0)
                SendMessageW(list, LB_SETITEMDATA, idx, stars);
        }
        SendMessageW(list, LB_SETITEMHEIGHT, 0, S(22));
        Mk(L"STATIC", L"请选择角色", SS_LEFT, 324, 12, 660, 22, IDC_ARTNAME, hwnd);
        Mk(L"BUTTON", L"用系统看图打开", BS_PUSHBUTTON, 324, ART_H - 54, 180, 32, IDC_ARTOPEN, hwnd);
        Mk(L"BUTTON", L"打开发布页(Wiki)", BS_PUSHBUTTON, 516, ART_H - 54, 180, 32, IDC_ARTWIKI, hwnd);
        Mk(L"BUTTON", L"关闭", BS_PUSHBUTTON, 708, ART_H - 54, 110, 32, IDC_ARTCLOSE, hwnd);
        return 0;
    }
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        if (id == IDC_ARTCLOSE)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        if (id == IDC_ARTLIST && HIWORD(wParam) == LBN_SELCHANGE)
        {
            int sel = (int)SendMessageW(GetDlgItem(hwnd, IDC_ARTLIST), LB_GETCURSEL, 0, 0);
            if (sel >= 0)
            {
                g_artSel = sel;
                LoadArtImage(g_artSel);
                wstringstream ss;
                ss << (sel + 1) << L". " << U8(ALL_CHARACTERS[sel]);
                SetWindowTextW(GetDlgItem(hwnd, IDC_ARTNAME), ss.str().c_str());
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        if ((id == IDC_ARTOPEN || id == IDC_ARTLIST) && g_artSel >= 0)
        {
            if (id == IDC_ARTLIST && HIWORD(wParam) != LBN_DBLCLK)
                return 0;
            wstring path = GetImagePathW(U8(ENGLISH[g_artSel]));
            DWORD attr = GetFileAttributesW(path.c_str());
            if (attr == INVALID_FILE_ATTRIBUTES)
            {
                MessageBoxW(hwnd, (L"图片不存在：\r\n" + path).c_str(), L"提示", MB_ICONWARNING);
                return 0;
            }
            HINSTANCE ret = ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
            if ((INT_PTR)ret <= 32)
            {
                wstringstream ss;
                ss << L"无法打开图片（错误代码 " << (INT_PTR)ret << L"）\r\n" << path;
                MessageBoxW(hwnd, ss.str().c_str(), L"提示", MB_ICONWARNING);
            }
            return 0;
        }
        if (id == IDC_ARTWIKI)
        {
            if (g_artSel < 0)
            {
                MessageBoxW(hwnd, L"请先在左侧选择角色。", L"提示", MB_ICONINFORMATION);
                return 0;
            }
            ShellExecuteW(NULL, L"open", U8(WEBSIDES[g_artSel]).c_str(), NULL, NULL, SW_SHOWNORMAL);
            return 0;
        }
        return 0;
    }
    case WM_DRAWITEM:
        if (wParam == IDC_ARTLIST)
        {
            DrawOwnerItem((const DRAWITEMSTRUCT*)lParam);
            return TRUE;
        }
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        PaintArtImage(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_hArt = NULL;
        if (g_artImage)
        {
            delete g_artImage;
            g_artImage = NULL;
        }
        return 0;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

//更新修改
//主窗口过程
LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        //① 选择卡池
        Mk(L"BUTTON", L"① 选择卡池", BS_GROUPBOX, 10, 10, 640, 150, 0, hwnd);
        Mk(L"STATIC", L"版本：", SS_LEFT, 24, 42, 56, 22, 0, hwnd);
        hVersion = Mk(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 90, 38, 180, 300, IDC_VERSION, hwnd);
        Mk(L"STATIC", L"卡池：", SS_LEFT, 286, 42, 56, 22, 0, hwnd);
        hBanner = Mk(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 344, 38, 294, 400, IDC_BANNER, hwnd);
        hInfo = Mk(L"STATIC", L"", SS_LEFT, 24, 70, 614, 80, IDC_INFO, hwnd);
        //② 保底与次数
        Mk(L"BUTTON", L"② 保底与次数", BS_GROUPBOX, 10, 170, 640, 110, 0, hwnd);
        Mk(L"STATIC", L"剩余大保底", SS_LEFT, 24, 200, 92, 20, 0, hwnd);
        hBig = Mk(L"EDIT", L"120", ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP, 120, 197, 60, 22, IDC_BIG, hwnd);
        Mk(L"STATIC", L"剩余小保底", SS_LEFT, 210, 200, 92, 20, 0, hwnd);
        hSmall = Mk(L"EDIT", L"80", ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP, 306, 197, 60, 22, IDC_SMALL, hwnd);
        Mk(L"STATIC", L"剩余5星保底", SS_LEFT, 24, 230, 100, 20, 0, hwnd);
        hFive = Mk(L"EDIT", L"10", ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP, 128, 227, 60, 22, IDC_FIVE, hwnd);
        Mk(L"STATIC", L"寻访次数", SS_LEFT, 210, 230, 72, 20, 0, hwnd);
        hCount = Mk(L"EDIT", L"10", ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP, 286, 227, 60, 22, IDC_COUNT, hwnd);
        Mk(L"STATIC", L"范围：大保底 0~120（0=已消耗）、小保底 1~80、5星保底 1~10、次数 1~10000", SS_LEFT, 24, 254, 610, 20, 0, hwnd);
        //③ 操作
        Mk(L"BUTTON", L"③ 操作", BS_GROUPBOX, 10, 290, 640, 120, 0, hwnd);
        Mk(L"BUTTON", L"寻访1次", BS_PUSHBUTTON | WS_TABSTOP, 24, 318, 110, 32, IDC_DRAW1, hwnd);
        Mk(L"BUTTON", L"寻访10次", BS_PUSHBUTTON | WS_TABSTOP, 142, 318, 110, 32, IDC_DRAW10, hwnd);
        Mk(L"BUTTON", L"按次数寻访", BS_PUSHBUTTON | WS_TABSTOP, 260, 318, 130, 32, IDC_DRAWCOUNT, hwnd);
        Mk(L"BUTTON", L"声明与规则", BS_PUSHBUTTON | WS_TABSTOP, 400, 318, 110, 32, IDC_DECLARE, hwnd);
        Mk(L"BUTTON", L"仓库统计", BS_PUSHBUTTON | WS_TABSTOP, 518, 318, 110, 32, IDC_STORE, hwnd);
        Mk(L"BUTTON", L"角色图鉴", BS_PUSHBUTTON | WS_TABSTOP, 24, 360, 110, 32, IDC_ART, hwnd);
        //④ 状态
        Mk(L"BUTTON", L"④ 当前状态", BS_GROUPBOX, 10, 420, 640, 320, 0, hwnd);
        hStatus = Mk(L"STATIC", L"", SS_LEFT, 24, 448, 612, 280, IDC_STATUS, hwnd);
        //结果列表
        Mk(L"BUTTON", L"寻访结果（最多显示最近2000行）", BS_GROUPBOX, 660, 10, 500, 730, 0, hwnd);
        hResult = Mk(L"LISTBOX", L"", LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_BORDER | WS_TABSTOP,
            672, 38, 476, 690, IDC_RESULT, hwnd);
        SendMessageW(hResult, LB_SETITEMHEIGHT, 0, S(20));

        //版本下拉
        for (int i = 1; i <= 7; i++)
        {
            const wchar_t* name = NULL;
            for (int k = 0; k < BANNER_COUNT; k++)
            {
                if (BANNERS[k].version == i)
                {
                    name = BANNERS[k].vname;
                    break;
                }
            }
            if (name)
                SendMessageW(hVersion, CB_ADDSTRING, 0, (LPARAM)name);
        }
        SendMessageW(hVersion, CB_SETCURSEL, 0, 0);
        //卡池下拉
        g_bannerCount = 0;
        for (int k = 0; k < BANNER_COUNT; k++)
        {
            if (BANNERS[k].version == 1)
                g_bannerIdx[g_bannerCount++] = k;
        }
        for (int i = 0; i < g_bannerCount; i++)
        {
            wstringstream ss;
            ss << BANNERS[g_bannerIdx[i]].name << L"（" << BANNERS[g_bannerIdx[i]].up << L"）";
            SendMessageW(hBanner, CB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
        }
        SendMessageW(hBanner, CB_SETCURSEL, 0, 0);
        g_curBanner = g_bannerIdx[0];
        RefreshBanner();
        return 0;
    }
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);
        if (id == IDC_VERSION && code == CBN_SELCHANGE)
        {
            int ver = (int)SendMessageW(hVersion, CB_GETCURSEL, 0, 0) + 1;   //1~6为版本，7为常驻
            g_bannerCount = 0;
            SendMessageW(hBanner, CB_RESETCONTENT, 0, 0);
            for (int k = 0; k < BANNER_COUNT; k++)
            {
                if (BANNERS[k].version == ver)
                    g_bannerIdx[g_bannerCount++] = k;
            }
            for (int i = 0; i < g_bannerCount; i++)
            {
                wstringstream ss;
                ss << BANNERS[g_bannerIdx[i]].name << L"（" << BANNERS[g_bannerIdx[i]].up << L"）";
                SendMessageW(hBanner, CB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
            }
            if (g_bannerCount > 0)
            {
                SendMessageW(hBanner, CB_SETCURSEL, 0, 0);
                g_curBanner = g_bannerIdx[0];
                RefreshBanner();
            }
            return 0;
        }
        if (id == IDC_BANNER && code == CBN_SELCHANGE)
        {
            int sel = (int)SendMessageW(hBanner, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < g_bannerCount)
            {
                g_curBanner = g_bannerIdx[sel];
                RefreshBanner();
            }
            return 0;
        }
        if (id == IDC_DRAW1)
        {
            DoPull(1);
            return 0;
        }
        if (id == IDC_DRAW10)
        {
            DoPull(10);
            return 0;
        }
        if (id == IDC_DRAWCOUNT)
        {
            int n = GetEditInt(hCount);
            if (n < 1 || n > 10000)
            {
                MessageBoxW(hwnd, L"寻访次数需填写 1~10000 的整数。", L"输入有误", MB_ICONWARNING);
                return 0;
            }
            DoPull(n);
            return 0;
        }
        if (id == IDC_DECLARE)
        {
            ShowTextWindow(L"声明与抽卡规则", DECLARE_TEXT);
            return 0;
        }
        if (id == IDC_STORE)
        {
            ShowTextWindow(L"仓库统计", BuildStoreText());
            return 0;
        }
        if (id == IDC_ART)
        {
            ShowArtWindow();
            return 0;
        }
        return 0;
    }
    case WM_DRAWITEM:
        if (wParam == IDC_RESULT)
        {
            DrawOwnerItem((const DRAWITEMSTRUCT*)lParam);
            return TRUE;
        }
        return 0;
    case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam, TRANSPARENT);
        SetTextColor((HDC)wParam, RGB(20, 20, 20));
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

//显示文本窗口
void ShowTextWindow(const wchar_t* title, const wstring& text)
{
    if (g_hText)
    {
        SetWindowTextW(g_hText, title);
        SetWindowTextW(GetDlgItem(g_hText, IDC_TEXTTEXT), text.c_str());
        SetForegroundWindow(g_hText);
        return;
    }
    RECT r = {0, 0, S(TEXT_W), S(TEXT_H)};
    AdjustWindowRect(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);
    g_hText = CreateWindowExW(WS_EX_TOOLWINDOW, L"EndfieldTextWnd", title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top, g_hMain, NULL, g_hInst, NULL);
    //把文本交给窗口过程
    //（WM_CREATE无法取到外部变量，这里创建后直接填充）
    if (g_hText)
    {
        SetWindowTextW(GetDlgItem(g_hText, IDC_TEXTTEXT), text.c_str());
        ShowWindow(g_hText, SW_SHOW);
    }
}

//显示角色图鉴窗口
void ShowArtWindow()
{
    if (g_hArt)
    {
        SetForegroundWindow(g_hArt);
        return;
    }
    RECT r = {0, 0, S(ART_W), S(ART_H)};
    AdjustWindowRect(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);
    g_hArt = CreateWindowExW(0, L"EndfieldArtWnd", L"角色图鉴",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top, g_hMain, NULL, g_hInst, NULL);
    if (g_hArt)
        ShowWindow(g_hArt, SW_SHOW);
}

//更新修改
//程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;
    //开启DPI感知并按系统DPI缩放，避免高分屏下界面模糊
    SetProcessDPIAware();
    HDC screen = GetDC(NULL);
    if (screen)
    {
        g_scale = (double)GetDeviceCaps(screen, LOGPIXELSX) / 96.0;
        ReleaseDC(NULL, screen);
    }
    //初始化GDI+（立绘预览）
    Gdiplus::GdiplusStartupInput gdiInput;
    Gdiplus::GdiplusStartup(&g_gdiToken, &gdiInput, NULL);

    g_hFont = CreateFontW(-S(16), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    if (!g_hFont)
        g_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    //注册主窗口
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = MainProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"EndfieldGachaMain";
    wc.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    RegisterClassExW(&wc);
    //注册文本窗口
    wc.lpfnWndProc = TextProc;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"EndfieldTextWnd";
    wc.hIcon = NULL;
    RegisterClassExW(&wc);
    //注册图鉴窗口
    wc.lpfnWndProc = ArtProc;
    wc.lpszClassName = L"EndfieldArtWnd";
    RegisterClassExW(&wc);

    RECT r = {0, 0, S(WIN_W), S(WIN_H)};
    AdjustWindowRect(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    g_hMain = CreateWindowExW(0, L"EndfieldGachaMain", L"终末地抽卡模拟器（图形界面版） v1.5",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top, NULL, NULL, hInstance, NULL);
    if (!g_hMain)
        return 0;
    ShowWindow(g_hMain, nCmdShow);
    UpdateWindow(g_hMain);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0)
    {
        if (!IsDialogMessageW(g_hMain, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    if (g_hFont)
        DeleteObject(g_hFont);
    Gdiplus::GdiplusShutdown(g_gdiToken);
    return (int)msg.wParam;
}
