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

#include "AuthlibInjectorDownload.h"

#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>

#include <utility>

#include "Application.h"
#include "BuildConfig.h"
#include "FileSystem.h"
#include "net/ChecksumValidator.h"
#include "net/Request.h"

namespace {
/// Creates @a dir if needed and checks that we can actually write into it.
bool isWritableDirectory(const QString& dir)
{
    if (dir.isEmpty() || !QDir().mkpath(dir))
        return false;

    QTemporaryFile probe(FS::PathCombine(dir, QStringLiteral(".write-probe-XXXXXX")));
    return probe.open();
}
}  // namespace

AuthlibInjectorDownload::AuthlibInjectorDownload(QString destination, QObject* parent)
    : Task(), m_destination(std::move(destination))
{
    // Task does not take a QObject parent in its constructor.
    if (parent)
        setParent(parent);

    setAbortable(true);
}

QString AuthlibInjectorDownload::fileName()
{
    return QStringLiteral("authlib-injector.jar");
}

QString AuthlibInjectorDownload::installedPath()
{
    // The launcher's own jar folders first, so that whatever is found here is
    // exactly what the launch step would attach to the game.
    const QString found = APPLICATION->getJarPath(fileName());
    if (!found.isEmpty())
        return found;

    // A copy dropped next to the executable also works.
    const QString besideExe = FS::PathCombine(QCoreApplication::applicationDirPath(), fileName());
    if (QFileInfo(besideExe).isFile())
        return besideExe;

    return {};
}

QString AuthlibInjectorDownload::targetPath()
{
    // Replace the copy that is actually in use, if there is one.
    const QString existing = installedPath();
    if (!existing.isEmpty())
        return existing;

    // Prefer the portable "jars" folder next to the executable, which is where
    // a portable installation keeps everything.
    const QString portableJars = FS::PathCombine(QCoreApplication::applicationDirPath(), QStringLiteral("jars"));
    if (isWritableDirectory(portableJars))
        return FS::PathCombine(portableJars, fileName());

    // Fall back to the launcher data folder, for read-only install locations
    // such as "C:\Program Files".
    const QString dataJars = FS::PathCombine(APPLICATION->root(), QStringLiteral("jars"));
    if (isWritableDirectory(dataJars))
        return FS::PathCombine(dataJars, fileName());

    return {};
}

bool AuthlibInjectorDownload::abort()
{
    if (!m_job)
        return false;

    return m_job->abort();
}

void AuthlibInjectorDownload::executeTask()
{
    if (m_destination.isEmpty()) {
        emitFailed(tr("No writable location for the authlib-injector jar was found."));
        return;
    }

    if (BuildConfig.AUTHLIB_INJECTOR_URL.isEmpty() || BuildConfig.AUTHLIB_INJECTOR_SHA256.isEmpty()) {
        emitFailed(tr("This build does not know where to download authlib-injector from."));
        return;
    }

    setStatus(tr("Downloading authlib-injector %1...").arg(BuildConfig.AUTHLIB_INJECTOR_VERSION));

    m_job.reset(new NetJob(QStringLiteral("Download authlib-injector"), APPLICATION->network()));
    m_job->setAskRetry(false);

    auto action = Net::Request::makeFile(QUrl(BuildConfig.AUTHLIB_INJECTOR_URL), m_destination);
    action->addValidator(new Net::ChecksumValidator(QCryptographicHash::Sha256, BuildConfig.AUTHLIB_INJECTOR_SHA256));
    m_job->addNetAction(action);

    connect(m_job.get(), &NetJob::succeeded, this, &AuthlibInjectorDownload::onSucceeded);
    connect(m_job.get(), &NetJob::failed, this, &AuthlibInjectorDownload::onFailed);
    connect(m_job.get(), &NetJob::aborted, this, &AuthlibInjectorDownload::onAborted);
    connect(m_job.get(), &NetJob::progress, this, &AuthlibInjectorDownload::onProgress);
    connect(m_job.get(), &NetJob::stepProgress, this, &AuthlibInjectorDownload::propagateStepProgress);

    m_job->start();
}

void AuthlibInjectorDownload::onSucceeded()
{
    if (!QFileInfo(m_destination).isFile()) {
        emitFailed(tr("The download finished but the file could not be found."));
        return;
    }

    emitSucceeded();
}

void AuthlibInjectorDownload::onFailed(const QString& reason)
{
    emitFailed(reason);
}

void AuthlibInjectorDownload::onAborted()
{
    emitAborted();
}

void AuthlibInjectorDownload::onProgress(qint64 current, qint64 total)
{
    setProgress(current, total);
}
