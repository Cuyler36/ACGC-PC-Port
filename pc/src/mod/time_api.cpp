#include "pc_mod_time_api.h"
#include "pc_mod_lua_internal.h"

extern "C" {
#include "m_common_data.h"
#include "lb_rtc.h"
#include "m_time.h"
}

static void pc_mod_time_get(lbRTC_time_c* out) {
    lbRTC_GetTime(out);
}

static int pc_mod_time_apply(const lbRTC_time_c* src) {
    lbRTC_time_c set_time;

    lbRTC_TimeCopy(&set_time, src);
    set_time.weekday = lbRTC_Week(set_time.year, set_time.month, set_time.day);

    if (!lbRTC_IsValidTime(&set_time)) {
        return 0;
    }

    lbRTC_SetTime(&set_time);
    lbRTC_GetTime(Common_GetPointer(time.rtc_time));
    Common_Set(time.now_sec, Common_Get(time.rtc_time.sec) + Common_Get(time.rtc_time.min) * mTM_SECONDS_IN_MINUTE +
                                  Common_Get(time.rtc_time.hour) * mTM_SECONDS_IN_HOUR);
    return 1;
}

static int l_time_get_second(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.sec);
    return 1;
}

static int l_time_get_minute(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.min);
    return 1;
}

static int l_time_get_hour(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.hour);
    return 1;
}

static int l_time_get_day(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.day);
    return 1;
}

static int l_time_get_month(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.month);
    return 1;
}

static int l_time_get_year(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.year);
    return 1;
}

static int l_time_get_weekday(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.weekday);
    return 1;
}

static int l_time_get_date(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.year);
    lua_pushinteger(L, time.month);
    lua_pushinteger(L, time.day);
    return 3;
}

static int l_time_get_time(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.hour);
    lua_pushinteger(L, time.min);
    lua_pushinteger(L, time.sec);
    return 3;
}

static int l_time_get_date_time(lua_State* L) {
    lbRTC_time_c time;

    pc_mod_time_get(&time);
    lua_pushinteger(L, time.year);
    lua_pushinteger(L, time.month);
    lua_pushinteger(L, time.day);
    lua_pushinteger(L, time.hour);
    lua_pushinteger(L, time.min);
    lua_pushinteger(L, time.sec);
    return 6;
}

static int l_time_set_second(lua_State* L) {
    lbRTC_time_c time;
    int sec = (int)luaL_checkinteger(L, 1);

    pc_mod_time_get(&time);
    time.sec = (u8)sec;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_minute(lua_State* L) {
    lbRTC_time_c time;
    int min = (int)luaL_checkinteger(L, 1);

    pc_mod_time_get(&time);
    time.min = (u8)min;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_hour(lua_State* L) {
    lbRTC_time_c time;
    int hour = (int)luaL_checkinteger(L, 1);

    pc_mod_time_get(&time);
    time.hour = (u8)hour;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_day(lua_State* L) {
    lbRTC_time_c time;
    int day = (int)luaL_checkinteger(L, 1);

    pc_mod_time_get(&time);
    time.day = (u8)day;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_month(lua_State* L) {
    lbRTC_time_c time;
    int month = (int)luaL_checkinteger(L, 1);

    pc_mod_time_get(&time);
    time.month = (u8)month;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_year(lua_State* L) {
    lbRTC_time_c time;
    int year = (int)luaL_checkinteger(L, 1);

    pc_mod_time_get(&time);
    time.year = (u16)year;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_date(lua_State* L) {
    lbRTC_time_c time;
    int year = (int)luaL_checkinteger(L, 1);
    int month = (int)luaL_checkinteger(L, 2);
    int day = (int)luaL_checkinteger(L, 3);

    pc_mod_time_get(&time);
    time.year = (u16)year;
    time.month = (u8)month;
    time.day = (u8)day;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_time(lua_State* L) {
    lbRTC_time_c time;
    int hour = (int)luaL_checkinteger(L, 1);
    int min = (int)luaL_checkinteger(L, 2);
    int sec = (int)luaL_optinteger(L, 3, 0);

    pc_mod_time_get(&time);
    time.hour = (u8)hour;
    time.min = (u8)min;
    time.sec = (u8)sec;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static int l_time_set_date_time(lua_State* L) {
    lbRTC_time_c time;
    int year = (int)luaL_checkinteger(L, 1);
    int month = (int)luaL_checkinteger(L, 2);
    int day = (int)luaL_checkinteger(L, 3);
    int hour = (int)luaL_checkinteger(L, 4);
    int min = (int)luaL_checkinteger(L, 5);
    int sec = (int)luaL_optinteger(L, 6, 0);

    pc_mod_time_get(&time);
    time.year = (u16)year;
    time.month = (u8)month;
    time.day = (u8)day;
    time.hour = (u8)hour;
    time.min = (u8)min;
    time.sec = (u8)sec;
    lua_pushboolean(L, pc_mod_time_apply(&time));
    return 1;
}

static const luaL_Reg time_module[] = {
    { "GetSecond",   l_time_get_second },
    { "GetMinute",   l_time_get_minute },
    { "GetHour",     l_time_get_hour },
    { "GetDay",      l_time_get_day },
    { "GetMonth",    l_time_get_month },
    { "GetYear",     l_time_get_year },
    { "GetWeekday",  l_time_get_weekday },
    { "GetDate",     l_time_get_date },
    { "GetTime",     l_time_get_time },
    { "GetDateTime", l_time_get_date_time },
    { "SetSecond",   l_time_set_second },
    { "SetMinute",   l_time_set_minute },
    { "SetHour",     l_time_set_hour },
    { "SetDay",      l_time_set_day },
    { "SetMonth",    l_time_set_month },
    { "SetYear",     l_time_set_year },
    { "SetDate",     l_time_set_date },
    { "SetTime",     l_time_set_time },
    { "SetDateTime", l_time_set_date_time },
    { nullptr,       nullptr },
};

extern "C" void pc_mod_register_time_api(lua_State* L) {
    luaL_register(L, "Time", time_module);
    lua_pop(L, 1);
}
