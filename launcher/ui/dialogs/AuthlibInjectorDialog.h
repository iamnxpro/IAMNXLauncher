// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QDialog>

class QLabel;
class QPushButton;

/**
 * Explains why offline account skins need authlib-injector on some Minecraft
 * versions and downloads the pinned release on request.
 *
 * Nothing is downloaded automatically: the user has to press the button, so the
 * third party file is only ever fetched with explicit consent.
 */
class AuthlibInjectorDialog : public QDialog {
    Q_OBJECT

   public:
    explicit AuthlibInjectorDialog(QWidget* parent = nullptr);

   private:
    void refreshStatus();
    void download();

    QLabel* m_statusLabel = nullptr;
    QPushButton* m_downloadButton = nullptr;
};
