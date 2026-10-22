/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchscreenimprovements_config.h"
#include "touchscreenimprovementsconfig.h"

#include <KPluginFactory>

#include <QCheckBox>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QLabel>
#include <QVBoxLayout>

K_PLUGIN_CLASS(TouchScreenImprovementsEffectConfig)

TouchScreenImprovementsEffectConfig::TouchScreenImprovementsEffectConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    TouchScreenImprovementsConfig::instance(QStringLiteral("kwinrc"));
    auto config = TouchScreenImprovementsConfig::self();

    auto layout = new QVBoxLayout(widget());

    // Widgets named kcfg_<Entry> are loaded, saved and reset to defaults by addConfig()
    const auto addOption = [&](const QString &entry, const QString &description) {
        auto checkBox = new QCheckBox(config->findItem(entry)->label(), widget());
        checkBox->setObjectName(QStringLiteral("kcfg_") + entry);
        layout->addWidget(checkBox);

        auto label = new QLabel(description, widget());
        label->setWordWrap(true);
        label->setContentsMargins(24, 0, 0, 8);
        label->setEnabled(false); // dimmed hint text
        layout->addWidget(label);
    };

    addOption(QStringLiteral("LongPressSelect"),
              QStringLiteral("Hold a finger still on text for a moment, then move it: the word under it is "
                             "highlighted and the selection grows word by word. Lifting opens the "
                             "right-click menu on the selection."));
    addOption(QStringLiteral("LongPressRightClick"),
              QStringLiteral("Hold a finger still for a moment and lift it to right-click there. When off, "
                             "apps get the long press themselves."));
    addOption(QStringLiteral("TapToShowKeyboard"),
              QStringLiteral("Scrolling over a text field, switching windows or long-pressing doesn't bring "
                             "up the on-screen keyboard; tapping the field does."));
    addOption(QStringLiteral("HideKeyboardOnPhysicalInput"),
              QStringLiteral("Moving or clicking a mouse or touchpad, or pressing a key on a real keyboard, "
                             "closes the on-screen keyboard, and typing on the real keyboard doesn't bring "
                             "it back. Volume, brightness and power keys don't count."));
    addOption(QStringLiteral("SwipeUpForKeyboard"),
              QStringLiteral("Brings up the on-screen keyboard even when no text field is selected; what you "
                             "type goes to the active window as key presses. With a right-click menu open, "
                             "closes the menu first, e.g. to delete highlighted text."));
    layout->addStretch();

    addConfig(config, widget());
}

void TouchScreenImprovementsEffectConfig::save()
{
    KCModule::save();

    // Apply right away
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/Effects"),
                                                  QStringLiteral("org.kde.kwin.Effects"),
                                                  QStringLiteral("reconfigureEffect"));
    message << QStringLiteral("touchscreenimprovements");
    QDBusConnection::sessionBus().send(message);
}

#include "touchscreenimprovements_config.moc"

#include "moc_touchscreenimprovements_config.cpp"
