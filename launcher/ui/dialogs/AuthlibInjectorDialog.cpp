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

#include "AuthlibInjectorDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "BuildConfig.h"
#include "minecraft/auth/AuthlibInjectorDownload.h"
#include "ui/dialogs/ProgressDialog.h"

AuthlibInjectorDialog::AuthlibInjectorDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("authlib-injector"));
    setMinimumWidth(560);

    auto* layout = new QVBoxLayout(this);

    auto* explanation = new QLabel(this);
    explanation->setTextFormat(Qt::RichText);
    explanation->setWordWrap(true);
    explanation->setOpenExternalLinks(true);
    explanation->setTextInteractionFlags(Qt::TextBrowserInteraction);
    explanation->setText(
        tr("<h3>authlib-injector</h3>"
           "<p>Offline accounts on these Minecraft versions need the "
           "<a href=\"https://github.com/yushijinhun/authlib-injector\">authlib-injector</a> "
           "java agent to show a custom skin or cape in game:</p>"
           "<ul>"
           "<li>Minecraft 1.7.10 up to 1.8.0</li>"
           "<li>Minecraft 1.21.9 (25w32a) and newer</li>"
           "</ul>"
           "<p>Versions between those releases use the profile properties of the launcher and do not "
           "need the jar.</p>"
           "<p>The launcher only downloads the pinned release below, from the official GitHub release "
           "page, when you press the button, and verifies its SHA-256 checksum before using it.</p>"
           "<p>authlib-injector is a third party project licensed under the AGPL-3.0 with the "
           "\"Authlib-Injector\" exception. It is not part of the launcher.</p>"));
    layout->addWidget(explanation);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    layout->addWidget(m_statusLabel);

    auto* checksumLabel = new QLabel(this);
    checksumLabel->setTextFormat(Qt::RichText);
    checksumLabel->setWordWrap(true);
    checksumLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    checksumLabel->setText(tr("Pinned version: <b>%1</b><br>SHA-256: <code>%2</code>")
                               .arg(BuildConfig.AUTHLIB_INJECTOR_VERSION.toHtmlEscaped(),
                                    BuildConfig.AUTHLIB_INJECTOR_SHA256.toHtmlEscaped()));
    checksumLabel->setStyleSheet(QStringLiteral("color: palette(mid);"));
    layout->addWidget(checksumLabel);

    layout->addStretch();

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_downloadButton = buttons->addButton(tr("Download"), QDialogButtonBox::ActionRole);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_downloadButton, &QPushButton::clicked, this, &AuthlibInjectorDialog::download);

    refreshStatus();
}

void AuthlibInjectorDialog::refreshStatus()
{
    const QString installed = AuthlibInjectorDownload::installedPath();
    if (installed.isEmpty()) {
        m_statusLabel->setText(tr("<p><b>Status:</b> not installed yet.</p>"));
        m_downloadButton->setText(tr("Download"));
        return;
    }

    m_statusLabel->setText(tr("<p><b>Status:</b> installed at<br><code>%1</code></p>").arg(installed.toHtmlEscaped()));
    m_downloadButton->setText(tr("Re-download"));
}

void AuthlibInjectorDialog::download()
{
    const QString destination = AuthlibInjectorDownload::targetPath();
    if (destination.isEmpty()) {
        QMessageBox::critical(this, tr("Cannot download authlib-injector"),
                              tr("No writable folder for the jar was found. Please check the permissions of the launcher "
                                 "folder and try again."));
        return;
    }

    auto* task = new AuthlibInjectorDownload(destination, this);

    ProgressDialog progress(this);
    progress.setSkipButton(true, tr("Cancel"));
    progress.execWithTask(task);

    switch (task->getState()) {
        case Task::State::Succeeded:
            refreshStatus();
            QMessageBox::information(this, tr("authlib-injector installed"),
                                     tr("The jar was downloaded and saved to:\n%1\n\nThe launcher will use it the next time "
                                        "it starts Minecraft.")
                                         .arg(destination));
            break;
        case Task::State::Failed:
            QMessageBox::critical(this, tr("Download failed"), task->failReason());
            break;
        default:
            // Aborted by the user, or the dialog was closed before it finished.
            break;
    }
}
