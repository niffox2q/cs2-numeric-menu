#pragma once
#include "include/menus.h"

#define NUMERIC_MENU_INTERFACE "INumericMenuApi"

#define ITEM_HIDE 0
#define ITEM_DEFAULT 1
#define ITEM_DISABLED 2


class INumericMenuApi
{
public:
	virtual void AddItemMenu(Menu& hMenu, const char* sBack, const char* sText, int iType = 1)      = 0;
	virtual void DisplayPlayerMenu(Menu& hMenu, int iSlot, bool bClose = true)                      = 0;
	virtual void SetExitMenu(Menu& hMenu, bool bExit)                                               = 0;
	virtual void SetBackMenu(Menu& hMenu, bool bBack)                                               = 0;
	virtual void SetTitleMenu(Menu& hMenu, const char* szTitle)                                     = 0;
	virtual void SetCallback(Menu& hMenu, MenuCallbackFunc func)                                    = 0;
    virtual void ClosePlayerMenu(int iSlot)                                                         = 0;
    virtual std::string escapeString(const std::string& input)                                      = 0;
    virtual bool IsMenuOpen(int iSlot)                                                              = 0;
	virtual void DisplayPlayerMenu(Menu& hMenu, int iSlot, bool bClose = true, bool bReset = true)  = 0;
    virtual void AddRawItemMenu(Menu &hMenu, const char* sBack, const char* sText, int iType = 1)   = 0;
    virtual MenuType GetMenuType(int iSlot)                                                         = 0; // Always CENTER
};