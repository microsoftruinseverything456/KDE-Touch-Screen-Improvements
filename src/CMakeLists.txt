# SPDX-License-Identifier: BSD-3-Clause

# Effect
kcoreaddons_add_plugin(touchscreenimprovements INSTALL_NAMESPACE "kwin/effects/plugins")

ecm_qt_declare_logging_category(touchscreenimprovements
    HEADER touchscreenimprovements_debug.h
    IDENTIFIER KWIN_TOUCHSCREENIMPROVEMENTS
    CATEGORY_NAME kwin_touchscreenimprovements
    DEFAULT_SEVERITY Warning
)

set(touchscreenimprovements_SOURCES
    main.cpp
    physicalinputcore.cpp
    taptoshowkeyboardcore.cpp
    touchlongpresscore.cpp
    touchscreenimprovementseffect.cpp
)
kconfig_add_kcfg_files(touchscreenimprovements_SOURCES touchscreenimprovementsconfig.kcfgc)
target_sources(touchscreenimprovements PRIVATE ${touchscreenimprovements_SOURCES})
target_link_libraries(touchscreenimprovements PRIVATE KWin::kwin KF6::ConfigGui)
# KWIN_EFFECT_FACTORY expands to a Q_OBJECT class
set_property(TARGET touchscreenimprovements APPEND PROPERTY AUTOMOC_MACRO_NAMES KWIN_EFFECT_FACTORY)

# Settings page (configure button in System Settings > Desktop Effects)
kcoreaddons_add_plugin(kwin_touchscreenimprovements_config INSTALL_NAMESPACE "kwin/effects/configs")

set(kwin_touchscreenimprovements_config_SOURCES
    kcm/touchscreenimprovements_config.cpp
)
kconfig_add_kcfg_files(kwin_touchscreenimprovements_config_SOURCES touchscreenimprovementsconfig.kcfgc)
target_sources(kwin_touchscreenimprovements_config PRIVATE ${kwin_touchscreenimprovements_config_SOURCES})
target_include_directories(kwin_touchscreenimprovements_config PRIVATE kcm)
target_link_libraries(kwin_touchscreenimprovements_config PRIVATE
    Qt6::DBus
    Qt6::Widgets
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::KCMUtils
)
