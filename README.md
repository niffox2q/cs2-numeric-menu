Numeric Menu - плагин-ядро, который заменяет стандартные меню от Utils, делая управление в меню через цифровые бинды, которые пользователь должен самостоятельно установить.
Бинды полностью настраиваемые но не отменяют обычное использование кнопок.

ВАЖНО: Ядро экспериментально и находится в тестовой фазе, при багах просьба обращаться в дискорд: @niffox

Ключевые отличия:
- Интерфейс по центру экрана с нумерованием под бинды, навигация совершается полностью через цифровые бинды, по желанию можно изменить их в конфиге (1..9)
- Встроенный колорайзер текста, как титула так и текста кнопок.
- Компактная навигация внизу меню без использования HTML-кнопок или материалов с аддона.

Совместимость:
- API полностью совместимо с Utils, так что для установки достаточно заменить получаемый интерфейс и выставить цвета в файле переводов, чтобы меню выглядело красиво.

Поддерживаемые цвета:
{DEFAULT}
{RED}
{LIGHTPURPLE}
{GREEN}
{LIME}
{LIGHTGREEN}
{LIGHTRED}
{GRAY}
{LIGHTOLIVE}
{OLIVE}
{LIGHTBLUE}
{BLUE}
{PURPLE}
{GRAYBLUE}

Требования:
https://github.com/Pisex/cs2-menus/tree/main

Туториал по подмене с Utils API:
При объявлении переменной с API замените тип данных с IMenusApi* на INumericMenuApi*
IMenusApi* menus_api;
заменить  на
INumericMenuApi* menus_api;

При инициализации:
menus_api = (IMenusApi*)g_SMAPI->MetaFactory(Menus_INTERFACE, &ret, nullptr);
    if (ret == META_IFACE_FAILED) {
        META_CONPRINTF("%s | Missing UTILS plugin.",g_PLAPI->GetLogTag());
        engine->ServerCommand(("meta unload " + std::to_string(g_PLID)).c_str());
        return;
    }

Заменить на:
menus_api = (INumericMenuApi*)g_SMAPI->MetaFactory(NUMERIC_MENU_INTERFACE, &ret, nullptr);
    if (ret == META_IFACE_FAILED) {
        META_CONPRINTF("%s | Missing UTILS plugin.",g_PLAPI->GetLogTag());
        engine->ServerCommand(("meta unload " + std::to_string(g_PLID)).c_str());
        return;
    }

