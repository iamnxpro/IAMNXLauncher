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

#include <QString>

#include "net/NetJob.h"
#include "tasks/Task.h"

/**
 * Downloads the authlib-injector java agent into the launcher's jar folder.
 *
 * Only the release that is pinned at build time is ever downloaded, and the
 * result is verified against the pinned SHA-256 checksum. A file that does not
 * match is rejected instead of being handed to the JVM as "-javaagent".
 *
 * authlib-injector is a third party project licensed under the AGPL-3.0 with
 * the "Authlib-Injector" exception. It is not part of the launcher and is only
 * fetched when the user explicitly asks for it.
 */
class AuthlibInjectorDownload : public Task {
    Q_OBJECT

   public:
    explicit AuthlibInjectorDownload(QString destination, QObject* parent = nullptr);
    ~AuthlibInjectorDownload() override = default;

    /// Name of the file the launcher looks for.
    static QString fileName();
    /// The jar the launcher would use right now. Empty when it is missing.
    static QString installedPath();
    /// Writable location the jar gets downloaded to. Empty when there is none.
    static QString targetPath();

    bool canAbort() const override { return true; }
    bool abort() override;

   protected:
    void executeTask() override;

   private slots:
    void onSucceeded();
    void onFailed(const QString& reason);
    void onAborted();
    void onProgress(qint64 current, qint64 total);

   private:
    QString m_destination;
    NetJob::Ptr m_job;
};
