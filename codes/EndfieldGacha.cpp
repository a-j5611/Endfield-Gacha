/*
终末地抽卡模拟器
制作者：爱写作业的好汉（https://space.bilibili.com/3546375050496178）
制作时间：2026.7.12：完成+加入1.0~1.4卡池
    2026.7.15：添加计算出率（分星级、角色）
    2026.7.16：完善1.4卡池信息 添加UTF-8编码运行 添加计算嵌晶玉和衍质源石数量
    2026.7.18：添加1.0~1.2可歪限定 补充声明 修改计算衍质源石向上取整 添加计算武库配额及申领次数
    2026.8.9：完善1.0卡池逻辑（三卡池可歪限定互通） 修改信物抽取逻辑（非保底，为附加奖励）
    2026.10.2：按游戏实际机制重写保底逻辑（软保底第66抽起每抽+5%） 修正辉光庆典特殊寻访与重构寻访 修正1.4/1.5卡池信息 修正货币换算与武库配额数值
注：更新请搜索注释“//更新修改”
*/
#include<iostream>
#include<vector>
#include<string>
#include<ctime>
#include<map>
#include<windows.h>
#include<cstdlib>
#include<cstring>
#include<iomanip>
#include<algorithm>
#include<shellapi.h>
using namespace std;

map<string, int> owned;// 记录角色获取次数

int act;//用户操作

//角色列表
vector<string>stars6;
int stars6_count;
int up_stars6_count;
bool up_first;
int able_get_stars6;
vector<string>stars5;
int stars5_count;
int able_get_stars5;
vector<string>stars4;
int stars4_count;
int able_get_stars4;

//版本及卡池
int version;
int section;

//大、小保底剩余
int stars6_big_baseline;
int stars6_small_baseline;
int stars5_baseline;

//一次性抽取次数
int g;

//总抽取次数
int g_count = 0;

//武库配额
long long weapon_p = 0;

int token[10005];

//更新修改
//当期寻访累计次数与里程碑奖励状态（每次选择卡池后重置）
int banner_pulls = 0;
int token_special = 0;//特殊寻访自选信物补给次数
int intel_book = 0;//寻访情报书数量（特许寻访累计60次获得）
int select_voucher = 0;//流光庆时调用凭证数量（辉光庆典累计120次获得）
bool milestone_30 = false;
bool milestone_60 = false;
bool milestone_90 = false;
bool milestone_120 = false;
int banner_type = 1;//0常驻 1特许寻访 2特殊寻访 3重构寻访

//更新修改
const vector<string> ALL_STARS6 = {
    "莱万汀", "艾尔黛拉", "余烬", "黎风", "别礼", "骏卫",
    "洁尔佩塔", "伊冯", "汤汤", "洛茜", "庄方宜", "弭弗", "卡缪","诀","梨诺",
    "提弗洛斯"
};
const vector<string> ALL_STARS5 = {
    "阿列什", "大潘", "昼雪", "艾维文娜", "赛希",
    "弧光", "狼卫", "陈千语", "佩丽卡","噗切娜"
};
const vector<string> ALL_STARS4 = {
    "安塔尔", "萤石", "埃特拉", "卡契尔", "秋栗"
};

//更新修改
//此处为角色列表（不分星级）
const vector<string> ALL_CHARACTERS = {
    //6星
    "莱万汀", "艾尔黛拉", "余烬", "黎风", "别礼", "骏卫","管理员-男","管理员-女",
    "洁尔佩塔", "伊冯", "汤汤", "洛茜", "庄方宜", "弭弗", "卡缪","诀","梨诺","提弗洛斯",

    //5星
    "阿列什", "大潘", "昼雪", "艾维文娜", "赛希",
    "弧光", "狼卫", "陈千语", "佩丽卡","噗切娜",

    //4星
    "安塔尔", "萤石", "埃特拉", "卡契尔", "秋栗"
};

//更新修改
//此处为角色英文名（与上方一一对应）
const vector<string> ENGLISH = {
    //6星
    "Laevatain", "Ardelia", "Ember", "Lifeng", "LastRite", "Pogranichnik","EndministratorMale","EndministratorFemale",
    "Gilberta", "Yvonne", "Tangtang", "Rossi", "ZhuangFangyi", "Mifu", "Camille","Arcane","Liino","Typhoeus",

    //5星
    "Alesh", "DaPan", "Snowshine", "Avywenna", "Xaihi",
    "Arclight", "Wulfgard", "ChenQianyu", "Perlica","Purrchena",
    //4星
    "Antal", "Fluorite", "Estella", "Catcher", "Akekuri"
};

//更新修改
//此处为角色介绍网址
const vector<string> WEBSIDES = {
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E8%8E%B1%E4%B8%87%E6%B1%80","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E8%89%BE%E5%B0%94%E9%BB%9B%E6%8B%89","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E4%BD%99%E7%83%AC",
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E9%BB%8E%E9%A3%8E","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%88%AB%E7%A4%BC","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E9%AA%8F%E5%8D%AB",
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E7%AE%A1%E7%90%86%E5%91%98%C2%B7%E7%94%B7","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E7%AE%A1%E7%90%86%E5%91%98%C2%B7%E5%A5%B3",

    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E6%B4%81%E5%B0%94%E4%BD%A9%E5%A1%94","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E4%BC%8A%E5%86%AF","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E6%B1%A4%E6%B1%A4",
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E6%B4%9B%E8%8C%9C","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%BA%84%E6%96%B9%E5%AE%9C","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%BC%AD%E5%BC%97",
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%8D%A1%E7%BC%AA","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E8%AF%80","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E6%A2%A8%E8%AF%BA","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E6%8F%90%E5%BC%97%E6%B4%9B%E6%96%AF",

    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E9%98%BF%E5%88%97%E4%BB%80","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%A4%A7%E6%BD%98","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E6%98%BC%E9%9B%AA",
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E8%89%BE%E7%BB%B4%E6%96%87%E5%A8%9C","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E8%B5%9B%E5%B8%8C",
    
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%BC%A7%E5%85%89","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E7%8B%BC%E5%8D%AB","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E9%99%88%E5%8D%83%E8%AF%AD",
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E4%BD%A9%E4%B8%BD%E5%8D%A1","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%99%97%E5%88%87%E5%A8%9C",

    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%AE%89%E5%A1%94%E5%B0%94","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E8%90%A4%E7%9F%B3","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%9F%83%E7%89%B9%E6%8B%89",
    "https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E5%8D%A1%E5%A5%91%E5%B0%94","https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98/%E7%A7%8B%E6%A0%97"
};

//获取英文名
string GetEnglish(const string& name) {
    auto it = find(ALL_CHARACTERS.begin(), ALL_CHARACTERS.end(), name);
    if (it != ALL_CHARACTERS.end())
        return ENGLISH[it - ALL_CHARACTERS.begin()];
    return "";
}

int getRand(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void Statement()
{
    system("cls");
    cout <<"本软件制作者：爱写作业的好汉（https://space.bilibili.com/3546375050496178）\n";
    cout <<"关于《明日方舟：终末地》的一切权利归鹰角网络所有，侵权必删！\n";
    cout <<"本工具：卡池信息来源于https://end.canmoe.com/zh-CN/banner-calendar \n干员介绍连接至https://www.fz.wiki/wiki/%E5%B9%B2%E5%91%98\n";
    cout <<"本工具无任何版权，侵权必删！\n";
    system("pause");
}

//更新修改
//此处为设置卡池信息
void SetSection(int section)
{
    while(!stars6.empty())
        stars6.pop_back();
    while(!stars5.empty())
        stars5.pop_back();
    while(!stars4.empty())
        stars4.pop_back();
    able_get_stars6 = 8;
    able_get_stars5 = 80;
    able_get_stars4 = 912;
    banner_type = 1;//更新修改：卡池类型，1特许寻访 2特殊寻访 3重构寻访 0常驻
    //1.0上半（更新修改：当期伊冯/洁尔佩塔尚未实装，6星池仅莱万汀+5名常驻）
    if(section == 1)
    {
        stars6.push_back("莱万汀");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 6;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.0中半（更新修改：当期伊冯尚未实装，6星池仅洁尔佩塔+莱万汀+5名常驻）
    if(section == 2)
    {
        stars6.push_back("洁尔佩塔");
        stars6.push_back("莱万汀");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 7;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.0下半
    if(section == 3)
    {
        stars6.push_back("伊冯");
        stars6.push_back("莱万汀");
        stars6.push_back("洁尔佩塔");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.1上半
    if(section == 4)
    {
        stars6.push_back("汤汤");
        stars6.push_back("伊冯");
        stars6.push_back("洁尔佩塔");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.1下半
    if(section == 5)
    {
        stars6.push_back("洛茜");
        stars6.push_back("汤汤");
        stars6.push_back("伊冯");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.2全场
    if(section == 6)
    {
        stars6.push_back("庄方宜");
        stars6.push_back("汤汤");
        stars6.push_back("洛茜");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.2辉光庆典（更新修改：特殊寻访，保底计数独立，池内6星仅有4名UP角色，
    //出6星必为四选一无50/50；30/60/120/240抽里程碑奖励见抽卡输出）
    if(section == 7)
    {
        banner_type = 2;
        stars6.push_back("莱万汀");
        stars6.push_back("洁尔佩塔");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("骏卫");
        stars6_count = 4;
        up_stars6_count = 4;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.3上半
    if(section == 8)
    {
        stars6.push_back("弭弗");
        stars6.push_back("庄方宜");
        stars6.push_back("洛茜");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.3下半
    if (section == 9)
    {
        stars6.push_back("卡缪");
        stars6.push_back("弭弗");
        stars6.push_back("庄方宜");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
    //1.4上半
    if(section == 10)
    {
        stars6.push_back("诀");
        stars6.push_back("卡缪");
        stars6.push_back("弭弗");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
    }
    //1.4下半
    if(section == 11)
    {
        stars6.push_back("梨诺");
        stars6.push_back("诀");
        stars6.push_back("卡缪");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
    }
    //1.5上半
    if(section == 12)
    {
        stars6.push_back("提弗洛斯");
        stars6.push_back("梨诺");
        stars6.push_back("诀");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
    }
    //1.5下半（更新修改：重构寻访，与特许寻访保底不互通；前120抽必得伊冯，30/60/90抽各送加急十连）
    if(section == 13)
    {
        banner_type = 3;
        stars6.push_back("伊冯");
        stars6.push_back("提弗洛斯");
        stars6.push_back("梨诺");
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 8;
        up_stars6_count = 1;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
    }
    //常驻（更新修改：无UP与角色井，亦无里程碑奖励）
    if(section == 999)
    {
        banner_type = 0;
        stars6.push_back("艾尔黛拉");
        stars6.push_back("余烬");
        stars6.push_back("黎风");
        stars6.push_back("别礼");
        stars6.push_back("骏卫");
        stars6_count = 5;
        up_stars6_count = 0;
        stars5.push_back("阿列什");
        stars5.push_back("大潘");
        stars5.push_back("昼雪");
        stars5.push_back("艾维文娜");
        stars5.push_back("赛希");
        stars5.push_back("弧光");
        stars5.push_back("狼卫");
        stars5.push_back("陈千语");
        stars5.push_back("佩丽卡");
        stars5_count = 9;
        stars4.push_back("安塔尔");
        stars4.push_back("萤石");
        stars4.push_back("埃特拉");
        stars4.push_back("卡契尔");
        stars4.push_back("秋栗");
        stars4_count = 5;
        return;
    }
}

//更新修改
//此处为选择卡池
bool AskSection(int version)
{
    while(true)
    {
        system("cls");
        cout <<"请选择卡池：\n";
        cout <<"对应版本 卡池名称       UP角色   时间                序号\n";
        //1.0版本
        if(version == 1)
        {
            cout <<"1.0上半  熔火灼痕       莱万汀   2026.1.22~2026.2.7  1\n";
            cout <<"1.0中半  轻飘飘的信使   洁尔佩塔 2026.2.7~2026.2.24  2\n";
            cout <<"1.0下半  热烈色彩       伊冯     2026.2.24~2026.3.12 3\n";
        }
        //1.1版本
        if(version == 2)
        {
            cout <<"1.1上半  河流的女儿     汤汤     2026.3.12~2026.3.29  4\n";
            cout <<"1.1下半  狼珀           洛茜     2026.3.29~2026.4.17  5\n";
        }
        //1.2版本
        if(version == 3)
        {
            cout <<"1.2全场  春雷动，万物生 庄方宜   2026.4.17~2026.5.22 6\n";
            cout <<"1.2特殊  辉光庆典       下方注释 2026.5.14~2026.6.5  7\n";
            cout <<"（注：辉光庆典为特殊寻访：池内6星仅莱万汀/洁尔佩塔/艾尔黛拉/骏卫，保底计数独立）\n";
        }
        //1.3版本
        if(version == 4)
        {
            cout <<"1.3上半  拳出无悔       弭弗     2026.6.5~2026.6.26  8\n";
            cout <<"1.3下半  逐罪者         卡缪     2026.6.26~2026.7.16 9\n";
        }
        //1.4版本
        if(version == 5)
        {
            cout <<"1.4上半  临渊望北       诀      2026.7.16~2026.8.9   10\n";
            cout <<"1.4下半  晨星于此闪耀   梨诺    2026.8.9~2026.9.2    11\n";
        }
        //1.5版本
        if(version == 6)
        {
            cout <<"1.5上半  冬猎           提弗洛斯 2026.9.2~2026.9.30   12\n";
            cout <<"1.5下半  绚丽异彩       伊冯    2026.9.24~2026.10.14  13\n";
        }
        //常驻
        if(version == 999)
        {
            SetSection(999);
        }
        cout <<"选择卡池（输入对应序号，返回上一级输入666）：";
        cin >> section;
        if(section==666)
            return false;
        bool valid = false;
        if (version == 1 && (section >= 1 && section <= 3)) 
            valid = true;
        else if (version == 2 && (section == 4 || section == 5)) 
            valid = true;
        else if (version == 3 && (section == 6 || section == 7)) 
            valid = true;
        else if (version == 4 && (section == 8 || section == 9)) 
            valid = true;
        else if (version == 5 && (section == 10 || section == 11))
            valid = true;
        else if(version == 6 && (section == 12 || section == 13))
            valid = true;
        if (valid) {
            SetSection(section);
            return true;
        } else {
            cout << "\n输入有误，请重新选择卡池\n";
            Sleep(1500);
        }
    }
}

//更新修改
//此处为选择版本
bool AskVersion()
{
    while(true)
    {
        system("cls");
        cout <<"请选择版本：\n";
        cout <<"版本 名称           时间                序号\n";
        cout <<"1.0  零号委托       2026.1.22~2026.3.12 1\n";
        cout <<"1.1  新潮起，故渊离 2026.3.12~2026.4.17 2\n";
        cout <<"1.2  春晓时         2026.4.17~2026.6.4  3\n";
        cout <<"1.3  寻遗散记       2026.6.5~2026.7.16  4\n";
        cout <<"1.4  向渊行         2026.7.16~2026.9.2  5\n";
        cout <<"1.5  雪凇幽梦       2026.9.2~2026.10.14 6\n";
        cout <<"/    常驻池        /                   999\n";
        cout <<"选择版本（输入对应序号，返回上一级输入666）：";
        cin >> version;
        if(version == 666)
            return false;
        if(version == 999)
        {
            SetSection(999);
            return true;
        }
        if (version >= 1 && version <= 6)
        {
            if (!AskSection(version))
                continue;
            else
                return true;
        }
        else
        {
            cout << "\n输入有误，请重新选择版本\n";
            Sleep(1500);
        }
    }
}

//更新修改
//抽卡（已按游戏实际规则重写：
//6星基础出率0.8%，距上次6星满66抽起（第66抽出率5.8%）每抽+5%，第80抽必得；
//5星基础出率8%，10抽内必得5星及以上，出5星或6星均清空计数；
//当期UP占6星出率的50%，120抽角色井单池仅生效一次（获得UP后即消耗）；
//信物赠礼：每累计寻访240次获得当期UP干员信物×1，特殊寻访为自选信物补给）
pair<int,int> gacha()
{
    int result_stars;
    int result_character;
    int now_able_get_6stars = able_get_stars6;
    int now_able_get_4stars = able_get_stars4;
    int no_six = 80 - stars6_small_baseline;//距上次6星的寻访次数
    if (no_six >= 65)
    {
        //软保底：第66抽起每抽+5%（第66抽5.8%，第79抽70.8%，第80抽必得）
        now_able_get_6stars = 8 + 50 * (no_six - 64);
        now_able_get_4stars = able_get_stars4 - 50 * (no_six - 64);
    }
    banner_pulls++;
    //信物赠礼：每累计寻访240次获得当期UP干员信物×1（特殊寻访为自选信物补给）
    if (banner_pulls % 240 == 0 && up_stars6_count > 0)
    {
        if (banner_type == 2)
            token_special++;
        else
            token[0]++;
    }
    result_stars = getRand(1,1000);
    if(result_stars <= now_able_get_6stars)
        result_stars = 6;
    else if(result_stars > now_able_get_6stars && result_stars <= now_able_get_6stars + able_get_stars5)
        result_stars = 5;
    else if(result_stars > now_able_get_6stars + able_get_stars5 && result_stars <= 1000)
        result_stars = 4;
    if(stars5_baseline == 1 && result_stars == 4)
    {
        //5星保底仅在本次未出5星及以上时触发（不会覆盖已掷出的6星）
        result_stars = 5;
    }
    if (stars6_big_baseline == 1 && up_first == true && up_stars6_count > 0)
    {
        //角色井触发：120抽内必得当期UP（单池仅生效一次，获得后即消耗）
        result_stars = 6;
        result_character = getRand(1,up_stars6_count)-1;
        stars6_big_baseline = 0;
        stars6_small_baseline = 80;
        stars5_baseline = 10;
        up_first = false;
        return {result_stars,result_character};
    }
    if (stars6_small_baseline == 1)
    {
        result_stars = 6;
        stars6_small_baseline = 80;
    }
    if (stars6_big_baseline > 0)
        stars6_big_baseline -= 1;//已消耗(0)后不再递减为负数，便于界面显示
    stars6_small_baseline -= 1;
    stars5_baseline -= 1;
    if (result_stars == 6)
    {
        if (up_stars6_count > 0 && up_stars6_count < stars6_count)
        {
            int up_chance = getRand(1, 1000);
            if (up_chance <= 500)
            {
                result_character = getRand(1, up_stars6_count) - 1;
                stars6_big_baseline = 0;//获得UP后角色井即消耗（单池仅一次）
            }
            else
                result_character = getRand(1, stars6_count - up_stars6_count) + up_stars6_count - 1;
        }
        else if (up_stars6_count == stars6_count)
        {
            //特殊寻访（辉光庆典）：卡池内6星均为UP角色，出6星必为四选一
            result_character = getRand(1, stars6_count) - 1;
        }
        else
            result_character = getRand(1, stars6_count) - 1;
        stars6_small_baseline = 80;
        stars5_baseline = 10;//出5星及以上清空5星保底
    }
    else if(result_stars == 5)
    {
        result_character = getRand(1,stars5_count) - 1;
        stars5_baseline = 10;
    }
    else if (result_stars == 4)
        result_character = getRand(1, stars4_count) - 1;
    return{result_stars,result_character};
}

//更新修改
//加急招募（免费十连）单抽：概率与当期寻访基础概率一致（不享受软保底加成），
//十连内必定能通过保底获取5星或以上干员；结果不计入任何保底计数与累计寻访次数
pair<int,int> urgent_gacha(int& five_guard)
{
    int result_stars = getRand(1,1000);
    int result_character;
    if(result_stars <= able_get_stars6)
        result_stars = 6;
    else if(result_stars > able_get_stars6 && result_stars <= able_get_stars6 + able_get_stars5)
        result_stars = 5;
    else
        result_stars = 4;
    if(five_guard == 9 && result_stars == 4)
        result_stars = 5;
    if(result_stars == 4)
        five_guard++;
    else
        five_guard = 0;
    if (result_stars == 6)
    {
        if (up_stars6_count > 0 && up_stars6_count < stars6_count)
        {
            int up_chance = getRand(1, 1000);
            if (up_chance <= 500)
                result_character = getRand(1, up_stars6_count) - 1;
            else
                result_character = getRand(1, stars6_count - up_stars6_count) + up_stars6_count - 1;
        }
        else
            result_character = getRand(1, stars6_count) - 1;
    }
    else if(result_stars == 5)
        result_character = getRand(1,stars5_count) - 1;
    else
        result_character = getRand(1, stars4_count) - 1;
    return{result_stars,result_character};
}

//更新修改
//内核：供控制台版与图形界面版共用的抽卡接口
//寻访输出项：type 0=寻访结果 1=加急招募结果 2=文字消息
struct GachaItem
{
    int type;
    int stars;
    string text;
};

//按星级与下标取角色名
string CharacterName(int stars, int index)
{
    if (stars == 6)
        return stars6[index];
    if (stars == 5)
        return stars5[index];
    return stars4[index];
}

//执行一次加急招募（免费十连）：不计入保底计数与累计寻访次数
vector<pair<int,int> > DrawUrgentRecruit()
{
    vector<pair<int,int> > out;
    int five_guard = 0;
    for (int i = 1; i <= 10; i++)
    {
        pair<int, int> a = urgent_gacha(five_guard);
        owned[CharacterName(a.first, a.second)]++;
        out.push_back(a);
    }
    return out;
}

//文字消息
void PushMessage(vector<GachaItem>& out, const string& text)
{
    GachaItem it;
    it.type = 2;
    it.stars = 0;
    it.text = text;
    out.push_back(it);
}

//寻访结果
void PushPull(vector<GachaItem>& out, int type, int i, const pair<int,int>& a)
{
    GachaItem it;
    it.type = type;
    it.stars = a.first;
    it.text = "第" + to_string(i) + "抽：" + (a.first == 6 ? "6星 " : (a.first == 5 ? "5星 " : "4星 ")) + CharacterName(a.first, a.second);
    out.push_back(it);
}

//加急招募（消息+10次结果）
void PushUrgentRecruit(vector<GachaItem>& out)
{
    PushMessage(out, "\n获得加急招募10次（免费十连，结果不计入保底计数与累计寻访次数）：");
    vector<pair<int,int> > a = DrawUrgentRecruit();
    for (size_t i = 0; i < a.size(); i++)
        PushPull(out, 1, (int)i + 1, a[i]);
}

//执行一次批量寻访（含里程碑奖励、加急招募与信物赠礼），按顺序返回输出项
vector<GachaItem> DoBatchPull(int times)
{
    vector<GachaItem> out;
    g_count += times;
    for (int i = 1; i <= times; i++)
    {
        pair<int, int> a = gacha();
        owned[CharacterName(a.first, a.second)]++;
        PushPull(out, 0, i, a);
    }
    //寻访里程碑奖励（常驻池无此类奖励）
    if (up_stars6_count > 0)
    {
        if (banner_pulls >= 30 && !milestone_30)
        {
            milestone_30 = true;
            PushUrgentRecruit(out);
        }
        if (banner_type == 3)
        {
            //重构寻访：累计30/60/90次分别额外获得1次免费十连
            if (banner_pulls >= 60 && !milestone_60)
            {
                milestone_60 = true;
                PushUrgentRecruit(out);
            }
            if (banner_pulls >= 90 && !milestone_90)
            {
                milestone_90 = true;
                PushUrgentRecruit(out);
            }
        }
        else if (banner_type == 2)
        {
            //特殊寻访（辉光庆典）：60次基础寻访凭证×10，120次自选调用凭证×1
            if (banner_pulls >= 60 && !milestone_60)
            {
                milestone_60 = true;
                PushMessage(out, "累计寻访达60次，获得【基础寻访凭证】×10");
            }
            if (banner_pulls >= 120 && !milestone_120)
            {
                milestone_120 = true;
                select_voucher++;
                PushMessage(out, "累计寻访达120次，获得【流光庆时调用凭证】×1（可从莱万汀/洁尔佩塔/艾尔黛拉/骏卫中自选一名干员获取）");
            }
        }
        else
        {
            //特许寻访：60次获得寻访情报书（下一次特许寻访开启后自动转化为10张专有寻访凭证）
            if (banner_pulls >= 60 && !milestone_60)
            {
                milestone_60 = true;
                intel_book++;
                PushMessage(out, "累计寻访达60次，获得【寻访情报书】×1（将在下一次特许寻访开启后自动转化为10张专有寻访凭证）");
            }
        }
    }
    //信物赠礼统计（每累计寻访240次获得1个，按1潜能计入角色获取次数）
    int l = 0;
    for(int i = 0;i < up_stars6_count;i++)
    {
        l += token[i];
        owned[stars6[i]] += token[i];
        token[i] = 0;
    }
    PushMessage(out, "本次获得当期UP干员信物" + to_string(l) + "个");
    if (token_special > 0)
    {
        PushMessage(out, "本次获得【流光庆时信物补给】×" + to_string(token_special) + "（可从莱万汀/洁尔佩塔/艾尔黛拉/骏卫的信物中自选，需已拥有对应干员）");
        token_special = 0;
    }
    return out;
}

//武库配额折算（每获得1名干员：6星2000、5星200、4星20）
long long CalcWeaponQuota()
{
    long long sum = 0;
    for (const auto& name : ALL_STARS6)
    {
        auto it = owned.find(name);
        if (it != owned.end())
            sum += (long long)it->second * 2000;
    }
    for (const auto& name : ALL_STARS5)
    {
        auto it = owned.find(name);
        if (it != owned.end())
            sum += (long long)it->second * 200;
    }
    for (const auto& name : ALL_STARS4)
    {
        auto it = owned.find(name);
        if (it != owned.end())
            sum += (long long)it->second * 20;
    }
    return sum;
}

//更新修改
//以可执行文件所在目录为基准拼接立绘路径（窄字符，控制台用；避免工作目录不同导致ShellExecute返回5）
string GetImagePath(const string& english)
{
    char exePath[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    string path = exePath;
    size_t pos = path.find_last_of("\\/");
    if (pos != string::npos)
        path = path.substr(0, pos + 1);
    return path + "CharacterImages\\" + english + ".jpg";
}

//以可执行文件所在目录为基准拼接立绘路径（宽字符，图形界面用）
wstring GetImagePathW(const wstring& english)
{
    wchar_t exePath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wstring path = exePath;
    size_t pos = path.find_last_of(L"\\/");
    if (pos != wstring::npos)
        path = path.substr(0, pos + 1);
    return path + L"CharacterImages\\" + english + L".jpg";
}

void GachaMain()
{
    while (true)
    {
        system("cls");
        if (!AskVersion())
            return;
        system("cls");
        srand(time(0));
        //更新修改
        //重置当期寻访累计次数与里程碑奖励状态（每次选择卡池后重置）
        banner_pulls = 0;
        token_special = 0;
        milestone_30 = false;
        milestone_60 = false;
        milestone_90 = false;
        milestone_120 = false;
        int act = 0;
        int max_big_baseline;
        do
        {
            cout << "抽取角色/信物（角色（第一次抽取）输入1，信物（第二次及以后抽取）输入2）：";
            cin >> act;
            if (act != 1 && act != 2)
                cout << "\n输入有误，请重新输入";
            system("cls");
        }while (act != 1 && act != 2);
        //更新修改
        //仅角色模式且有UP角色井的卡池（特许/重构寻访）设置大保底；
        //信物模式、常驻池、特殊寻访（辉光庆典）均无角色井（特殊寻访120抽为自选凭证奖励，非保底）
        if(act == 1 && up_stars6_count > 0 && banner_type != 2)
        {
            max_big_baseline = 120;
            up_first = true;
            do
            {
                cout << "请输入剩余大保底（最大" << max_big_baseline << "）：";
                cin >> stars6_big_baseline;
                if (stars6_big_baseline <= 0 || stars6_big_baseline > max_big_baseline)
                    cout << "\n输入有误，请重新输入";
                system("cls");
            }while (stars6_big_baseline <= 0 || stars6_big_baseline > max_big_baseline);
        }
        else
        {
            stars6_big_baseline = 0;
            up_first = false;
        }
        do
        {
            cout << "请输入剩余小保底（最大80）：";
            cin >> stars6_small_baseline;
            if (stars6_small_baseline <= 0 || stars6_small_baseline > 80)
                cout << "\n输入有误，请重新输入";
            system("cls");
        }while (stars6_small_baseline <= 0 || stars6_small_baseline > 80);

        do
        {
            cout << "请输入剩余5星角色保底（最大10）：";
            cin >> stars5_baseline;
            if (stars5_baseline <= 0 || stars5_baseline > 10)
                cout << "\n输入有误，请重新输入";
            system("cls");
        }while (stars5_baseline <= 0 || stars5_baseline > 10);
        while (true)
        {
            system("cls");
            cout << "当期6星UP角色：";
            for (int i = 0; i < up_stars6_count; i++)
                cout << stars6[i] << ' ';
            cout << '\n';
            cout << "当期6星非UP角色：";
            for (int i = up_stars6_count; i < stars6_count; i++)
                cout << stars6[i] << ' ';
            cout << '\n';
            cout << "当期5星角色：";
            for (int i = 0; i < stars5_count; i++)
                cout << stars5[i] << ' ';
            cout << '\n';
            if (stars6_big_baseline > 0)
                cout << "当前剩余大保底：" << stars6_big_baseline << "抽必出当期UP\n";
            else
                cout << "当前大保底：本期角色井已消耗\n";
            cout << "当前剩余小保底：" << stars6_small_baseline << "抽必出6星角色\n";
            cout << "输入抽卡次数（输入66666返回卡池选择，最大值为10000）：";
            cin >> g;
            if (g == 66666)
                break;
            if (g > 10000 || g <= 0)
            {
                cout << "\n输入有误，请重新输入\n";
                Sleep(1500);
                continue;
            }
            //更新修改
            //统一调用抽卡内核：保底、里程碑奖励、加急招募与信物赠礼均在内核中处理
            cout << "\n抽卡中……\n";
            vector<GachaItem> items = DoBatchPull(g);
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            for (size_t k = 0; k < items.size(); k++)
            {
                if (items[k].type == 2)
                    SetConsoleTextAttribute(hConsole, 7);        // 文字消息
                else if (items[k].stars == 6)
                    SetConsoleTextAttribute(hConsole, 6);        // 暗黄色(橙色)
                else if (items[k].stars == 5)
                    SetConsoleTextAttribute(hConsole, 14);       // 亮黄色(金色)
                else
                    SetConsoleTextAttribute(hConsole, 13);       // 亮紫色
                cout << items[k].text << '\n';
            }
            SetConsoleTextAttribute(hConsole, 7);
            system("pause");
        }
    }
}

void ShowOwned() {
    system("cls");
    //获得角色个数
    int count_6stars = 0;
    int count_5stars = 0;
    int count_4stars = 0;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (owned.empty()) {
        cout << "仓库为空，快去抽卡吧！\n";
    } else {
        weapon_p = CalcWeaponQuota();
        cout << "当前角色获取统计：\n";
        bool has6 = false;
        for (const auto& name : ALL_STARS6)
        {
            auto it = owned.find(name);
            if (it != owned.end())
            {
                SetConsoleTextAttribute(hConsole, 6);
                if (!has6)
                    cout << "【6星】\n"; has6 = true;
                cout << "  " << name << " × " << it->second <<"    出率："<< setprecision(4) << (double(it->second * 100) / double(g_count)) <<"%\n";
                count_6stars += it->second;
                SetConsoleTextAttribute(hConsole, 7);
            }
        }
        bool has5 = false;
        for (const auto& name : ALL_STARS5)
        {
            auto it = owned.find(name);
            if (it != owned.end())
            {
                SetConsoleTextAttribute(hConsole, 14);
                if (!has5)
                    cout << "【5星】\n"; has5 = true;
                cout << "  " << name << " × " << it->second <<"    出率："<< setprecision(4) << (double(it->second * 100) / double(g_count)) <<"%\n";
                count_5stars += it->second;
                SetConsoleTextAttribute(hConsole, 7);
            }
        }
        bool has4 = false;
        for (const auto& name : ALL_STARS4)
        {
            auto it = owned.find(name);
            if (it != owned.end())
            {
                SetConsoleTextAttribute(hConsole, 13);
                if (!has4)
                    cout << "【4星】\n"; has4 = true;
                cout << "  " << name << " × " << it->second <<"    出率："<< setprecision(4) << (double(it->second * 100) / double(g_count)) <<"%\n";
                count_4stars += it->second;
                SetConsoleTextAttribute(hConsole, 7);
            }
        }
        cout <<"\n综合出率：\n";
        SetConsoleTextAttribute(hConsole, 6);
        cout <<"【6星】："<< setprecision(4) << (double(count_6stars * 100) / double(g_count)) <<"%\n";
        SetConsoleTextAttribute(hConsole, 14);
        cout <<"【5星】："<< setprecision(4) << (double(count_5stars * 100) / double(g_count)) <<"%\n";
        SetConsoleTextAttribute(hConsole, 13);
        cout <<"【4星】："<< setprecision(4) << (double(count_4stars * 100) / double(g_count)) <<"%\n";
        SetConsoleTextAttribute(hConsole, 7);
        cout <<"\n\n";
        cout <<"目前已抽："<< g_count <<"抽\n";
        SetConsoleTextAttribute(hConsole, 0xC);
        cout <<"相当于嵌晶玉 × "<< (long long)g_count * 500 <<'\n';
        SetConsoleTextAttribute(hConsole, 0xE);
        if(g_count % 3 == 0)
        cout <<"或相当于衍质源石 × "<< (long long)g_count * 20 / 3 <<'\n';
        else
        cout <<"或相当于衍质源石 × "<< (long long)g_count * 20 / 3 + 1 <<'\n';
        cout <<"共计可获得武库配额 × "<< weapon_p <<'\n';
        cout <<"相当于"<< weapon_p / 1980 <<"次申领（武器池"<< weapon_p / 1980 * 10  <<"抽）\n";
        SetConsoleTextAttribute(hConsole,7);
    }
    //更新修改
    //寻访道具数量（寻访情报书、流光庆时调用凭证）
    cout <<"\n其他寻访道具：\n";
    SetConsoleTextAttribute(hConsole, 0xE);
    cout <<"  【寻访情报书】 × "<< intel_book <<"（下一次特许寻访开启后自动转化为10张专有寻访凭证）\n";
    cout <<"  【流光庆时调用凭证】 × "<< select_voucher <<"（可从莱万汀/洁尔佩塔/艾尔黛拉/骏卫中自选一名干员获取）\n";
    SetConsoleTextAttribute(hConsole, 7);
    system("pause");
}

//更新修改
//此处为展示角色立绘
void ShowCharacterIllustration() {
    while (true) {
        system("cls");
        cout << "角色图鉴（选择角色查看立绘，输入666返回）\n\n";
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        int idx = 1;
        cout << "【6星】\n";

        //更新修改
        //修改循环次数——————————
        for (int i = 0; i < 18; i++)
        {
            SetConsoleTextAttribute(hConsole, 6);
            cout << idx << ". " << ALL_CHARACTERS[i] <<' '<< setw(100) << WEBSIDES[i] << endl;
            idx++;
        }
        SetConsoleTextAttribute(hConsole, 7);
        cout << "【5星】\n";
        for (int i = 18; i < 28; i++)
        {
            SetConsoleTextAttribute(hConsole, 14);
            cout << idx << ". " << ALL_CHARACTERS[i] <<' '<< setw(100) << WEBSIDES[i]  << endl;
            idx++;
        }
        SetConsoleTextAttribute(hConsole, 7);
        cout << "【4星】\n";
        for (int i = 28; i < 33; i++)
        {
            SetConsoleTextAttribute(hConsole, 13);
            cout << idx << ". " << ALL_CHARACTERS[i] <<' '<< setw(100) << WEBSIDES[i]  << endl;
            idx++;
        }
        SetConsoleTextAttribute(hConsole, 7);
        //——————————————————————

        cout << "\n请输入序号：";
        int choice;
        cin >> choice;
        if (choice == 666) return;
        if (choice < 1 || choice > ALL_CHARACTERS.size())
        {
            cout << "序号无效\n";
            system("pause");
            continue;
        }
        string name = ALL_CHARACTERS[choice - 1];
        string english = ENGLISH[choice - 1];
        string filename = GetImagePath(english);

        //更新修改
        //先检查文件是否真的存在，便于区分“文件不存在”与“无法打开”
        DWORD attr = GetFileAttributesA(filename.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES)
        {
            cout << "图片不存在：" << filename << "\n请确认 CharacterImages 文件夹与 exe 放在同一目录下。\n";
            system("pause");
            continue;
        }
        if (attr & FILE_ATTRIBUTE_DIRECTORY)
        {
            cout << "该路径是文件夹而不是图片文件：" << filename << "\n";
            system("pause");
            continue;
        }
        HINSTANCE ret = ShellExecuteA(NULL, "open", filename.c_str(), NULL, NULL, SW_SHOWNORMAL);
        if ((INT_PTR)ret <= 32)
        {
            cout << "无法打开文件 " << filename << "（错误代码 " << (INT_PTR)ret << "）\n";
            if ((INT_PTR)ret == 5)
                cout << "错误代码5：访问被拒绝。常见原因：图片被其他程序占用、被杀毒软件拦截，或被系统安全策略限制。\n";
            else if ((INT_PTR)ret == 2 || (INT_PTR)ret == 3)
                cout << "错误代码" << (INT_PTR)ret << "：找不到文件或路径。\n";
            else if ((INT_PTR)ret == 31)
                cout << "错误代码31：该文件类型没有关联的打开程序。\n";
            system("pause");
        }
    }
}

int main()
{
    system("chcp 65001>nul");
    srand(time(0));
    memset(token,0,sizeof(token));
    do
    {
        system("cls");
        cout << "终末地抽卡模拟器\n最新终末地版本：1.5\n"
             << "输入1，进入抽卡\n"
             << "输入2，查看声明\n"
             << "输入3，查看已获得角色\n"
             << "输入4，角色图鉴\n"
             << "输入666，退出：";
        cin >> act;
        if(act == 1)
            GachaMain();
        else if(act == 2)
            Statement();
        else if(act == 3)
            ShowOwned();
        else if(act == 4)
            ShowCharacterIllustration();
        else if(act == 666)
            return 0;
        else
        {
            system("cls");
            cout <<"\n输入有误，请重新输入：";
            Sleep(1000);
        }
    }while(true);
}