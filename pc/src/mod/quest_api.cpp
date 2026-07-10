#include "pc_mod_quest_api.h"
#include "pc_mod_quest_internal.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_lua_internal.h"

extern "C" {
#include "m_common_data.h"
#include "m_soncho.h"
#include "m_private.h"
#include "lb_rtc.h"
#include "m_time.h"
#include "m_room_type.h"
#include "m_lib.h"
#include "m_name_table.h"
}

static void pc_mod_quest_require_save(lua_State* L) {
    if (!pc_mod_save_is_ready()) {
        luaL_error(L, "Quest API requires a loaded save");
    }
}

static int pc_mod_quest_opt_player(lua_State* L, int arg) {
    if (lua_isnoneornil(L, arg)) {
        return (int)Common_Get(player_no);
    }
    return pc_mod_check_player_index(L, arg, 0);
}

static void pc_mod_quest_sync_rtc(const lbRTC_time_c* src) {
    lbRTC_time_c set_time;

    lbRTC_TimeCopy(&set_time, src);
    set_time.weekday = lbRTC_Week(set_time.year, set_time.month, set_time.day);
    lbRTC_SetTime(&set_time);
    lbRTC_GetTime(Common_GetPointer(time.rtc_time));
    Common_Set(time.now_sec, Common_Get(time.rtc_time.sec) + Common_Get(time.rtc_time.min) * mTM_SECONDS_IN_MINUTE +
                                  Common_Get(time.rtc_time.hour) * mTM_SECONDS_IN_HOUR);
}

static int l_lh_get_period(lua_State* L) {
    pc_mod_quest_require_save(L);
    lua_pushinteger(L, mSC_LightHouse_get_period(Common_GetPointer(time.rtc_time)));
    return 1;
}

static int l_lh_is_active(lua_State* L) {
    pc_mod_quest_require_save(L);
    lua_pushboolean(L, mSC_LightHouse_get_period(Common_GetPointer(time.rtc_time)) != mSC_LIGHTHOUSE_PERIOD_NONE);
    return 1;
}

static int l_lh_get_day(lua_State* L) {
    pc_mod_quest_require_save(L);
    lua_pushinteger(L, mSC_LightHouse_day(Common_GetPointer(time.rtc_time)));
    return 1;
}

static int l_lh_get_renew_date(lua_State* L) {
    LightHouse_c* lh;

    pc_mod_quest_require_save(L);
    lh = Save_GetPointer(LightHouse);
    lua_pushinteger(L, lh->renew_time.year);
    lua_pushinteger(L, lh->renew_time.month);
    lua_pushinteger(L, lh->renew_time.day);
    return 3;
}

static int l_lh_get_days_on(lua_State* L) {
    pc_mod_quest_require_save(L);
    lua_pushinteger(L, Save_Get(LightHouse).days_switched_on);
    return 1;
}

static int l_lh_start(lua_State* L) {
    pc_mod_quest_require_save(L);
    mSC_LightHouse_Quest_Start();
    lua_pushboolean(L, 1);
    return 1;
}

static int l_lh_can_switch(lua_State* L) {
    pc_mod_quest_require_save(L);
    lua_pushboolean(L, mSC_LightHouse_In_Check());
    return 1;
}

static int l_lh_switch_on(lua_State* L) {
    pc_mod_quest_require_save(L);
    mSC_LightHouse_Switch_On();
    lua_pushboolean(L, 1);
    return 1;
}

static int l_lh_player_started(lua_State* L) {
    int player_no;
    LightHouse_c* lh;

    pc_mod_quest_require_save(L);
    player_no = pc_mod_quest_opt_player(L, 1);
    lh = Save_GetPointer(LightHouse);
    lua_pushboolean(L, player_no < mPr_FOREIGNER && (lh->players_quest_started & (1 << player_no)) != 0);
    return 1;
}

static int l_lh_player_contributed(lua_State* L) {
    int player_no;
    LightHouse_c* lh;

    pc_mod_quest_require_save(L);
    player_no = pc_mod_quest_opt_player(L, 1);
    lh = Save_GetPointer(LightHouse);
    lua_pushboolean(L, player_no < mPr_FOREIGNER && (lh->players_quest_started & (1 << (player_no + 4))) != 0);
    return 1;
}

static int l_lh_player_completed(lua_State* L) {
    int player_no;
    LightHouse_c* lh;

    pc_mod_quest_require_save(L);
    player_no = pc_mod_quest_opt_player(L, 1);
    lh = Save_GetPointer(LightHouse);
    lua_pushboolean(L, player_no < mPr_FOREIGNER && (lh->players_completed & (1 << player_no)) != 0);
    return 1;
}

static int l_lh_get_event(lua_State* L) {
    int player_no;

    pc_mod_quest_require_save(L);
    player_no = pc_mod_quest_opt_player(L, 1);
    lua_pushinteger(L, mSC_LightHouse_Event_Check(player_no));
    return 1;
}

static int l_lh_clear_event(lua_State* L) {
    int player_no;

    pc_mod_quest_require_save(L);
    player_no = pc_mod_quest_opt_player(L, 1);
    mSC_LightHouse_Event_Clear(player_no);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_lh_get_present_item(lua_State* L) {
    int player_no;
    mActor_name_t item;

    pc_mod_quest_require_save(L);
    player_no = pc_mod_quest_opt_player(L, 1);
    item = mSC_LightHouse_Event_Present_Item((u32)player_no);
    lua_pushinteger(L, item);
    return 1;
}

static int l_lh_clear(lua_State* L) {
    pc_mod_quest_require_save(L);
    mem_clear((u8*)Save_GetPointer(LightHouse), sizeof(LightHouse_c), 0);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_lh_setup_for_switch(lua_State* L) {
    LightHouse_c* lh;
    lbRTC_time_c rtc;

    pc_mod_quest_require_save(L);

    mSC_LightHouse_Quest_Start();

    lbRTC_GetTime(&rtc);
    lbRTC_Add_DD(&rtc, 1);
    rtc.hour = 20;
    rtc.min = 0;
    rtc.sec = 0;
    pc_mod_quest_sync_rtc(&rtc);

    lh = Save_GetPointer(LightHouse);
    lh->days_switched_on = 0;
    lh->players_quest_started &= 0x0F;
    lh->players_completed = 0;
    mRmTp_IndexLightSwitchOFF(mRmTp_LIGHT_SWITCH_LIGHTHOUSE);

    lua_pushboolean(L, 1);
    return 1;
}

static const luaL_Reg lighthouse_module[] = {
    { "GetPeriod",         l_lh_get_period },
    { "IsActive",          l_lh_is_active },
    { "GetDay",            l_lh_get_day },
    { "GetRenewDate",      l_lh_get_renew_date },
    { "GetDaysOn",         l_lh_get_days_on },
    { "Start",             l_lh_start },
    { "CanSwitch",         l_lh_can_switch },
    { "SwitchOn",          l_lh_switch_on },
    { "PlayerStarted",     l_lh_player_started },
    { "PlayerContributed", l_lh_player_contributed },
    { "PlayerCompleted",   l_lh_player_completed },
    { "GetEvent",          l_lh_get_event },
    { "ClearEvent",        l_lh_clear_event },
    { "GetPresentItem",    l_lh_get_present_item },
    { "Clear",             l_lh_clear },
    { "SetupForSwitch",    l_lh_setup_for_switch },
    { nullptr,             nullptr },
};

extern "C" void pc_mod_register_quest_api(lua_State* L) {
    pc_mod_register_quest_typed(L);

    lua_newtable(L);
    lua_newtable(L);
    luaL_register(L, nullptr, lighthouse_module);
    lua_setfield(L, -2, "Lighthouse");
    lua_setglobal(L, "Quest");
}
