/**
 * @file weather.c
 * @brief 天气数据解析模块源文件
 * @details 解析yytianqi.com的JSON天气数据含unicode转义解码
 * @author He
 * @date 2026-05-06
 */

#include "weather.h"

/* Unicode到GBK映射表 按Unicode排序用于二分查找 */
typedef struct { u16 uni; u8 hi; u8 lo; } UniToGbk;

static const UniToGbk uni2gbk[] = {
    {0x4e0a, 0xc9, 0xcf},  /* 上 */
    {0x4e1c, 0xb6, 0xab},  /* 东 */
    {0x4e2d, 0xd6, 0xd0},  /* 中 */
    {0x4e4b, 0xd6, 0xae},  /* 之 */
    {0x4e91, 0xd4, 0xc6},  /* 云 */
    {0x4eac, 0xbe, 0xa9},  /* 京 */
    {0x4f18, 0xd3, 0xc5},  /* 优 */
    {0x4f4e, 0xb5, 0xcd},  /* 低 */
    {0x4f5b, 0xb7, 0xf0},  /* 佛 */
    {0x5170, 0xc0, 0xbc},  /* 兰 */
    {0x51b0, 0xb1, 0xf9},  /* 冰 */
    {0x51b7, 0xc0, 0xe4},  /* 冷 */
    {0x51c9, 0xc1, 0xb9},  /* 凉 */
    {0x5206, 0xb7, 0xd6},  /* 分 */
    {0x5230, 0xb5, 0xbd},  /* 到 */
    {0x529b, 0xc1, 0xa6},  /* 力 */
    {0x5317, 0xb1, 0xb1},  /* 北 */
    {0x533a, 0xc7, 0xf8},  /* 区 */
    {0x5347, 0xc9, 0xfd},  /* 升 */
    {0x5357, 0xc4, 0xcf},  /* 南 */
    {0x53a6, 0xcf, 0xc3},  /* 厦 */
    {0x53ef, 0xbf, 0xc9},  /* 可 */
    {0x53f0, 0xcc, 0xa8},  /* 台 */
    {0x5408, 0xba, 0xcf},  /* 合 */
    {0x5411, 0xcf, 0xf2},  /* 向 */
    {0x5730, 0xb5, 0xd8},  /* 地 */
    {0x5733, 0xdb, 0xda},  /* 圳 */
    {0x591a, 0xb6, 0xe0},  /* 多 */
    {0x5927, 0xb4, 0xf3},  /* 大 */
    {0x5929, 0xcc, 0xec},  /* 天 */
    {0x5b81, 0xc4, 0xfe},  /* 宁 */
    {0x5b89, 0xb0, 0xb2},  /* 安 */
    {0x5b9c, 0xd2, 0xcb},  /* 宜 */
    {0x5c0f, 0xd0, 0xa1},  /* 小 */
    {0x5c18, 0xb3, 0xbe},  /* 尘 */
    {0x5c9b, 0xb5, 0xba},  /* 岛 */
    {0x5dde, 0xd6, 0xdd},  /* 州 */
    {0x5e72, 0xb8, 0xc9},  /* 干 */
    {0x5e7f, 0xb9, 0xe3},  /* 广 */
    {0x5e86, 0xc7, 0xec},  /* 庆 */
    {0x5ea6, 0xb6, 0xc8},  /* 度 */
    {0x5fae, 0xce, 0xa2},  /* 微 */
    {0x60e0, 0xbb, 0xdd},  /* 惠 */
    {0x610f, 0xd2, 0xe2},  /* 意 */
    {0x6210, 0xb3, 0xc9},  /* 成 */
    {0x626c, 0xd1, 0xef},  /* 扬 */
    {0x62c9, 0xc0, 0xad},  /* 拉 */
    {0x6301, 0xb3, 0xd6},  /* 持 */
    {0x6307, 0xd6, 0xb8},  /* 指 */
    {0x6570, 0xca, 0xfd},  /* 数 */
    {0x65b0, 0xd0, 0xc2},  /* 新 */
    {0x65e0, 0xce, 0xde},  /* 无 */
    {0x65f6, 0xca, 0xb1},  /* 时 */
    {0x6606, 0xc0, 0xa5},  /* 昆 */
    {0x660e, 0xc3, 0xf7},  /* 明 */
    {0x6668, 0xb3, 0xbf},  /* 晨 */
    {0x6674, 0xc7, 0xe7},  /* 晴 */
    {0x6696, 0xc5, 0xaf},  /* 暖 */
    {0x66b4, 0xb1, 0xa9},  /* 暴 */
    {0x6700, 0xd7, 0xee},  /* 最 */
    {0x6709, 0xd3, 0xd0},  /* 有 */
    {0x676d, 0xba, 0xbc},  /* 杭 */
    {0x6797, 0xc1, 0xd6},  /* 林 */
    {0x67d3, 0xc8, 0xbe},  /* 染 */
    {0x6842, 0xb9, 0xf0},  /* 桂 */
    {0x6a59, 0xb3, 0xc8},  /* 橙 */
    {0x6b66, 0xce, 0xe4},  /* 武 */
    {0x6c14, 0xc6, 0xf8},  /* 气 */
    {0x6c49, 0xba, 0xba},  /* 汉 */
    {0x6c61, 0xce, 0xdb},  /* 污 */
    {0x6c99, 0xc9, 0xb3},  /* 沙 */
    {0x6ce2, 0xb2, 0xa8},  /* 波 */
    {0x6ce8, 0xd7, 0xa2},  /* 注 */
    {0x6d25, 0xbd, 0xf2},  /* 津 */
    {0x6d53, 0xc5, 0xa8},  /* 浓 */
    {0x6d6e, 0xb8, 0xa1},  /* 浮 */
    {0x6d77, 0xba, 0xa3},  /* 海 */
    {0x6df1, 0xc9, 0xee},  /* 深 */
    {0x6e29, 0xce, 0xc2},  /* 温 */
    {0x6e2f, 0xb8, 0xdb},  /* 港 */
    {0x6e7f, 0xca, 0xaa},  /* 湿 */
    {0x6f6e, 0xb3, 0xb1},  /* 潮 */
    {0x6fb3, 0xb0, 0xc4},  /* 澳 */
    {0x70b9, 0xb5, 0xe3},  /* 点 */
    {0x70ed, 0xc8, 0xc8},  /* 热 */
    {0x723d, 0xcb, 0xac},  /* 爽 */
    {0x7586, 0xbd, 0xae},  /* 疆 */
    {0x767e, 0xb0, 0xd9},  /* 百 */
    {0x7a7a, 0xbf, 0xd5},  /* 空 */
    {0x7b49, 0xb5, 0xc8},  /* 等 */
    {0x7ea2, 0xba, 0xec},  /* 红 */
    {0x7ea7, 0xbc, 0xb6},  /* 级 */
    {0x7eed, 0xd0, 0xf8},  /* 续 */
    {0x80a5, 0xb7, 0xca},  /* 肥 */
    {0x80fd, 0xc4, 0xdc},  /* 能 */
    {0x826f, 0xc1, 0xbc},  /* 良 */
    {0x8272, 0xc9, 0xab},  /* 色 */
    {0x82cf, 0xcb, 0xd5},  /* 苏 */
    {0x839e, 0xdd, 0xb8},  /* 莞 */
    {0x8428, 0xc8, 0xf8},  /* 萨 */
    {0x84dd, 0xc0, 0xb6},  /* 蓝 */
    {0x8584, 0xb1, 0xa1},  /* 薄 */
    {0x897f, 0xce, 0xf7},  /* 西 */
    {0x89c1, 0xbc, 0xfb},  /* 见 */
    {0x8b66, 0xbe, 0xaf},  /* 警 */
    {0x8d28, 0xd6, 0xca},  /* 质 */
    {0x8d35, 0xb9, 0xf3},  /* 贵 */
    {0x8f6c, 0xd7, 0xaa},  /* 转 */
    {0x8f7b, 0xc7, 0xe1},  /* 轻 */
    {0x8f83, 0xbd, 0xcf},  /* 较 */
    {0x8fde, 0xc1, 0xac},  /* 连 */
    {0x90d1, 0xd6, 0xa3},  /* 郑 */
    {0x90e8, 0xb2, 0xbf},  /* 部 */
    {0x90fd, 0xb6, 0xbc},  /* 都 */
    {0x91cd, 0xd6, 0xd8},  /* 重 */
    {0x91cf, 0xc1, 0xbf},  /* 量 */
    {0x957f, 0xb3, 0xa4},  /* 长 */
    {0x95e8, 0xc3, 0xc5},  /* 门 */
    {0x95f4, 0xbc, 0xe4},  /* 间 */
    {0x9633, 0xd1, 0xf4},  /* 阳 */
    {0x9634, 0xd2, 0xf5},  /* 阴 */
    {0x9635, 0xd5, 0xf3},  /* 阵 */
    {0x96c4, 0xd0, 0xdb},  /* 雄 */
    {0x96e8, 0xd3, 0xea},  /* 雨 */
    {0x96ea, 0xd1, 0xa9},  /* 雪 */
    {0x96f7, 0xc0, 0xd7},  /* 雷 */
    {0x96fe, 0xce, 0xed},  /* 雾 */
    {0x971c, 0xcb, 0xaa},  /* 霜 */
    {0x973e, 0xf6, 0xb2},  /* 霾 */
    {0x9752, 0xc7, 0xe0},  /* 青 */
    {0x9884, 0xd4, 0xa4},  /* 预 */
    {0x98ce, 0xb7, 0xe7},  /* 风 */
    {0x9999, 0xcf, 0xe3},  /* 香 */
    {0x9ad8, 0xb8, 0xdf},  /* 高 */
    {0x9ec4, 0xbb, 0xc6},  /* 黄 */
};

#define UNI2GBK_SIZE  (sizeof(uni2gbk) / sizeof(uni2gbk[0]))


/**
 * @brief  将\uXXXX转义解码为GBK就地转换
 * @param  str: 输入输出字符串
 */
static void DecodeUnicode(char *str)
{
    char *src = str, *dst = str;
    u16 code;
    u8 i;

    while (*src)
    {
        if (src[0] == '\\' && src[1] == 'u')
        {
            code = 0;
            for (i = 0; i < 4; i++)
            {
                char c = src[2 + i];
                if (c >= '0' && c <= '9')       code = (code << 4) | (c - '0');
                else if (c >= 'a' && c <= 'f')  code = (code << 4) | (c - 'a' + 10);
                else if (c >= 'A' && c <= 'F')  code = (code << 4) | (c - 'A' + 10);
                else break;
            }

            if (i == 4)
            {
                /* 二分查找映射表 */
                s16 lo = 0, hi = UNI2GBK_SIZE - 1, mid;
                u16 found = 0xFFFF;
                while (lo <= hi)
                {
                    mid = (lo + hi) >> 1;
                    if (uni2gbk[mid].uni < code)      lo = mid + 1;
                    else if (uni2gbk[mid].uni > code) hi = mid - 1;
                    else { found = ((u16)uni2gbk[mid].hi << 8) | uni2gbk[mid].lo; break; }
                }

                if (found != 0xFFFF)
                {
                    *dst++ = (u8)(found >> 8);
                    *dst++ = (u8)(found & 0xFF);
                    src += 6;
                    continue;
                }
            }
        }
        *dst++ = *src++;
    }
    *dst = '\0';
}


/**
 * @brief  解析yytianqi.com返回的JSON天气数据
 * @param  json: 原始JSON字符串来自HTTP响应
 * @param  w: WeatherData结构体指针
 */
void Weather_Parse(char *json, WeatherData *w)
{
    char *p, *val;
    u8 i;

    /* 跳过HTTP头找到JSON主体 */
    char *json_body = strchr(json, '{');
    if (json_body)
    {
        memmove(json, json_body, strlen(json_body) + 1);
    }

    /* 解码unicode转义序列 */
    DecodeUnicode(json);

    /* cityName */
    p = strstr(json, "\"cityName\":\"");
    if (p)
    {
        val = p + 12;
        for (i = 0; *val && *val != '"' && i < sizeof(w->cityName) - 1; i++)
            w->cityName[i] = *val++;
        w->cityName[i] = '\0';
    }

    /* tq 天气现象 */
    p = strstr(json, "\"tq\":\"");
    if (p)
    {
        val = p + 6;
        for (i = 0; *val && *val != '"' && i < sizeof(w->tq) - 1; i++)
            w->tq[i] = *val++;
        w->tq[i] = '\0';
    }

    /* numtq 转 voice_file */
    p = strstr(json, "\"numtq\":\"");
    w->voice_file = 0;
    if (p)
    {
        val = p + 9;
        while (*val >= '0' && *val <= '9') { w->voice_file = w->voice_file * 10 + (*val - '0'); val++; }
    }

    /* qw 气温 */
    p = strstr(json, "\"qw\":\"");
    w->qw = 0;
    if (p)
    {
        val = p + 6;
        while (*val >= '0' && *val <= '9') { w->qw = w->qw * 10 + (*val - '0'); val++; }
    }

    /* sd 湿度 */
    p = strstr(json, "\"sd\":\"");
    w->sd = 0;
    if (p)
    {
        val = p + 6;
        while (*val >= '0' && *val <= '9') { w->sd = w->sd * 10 + (*val - '0'); val++; }
    }

    /* fl 风力 */
    p = strstr(json, "\"fl\":\"");
    if (p)
    {
        val = p + 6;
        for (i = 0; *val && *val != '"' && i < sizeof(w->fl) - 1; i++)
            w->fl[i] = *val++;
        w->fl[i] = '\0';
    }

    /* fx 风向 */
    p = strstr(json, "\"fx\":\"");
    if (p)
    {
        val = p + 6;
        for (i = 0; *val && *val != '"' && i < sizeof(w->fx) - 1; i++)
            w->fx[i] = *val++;
        w->fx[i] = '\0';
    }
}

/**
 * @brief  LCD显示天气信息
 * @param  w: 天气数据指针
 */
void Weather_Display(const WeatherData *w)
{
    char line[32];

    /* 清屏 */
    Lcd_Clear(0, 0, 320, 240, WHITE);

    /* 显示城市名 */
    if (strlen(w->cityName) > 0)
        Lcd_DisplayStr(10, 10, RED, WHITE, 32, (u8 *)w->cityName);

    /* 显示温度 */
    sprintf(line, "温度: %d C", w->qw);
    Lcd_DisplayStr(10, 55, BLACK, WHITE, 24, (u8 *)line);

    /* 显示天气状况 */
    if (strlen(w->tq) > 0)
        Lcd_DisplayStr(10, 90, BLUE, WHITE, 24, (u8 *)w->tq);

    /* 显示湿度 */
    sprintf(line, "湿度: %d%%", w->sd);
    Lcd_DisplayStr(10, 130, BLACK, WHITE, 16, (u8 *)line);

    /* 显示风向风力 */
    if (strlen(w->fx) > 0 && strlen(w->fl) > 0)
    {
        sprintf(line, "风向: %s %s", w->fx, w->fl);
        Lcd_DisplayStr(10, 150, BLACK, WHITE, 16, (u8 *)line);
    }
}
