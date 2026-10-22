/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>

/**
 * Settings page behind the configure button of the effect in
 * System Settings > Desktop Effects.
 */
class TouchScreenImprovementsEffectConfig : public KCModule
{
    Q_OBJECT

public:
    explicit TouchScreenImprovementsEffectConfig(QObject *parent, const KPluginMetaData &data);

    void save() override;
};
