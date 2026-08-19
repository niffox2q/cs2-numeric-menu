#ifndef _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_ 

#include <ISmmPlugin.h>
#include <cstdio>
#include <sh_vector.h>
#include <iserver.h>
#include "vector.h"
#include <keyvalues.h>
#include <string>
#include <vector> 
class CBasePlayerController;
#include "CCSPlayerController.h"
#include "igameevents.h"
#include <complex>
#include <iomanip>
#include "metamod_oslink.h"
#include <ctime>
#include "entity2/entitykeyvalues.h"
#include "ctimer.h"
#include "networksystem/inetworksystem.h"   
#include <functional>

#include "include/menus.h"
#include "include/numeric_menu_api.h"

class NumericMenuMain final : public ISmmPlugin, public IMetamodListener {
public:
    bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
    bool Unload(char* error, size_t maxlen);
    void AllPluginsLoaded();
    void* OnMetamodQuery(const char* iface, int* ret) override;

private:
    const char* GetAuthor();
    const char* GetName();
    const char* GetDescription();
    const char* GetURL();
    const char* GetLicense();
    const char* GetVersion();
    const char* GetDate();
    const char* GetLogTag();
};

class NumericMenuApi : public INumericMenuApi {
private:

    

public:
    
    void AddItemMenu(Menu& hMenu, const char* sBack, const char* sText, int iType = 1)      override;
	void DisplayPlayerMenu(Menu& hMenu, int iSlot, bool bClose = true)                      override;
	void SetExitMenu(Menu& hMenu, bool bExit)                                               override;
	void SetBackMenu(Menu& hMenu, bool bBack)                                               override;
	void SetTitleMenu(Menu& hMenu, const char* szTitle)                                     override;
	void SetCallback(Menu& hMenu, MenuCallbackFunc func)                                    override;
    void ClosePlayerMenu(int iSlot)                                                         override;
    std::string escapeString(const std::string& input)                                      override;
    bool IsMenuOpen(int iSlot)                                                              override;
	void DisplayPlayerMenu(Menu& hMenu, int iSlot, bool bClose = true, bool bReset = true)  override;
    void AddRawItemMenu(Menu &hMenu, const char* sBack, const char* sText, int iType = 1)   override;
    MenuType GetMenuType(int iSlot)                                                         override;



};

extern NumericMenuApi g_NumericMenuApi;

#endif