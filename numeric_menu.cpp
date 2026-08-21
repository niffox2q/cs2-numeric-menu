#include "numeric_menu.h"
#include <random>
#include <cstdio>
#include <algorithm>


NumericMenuMain g_NumericMenuMain;
PLUGIN_EXPOSE(NumericMenuMain, g_NumericMenuMain);

NumericMenuApi g_NumericMenuApi;

// SYSTEM API`s
IVEngineServer2* engine = nullptr;
CGlobalVars* gpGlobals = nullptr;
CGameEntitySystem* g_pGameEntitySystem = nullptr;
CEntitySystem* g_pEntitySystem = nullptr;

// API
IUtilsApi* utils = nullptr;
IPlayersApi* players_api;

#define MAX_PLAYERS 64


struct Config_t
{
    std::string sFirstButton;
    std::string sSecondButton;
    std::string sThirdButton;
    std::string sFourthButton;
    std::string sFifthButton;
    std::string sSixthButton;

    std::string sBackButton;
    std::string sNextButton;
    std::string sExitButton;


    int iSoundType;
    std::string sButtonSound;
    std::string sBackButtonSound;
    std::string sNextButtonSound;
    std::string sExitButtonSound;
};

Config_t g_Config;


std::map<std::string, std::string> phrases;

// API

MenuPlayer g_NumMenuPlayer[MAX_PLAYERS];
std::string g_NumTextMenuPlayer[MAX_PLAYERS];

void GenerateMenuHtml(int iSlot);
void PrintToCenterHtml(int iSlot, int iDuration, const char* msg);
std::string ColorizeMenuText(std::string str);


void NumericMenuApi::AddItemMenu(Menu& hMenu, const char* sBack, const char* sText, int iType) {
    if (iType == ITEM_HIDE) return;

    Items hItem;
    hItem.iType = iType;
    hItem.sBack = sBack ? std::string(sBack) : "";
    
    hItem.sText = ColorizeMenuText(escapeString(sText ? sText : ""));
    
    hMenu.hItems.push_back(hItem);
}

void NumericMenuApi::AddRawItemMenu(Menu &hMenu, const char* sBack, const char* sText, int iType) {
    if (iType == ITEM_HIDE) return;

    Items hItem;
    hItem.iType = iType;
    hItem.sBack = sBack ? std::string(sBack) : "";
    hItem.sText = sText ? std::string(sText) : "";
    hMenu.hItems.push_back(hItem);
}

void NumericMenuApi::SetExitMenu(Menu& hMenu, bool bExit) {
    hMenu.bExit = bExit;
}

void NumericMenuApi::SetBackMenu(Menu& hMenu, bool bBack) {
    hMenu.bBack = bBack;
}

void NumericMenuApi::SetTitleMenu(Menu& hMenu, const char* szTitle) {
    hMenu.szTitle = szTitle ? std::string(szTitle) : "";
}

void NumericMenuApi::SetCallback(Menu& hMenu, MenuCallbackFunc func) {
    hMenu.hFunc = func;
}

void NumericMenuApi::ClosePlayerMenu(int iSlot) {
    if (iSlot < 0 || iSlot >= MAX_PLAYERS) return;
    
    g_NumMenuPlayer[iSlot].clear();
    g_NumTextMenuPlayer[iSlot].clear();
    
    PrintToCenterHtml(iSlot, 1, " "); 
}

bool NumericMenuApi::IsMenuOpen(int iSlot) {
    if (iSlot < 0 || iSlot >= MAX_PLAYERS) return false;
    return g_NumMenuPlayer[iSlot].bEnabled;
}

MenuType NumericMenuApi::GetMenuType(int iSlot) {
    return MenuType::CENTER; 
}

std::string NumericMenuApi::escapeString(const std::string& input) {
    std::string escaped;
    for (char c : input) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '\"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            case '<':  escaped += "&lt;"; break;
            case '>':  escaped += "&gt;"; break;
            case '%':  escaped += "%%"; break;
            default:   escaped += c; break;
        }
    }
    return escaped;
}

void NumericMenuApi::DisplayPlayerMenu(Menu& hMenu, int iSlot, bool bClose) {
    DisplayPlayerMenu(hMenu, iSlot, bClose, true);
}

void NumericMenuApi::DisplayPlayerMenu(Menu& hMenu, int iSlot, bool bClose, bool bReset) {
    if (iSlot < 0 || iSlot >= MAX_PLAYERS) return;
    
    MenuPlayer& mp = g_NumMenuPlayer[iSlot];
    
    if (mp.bEnabled && bClose && bReset) {
        mp.clear();
    }
    
    mp.bEnabled = true;
    mp.hMenu = hMenu;
    mp.iEnd = std::time(0) + 60;
    
    if (bReset) {
        mp.iList = 0; 
    } else {

        int ITEMS_PER_PAGE = 5;
        int maxPage = (hMenu.hItems.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE - 1;
        if (maxPage < 0) maxPage = 0;
        if (mp.iList > maxPage) mp.iList = maxPage;
    }
    

    GenerateMenuHtml(iSlot);
    
    PrintToCenterHtml(iSlot, 5, g_NumTextMenuPlayer[iSlot].c_str());
}

void LoadConfig() {
    KeyValues* config = new KeyValues("Config");
    const char* path = "addons/configs/NumericMenu.ini";
    
    if (!config->LoadFromFile(g_pFullFileSystem, path)) {
        utils->ErrorLog("%s Failed to load: %s", g_PLAPI->GetLogTag(), path);
        delete config;
        return;
    }

    g_Config.sFirstButton       = config->GetString("FirstButton", "mm_button1");
    g_Config.sSecondButton      = config->GetString("SecondButton", "mm_button2");
    g_Config.sThirdButton       = config->GetString("ThirdButton", "mm_button3");
    g_Config.sFourthButton      = config->GetString("FourthButton", "mm_button4");
    g_Config.sFifthButton       = config->GetString("FifthButton", "mm_button5");
    g_Config.sSixthButton       = config->GetString("SixthButton", "mm_button6");

    g_Config.sBackButton        = config->GetString("BackButton", "mm_backbutton");
    g_Config.sNextButton        = config->GetString("NextButton", "mm_nextbutton");
    g_Config.sExitButton        = config->GetString("ExitButton", "mm_exitbutton");

    g_Config.iSoundType         = config->GetInt("SoundType", 1);
    g_Config.sButtonSound       = config->GetString("ButtonSound", "");
    g_Config.sBackButtonSound   = config->GetString("BackButtonSound", "");
    g_Config.sNextButtonSound   = config->GetString("NextButtonSound", "");
    g_Config.sExitButtonSound   = config->GetString("ExitButtonSound", "");

    delete config;
}

void LoadTranslations() {
    phrases.clear();
    KeyValues* g_kvPhrases = new KeyValues("Phrases");
    const char *pszPath = "addons/translations/NumericMenu.phrases.txt";

    if (!g_kvPhrases->LoadFromFile(g_pFullFileSystem, pszPath))
    {
        utils->ErrorLog("%s Failed to load %s", g_PLAPI->GetLogTag(), pszPath);
        delete g_kvPhrases;
        return;
    }

    const char* language = utils->GetLanguage();

    for (KeyValues *pKey = g_kvPhrases->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey()) {
        phrases[std::string(pKey->GetName())] = std::string(pKey->GetString(language));
    }
    delete g_kvPhrases;
}

const char* GetTranslation(const char* key) {
    auto it = phrases.find(key);
    if (it == phrases.end()) return key;
    else return it->second.c_str();
}


void PrintSlotPrefixed(int iSlot, const char* content, ...) {
    if (!content || content[0] == '\0') return;

    char formattedContent[1024];
    va_list va;
    va_start(va, content);
    V_vsnprintf(formattedContent, sizeof(formattedContent), content, va);
    va_end(va);

    char finalMessage[2048];
    g_SMAPI->Format(finalMessage, sizeof(finalMessage), "%s %s", GetTranslation("Prefix"), formattedContent);


    utils->PrintToChat(iSlot, "%s", finalMessage);
}

void PrintAllPrefixed(const char* content, ...) {
    if (!content || content[0] == '\0') return;

    char formattedContent[1024];
    va_list va;
    va_start(va, content);
    V_vsnprintf(formattedContent, sizeof(formattedContent), content, va);
    va_end(va);

    char finalMessage[2048];
    g_SMAPI->Format(finalMessage, sizeof(finalMessage), "%s %s", GetTranslation("Prefix"), formattedContent);

    utils->PrintToChatAll("%s",finalMessage);
}

CCSPlayerController* GetController(int iSlot) {
    auto pController = CCSPlayerController::FromSlot(iSlot);
    if (!pController || !pController->IsConnected()) return nullptr;
    return pController;
}

CCSPlayerPawn* GetPawn(int iSlot) {
    auto controller = GetController(iSlot);
    if (controller) {
        auto pawn =controller->GetPlayerPawn();
        if (pawn) {
            return pawn;
        }
    }
    return nullptr;
}


std::string ColorizeMenuText(std::string str) {
    std::map<std::string, std::string> colors = {
        {"{DEFAULT}",     "</font>"},                
        {"{RED}",         "<font color='#FF4040'>"},   
        {"{LIGHTPURPLE}", "<font color='#CC99FF'>"},   
        {"{GREEN}",       "<font color='#40FF40'>"},  
        {"{LIME}",        "<font color='#BFFF00'>"},  
        {"{LIGHTGREEN}",  "<font color='#99FF99'>"},   
        {"{LIGHTRED}",    "<font color='#FF8080'>"},   
        {"{GRAY}",        "<font color='#808080'>"},    
        {"{LIGHTOLIVE}",  "<font color='#E0E68A'>"},   
        {"{OLIVE}",       "<font color='#9ACD32'>"},   
        {"{LIGHTBLUE}",   "<font color='#99CCFF'>"},  
        {"{BLUE}",        "<font color='#4040FF'>"},   
        {"{PURPLE}",      "<font color='#800080'>"},   
        {"{GRAYBLUE}",    "<font color='#99CCDA'>"}  
    };

    for (const auto& pair : colors) {
        size_t pos = 0;
        while ((pos = str.find(pair.first, pos)) != std::string::npos) {
            str.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length(); 
        }
    }
    return str;
}


void PrintToCenterHtml(int iSlot, int iDuration, const char* msg) {
    IGameEventManager2* gameeventmanager = utils->GetGameEventManager();
    if (!gameeventmanager) return;
    

    IGameEvent* pEvent = gameeventmanager->CreateEvent("show_survival_respawn_status");
    if (!pEvent) return;
    
    pEvent->SetString("loc_token", msg);
    pEvent->SetInt("duration", iDuration);
    pEvent->SetInt("userid", iSlot); 
    
    gameeventmanager->FireEvent(pEvent);
}

void GenerateMenuHtml(int iSlot) {
    MenuPlayer& mp = g_NumMenuPlayer[iSlot];
    Menu& m = mp.hMenu;
    
    std::string title = ColorizeMenuText(m.szTitle);

    std::string html = "<div align='center'><font class='fontSize-md'><b>" + title + "</b></font><br>";
    
    int ITEMS_PER_PAGE = 5;
    int iTotalItems = m.hItems.size();
    int startIndex = mp.iList * ITEMS_PER_PAGE;
    int endIndex = std::min(startIndex + ITEMS_PER_PAGE, iTotalItems);


    int currentCount = 0;
    for (int i = startIndex; i < endIndex; i++) {
        int btnNumber = (i - startIndex) + 1;

        if (m.hItems[i].iType == ITEM_DISABLED) {
            html += "<font class='fontSize-sm' color='gray'>" + std::to_string(btnNumber) + ". " + m.hItems[i].sText + "</font><br>";
        } else {
            html += "<font class='fontSize-sm' color='yellow'>" + std::to_string(btnNumber) + ". </font> <font class='fontSize-sm' color='#ffffff'>" + m.hItems[i].sText + "</font><br>";
        }
        currentCount++;
    }
    

    
    std::string nav = "<br>"; 
    
    if (mp.iList > 0 || m.bBack) {
        std::string backTxt = ColorizeMenuText(std::string(GetTranslation("Menu_Back")));
        nav += "<font class='fontSize-sm'>" + backTxt + "</font>&nbsp;&nbsp;&nbsp;&nbsp;";
    }
    
    if (endIndex < iTotalItems) {
        std::string nextTxt = ColorizeMenuText(std::string(GetTranslation("Menu_Next")));
        nav += "<font class='fontSize-sm'>" + nextTxt + "</font>&nbsp;&nbsp;&nbsp;&nbsp;";
    }
    
    if (m.bExit) {
        std::string exitTxt = ColorizeMenuText(std::string(GetTranslation("Menu_Exit")));
        nav += "<font class='fontSize-sm'>" + exitTxt + "</font>";
    }
    
    html += nav + "</div>";
    
    g_NumTextMenuPlayer[iSlot] = html;
}


void PlayMenuSound(int iSlot, const std::string& soundPath) {
    if (soundPath.empty()) return;
    
    if (g_Config.iSoundType == 1) {
        auto pPawn = GetPawn(iSlot);
        if (pPawn) {
            players_api->EmitSound(iSlot, pPawn->entindex(), soundPath, 100, 1.0f);
        }
    } else if (g_Config.iSoundType == 2) {
        engine->ClientCommand(iSlot, "play %s", soundPath.c_str());
    }
}

void HandleMenuInput(int iSlot, int iButton) {
    if (iSlot < 0 || iSlot >= MAX_PLAYERS) return;

    MenuPlayer& hMenuPlayer = g_NumMenuPlayer[iSlot];
    if (!hMenuPlayer.bEnabled) return; 

    Menu& hMenu = hMenuPlayer.hMenu;
    
    const int ITEMS_PER_PAGE = 5;
    int iTotalItems = hMenu.hItems.size();
    
    int iTotalPages = (iTotalItems + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE; 
    if (iTotalPages == 0) iTotalPages = 1;

    hMenuPlayer.iEnd = std::time(0) + 60; 

    if (iButton == 9) {
        if (hMenu.bExit) {
            PlayMenuSound(iSlot, g_Config.sExitButtonSound);
            
            auto callback = hMenu.hFunc;
            g_NumericMenuApi.ClosePlayerMenu(iSlot);
            
            if (callback) {
                callback("exit", "exit", 9, iSlot);
            }
        }
        return;
    }

    if (iButton == 8) {
        if (hMenuPlayer.iList + 1 < iTotalPages) {
            hMenuPlayer.iList++;
            PlayMenuSound(iSlot, g_Config.sNextButtonSound);
            
            g_NumericMenuApi.DisplayPlayerMenu(hMenu, iSlot, false, false);
            
            if (hMenu.hFunc) {
                hMenu.hFunc("next", "next", 8, iSlot);
            }
        }
        return;
    }

    if (iButton == 7) {
        if (hMenuPlayer.iList > 0) { 
            hMenuPlayer.iList--;
            PlayMenuSound(iSlot, g_Config.sBackButtonSound);
            
            g_NumericMenuApi.DisplayPlayerMenu(hMenu, iSlot, false, false);
        }
        else if (hMenuPlayer.iList == 0 && hMenu.bBack) {
            PlayMenuSound(iSlot, g_Config.sBackButtonSound);
            
            auto callback = hMenu.hFunc;

            if (callback) {
                callback("back", "back", 7, iSlot);
            }
        }
        return;
    }

    if (iButton >= 1 && iButton <= 6) {
        int iIndex = (hMenuPlayer.iList * ITEMS_PER_PAGE) + (iButton - 1);
        
        if (iIndex < iTotalItems) {
            if (hMenu.hItems[iIndex].iType == ITEM_DEFAULT) { 
                PlayMenuSound(iSlot, g_Config.sButtonSound);
                
                auto callback = hMenu.hFunc;
                std::string sBack = hMenu.hItems[iIndex].sBack;
                std::string sText = hMenu.hItems[iIndex].sText;
                
                if (callback) {
                    callback(sBack.c_str(), sText.c_str(), iButton, iSlot);
                }
            }
        }
        return;
    }
}




CGameEntitySystem* GameEntitySystem() {
    return utils ? utils->GetCGameEntitySystem() : nullptr;
}

void StartupServer() {
    g_pGameEntitySystem = GameEntitySystem();
    g_pEntitySystem = utils->GetCEntitySystem();
    gpGlobals = utils->GetCGlobalVars();

}


bool OnMenuTest(int iSlot, const char* content) {
    Menu hMenu;

    g_NumericMenuApi.SetTitleMenu(hMenu,"{RED}Test Menu{DEFAULT}");

    g_NumericMenuApi.AddItemMenu(hMenu,"hello","Say {BLUE}Hel{DEFAULT}lo");
    g_NumericMenuApi.AddItemMenu(hMenu,"","No Hello",ITEM_DISABLED);
    g_NumericMenuApi.AddItemMenu(hMenu,"3","3");
    g_NumericMenuApi.AddItemMenu(hMenu,"4","4");
    g_NumericMenuApi.AddItemMenu(hMenu,"5","5");
    g_NumericMenuApi.AddItemMenu(hMenu,"6","6");
    g_NumericMenuApi.AddItemMenu(hMenu,"7","7");
    g_NumericMenuApi.AddItemMenu(hMenu,"8","8");

    g_NumericMenuApi.SetExitMenu(hMenu,true);
    
    g_NumericMenuApi.SetCallback(hMenu,[](const char* szBack, const char* szFront, int iItem, int iSlot){
        if (strcmp(szBack,"exit") == 0) {
            g_NumericMenuApi.ClosePlayerMenu(iSlot);
            return;
        }

        if (strcmp(szBack,"hello") == 0) {
            PrintSlotPrefixed(iSlot,"Hello There!");
            return;
        }
    });

    g_NumericMenuApi.DisplayPlayerMenu(hMenu,iSlot,true,true);

    return  true;
}

void NumericMenuMain::AllPluginsLoaded() {
    int ret;
    utils = (IUtilsApi*)g_SMAPI->MetaFactory(Utils_INTERFACE, &ret, nullptr);
    if (ret == META_IFACE_FAILED) {
        META_CONPRINTF("%s | Missing UTILS plugin.\n", g_PLAPI->GetLogTag());
        engine->ServerCommand(("meta unload " + std::to_string(g_PLID)).c_str());
        return;
    }

    players_api = (IPlayersApi*)g_SMAPI->MetaFactory(PLAYERS_INTERFACE, &ret, nullptr);
    if (ret == META_IFACE_FAILED) {
        META_CONPRINTF("%s | Missing UTILS plugin.",g_PLAPI->GetLogTag());
        engine->ServerCommand(("meta unload " + std::to_string(g_PLID)).c_str());
        return;
    }


    LoadConfig();
    LoadTranslations();

    utils->HookEvent(g_PLID, "player_disconnect", [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast) {
        int iSlot = pEvent->GetInt("userid"); 
        if (iSlot >= 0 && iSlot < MAX_PLAYERS) {
            g_NumericMenuApi.ClosePlayerMenu(iSlot);
        }
    });

    utils->RegCommand(g_PLID, {g_Config.sFirstButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 1);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sSecondButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 2);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sThirdButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 3);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sFourthButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 4);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sFifthButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 5);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sSixthButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 6);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sBackButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 7);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sNextButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 8);
        return true;
    });
    utils->RegCommand(g_PLID, {g_Config.sExitButton.c_str()}, {}, [](int iSlot, const char* content){
        HandleMenuInput(iSlot, 9);
        return true;
    });

    utils->CreateTimer(1.0f, []() {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (g_NumMenuPlayer[i].bEnabled) {
                if (std::time(0) >= g_NumMenuPlayer[i].iEnd) {
                    g_NumericMenuApi.ClosePlayerMenu(i);
                    continue;
                }
                

                if (!g_NumTextMenuPlayer[i].empty()) {
                    PrintToCenterHtml(i, 5, g_NumTextMenuPlayer[i].c_str());
                }
            }
        }
        return 1.0f;
    });

    
    // utils->HookEvent(g_PLID, "player_death", [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast) {
    //     int iSlot = pEvent->GetInt("userid");
    //     if (iSlot >= 0 && iSlot < MAX_PLAYERS) {
    //         g_NumericMenuApi.ClosePlayerMenu(iSlot);
    //     }
    // });

    utils->RegCommand(g_PLID,{""},{"!mt"},OnMenuTest);


    utils->StartupServer(g_PLID, StartupServer);
}

bool NumericMenuMain::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) {
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetServerFactory, g_pSource2Server, ISource2Server, SOURCE2SERVER_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetServerFactory, g_pSource2GameClients, IServerGameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetServerFactory, g_pSource2GameEntities, ISource2GameEntities, SOURCE2GAMEENTITIES_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkSystem, INetworkSystem, NETWORKSYSTEM_INTERFACE_VERSION);


    ConVar_Register(FCVAR_SERVER_CAN_EXECUTE | FCVAR_GAMEDLL);
    g_SMAPI->AddListener(this, this);

    

    return true;
}

void* NumericMenuMain::OnMetamodQuery(const char* iface, int* ret) {
    if (strcmp(iface, NUMERIC_MENU_INTERFACE) == 0) {
        *ret = META_IFACE_OK;
        return &g_NumericMenuApi;
    }

    *ret = META_IFACE_FAILED;
    return nullptr;
}

bool NumericMenuMain::Unload(char* error, size_t maxlen) {

    utils->ClearAllHooks(g_PLID);
    ConVar_Unregister();
    return true;
}

const char* NumericMenuMain::GetAuthor() { return "niffox"; }
const char* NumericMenuMain::GetDate() { return __DATE__; }
const char* NumericMenuMain::GetDescription() { return "Numeric Menu"; }
const char* NumericMenuMain::GetLicense() { return "Paid"; }
const char* NumericMenuMain::GetLogTag() { return "Numeric Menu"; }
const char* NumericMenuMain::GetName() { return "Numeric Menu"; }
const char* NumericMenuMain::GetURL() { return "https://t.me/niffox_2q"; }
const char* NumericMenuMain::GetVersion() { return "1.0.0"; }