// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "AccountListPage.h"
#include "ui/dialogs/skins/SkinManageDialog.h"
#include "ui_AccountListPage.h"

#include <QItemSelectionModel>
#include <QCryptographicHash>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QImageReader>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QTreeView>
#include <QUuid>

#include <QDebug>

#include "ui/dialogs/ChooseOfflineNameDialog.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/MSALoginDialog.h"

#include "Application.h"

namespace {
MinecraftAccountPtr selectedAccount(QTreeView* view)
{
    const auto selection = view->selectionModel()->selectedRows();
    if (selection.isEmpty()) return nullptr;
    return selection.first().data(AccountList::PointerRole).value<MinecraftAccountPtr>();
}

QString localAssetId(const QByteArray& data, const QString& kind)
{
    return QStringLiteral("local:%1:%2").arg(kind, QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex()));
}

bool readPng(const QString& path, QByteArray& bytes, QSize& size)
{
    QImageReader reader(path);
    if (!reader.canRead() || reader.format().toLower() != "png") return false;
    const QImage image = reader.read();
    if (image.isNull()) return false;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    bytes = file.readAll();
    size = image.size();
    return !bytes.isEmpty();
}
}

AccountListPage::AccountListPage(QWidget* parent) : QMainWindow(parent), ui(new Ui::AccountListPage)
{
    ui->setupUi(this);
    ui->listView->setEmptyString(
        tr("Welcome!\n"
           "If you're new here, you can select the \"Add Microsoft\" button to link your Microsoft account."));
    ui->listView->setEmptyMode(VersionListView::String);
    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_accounts = APPLICATION->accounts();

    ui->listView->setModel(m_accounts);
    ui->listView->header()->setSectionResizeMode(AccountList::VListColumns::ProfileNameColumn, QHeaderView::Stretch);
    ui->listView->header()->setSectionResizeMode(AccountList::VListColumns::TypeColumn, QHeaderView::ResizeToContents);
    ui->listView->header()->setSectionResizeMode(AccountList::VListColumns::StatusColumn, QHeaderView::ResizeToContents);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);

    // Expand the account column

    QItemSelectionModel* selectionModel = ui->listView->selectionModel();

    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
            [this]([[maybe_unused]] const QItemSelection& sel, [[maybe_unused]] const QItemSelection& dsel) { updateButtonStates(); });
    connect(ui->listView, &VersionListView::customContextMenuRequested, this, &AccountListPage::ShowContextMenu);
    connect(ui->listView, &VersionListView::activated, this,
            [this](const QModelIndex& index) { m_accounts->setDefaultAccount(m_accounts->at(index.row())); });

    connect(m_accounts, &AccountList::listChanged, this, &AccountListPage::listChanged);
    connect(m_accounts, &AccountList::listActivityChanged, this, &AccountListPage::listChanged);
    connect(m_accounts, &AccountList::defaultAccountChanged, this, &AccountListPage::listChanged);

    updateButtonStates();

    // Xbox authentication won't work without a client identifier, so disable the button if it is missing
    if (~APPLICATION->capabilities() & Application::SupportsMSA) {
        ui->actionAddMicrosoft->setVisible(false);
        ui->actionAddMicrosoft->setToolTip(tr("No Microsoft Authentication client ID was set."));
    }

    auto* importSkin = new QAction(tr("Choose local skin PNG"), this);
    auto* importCape = new QAction(tr("Choose local cape PNG"), this);
    auto* renameOffline = new QAction(tr("Rename offline account"), this);
    auto* copyOffline = new QAction(tr("Copy offline account"), this);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(importSkin);
    ui->toolBar->addAction(importCape);
    ui->toolBar->addAction(renameOffline);
    ui->toolBar->addAction(copyOffline);

    connect(importSkin, &QAction::triggered, this, [this] {
        auto account = selectedAccount(ui->listView);
        if (!account || account->accountType() != AccountType::Offline) {
            QMessageBox::information(this, tr("Offline account"), tr("Select an offline account first."));
            return;
        }
        const auto path = QFileDialog::getOpenFileName(this, tr("Select skin PNG"), {}, tr("PNG images (*.png)"));
        if (path.isEmpty()) return;
        QByteArray bytes;
        QSize size;
        if (!readPng(path, bytes, size) || !(size == QSize(64, 64) || size == QSize(64, 32))) {
            QMessageBox::warning(this, tr("Invalid skin"), tr("The skin must be a PNG with dimensions 64x64 or 64x32."));
            return;
        }
        auto* data = account->accountData();
        data->minecraftProfile.skin.id = localAssetId(bytes, QStringLiteral("skin"));
        data->minecraftProfile.skin.url.clear();
        data->minecraftProfile.skin.variant = QStringLiteral("classic");
        data->minecraftProfile.skin.data = bytes;
        data->minecraftProfile.validity = Validity::Certain;
        emit account->changed();
    });

    connect(importCape, &QAction::triggered, this, [this] {
        auto account = selectedAccount(ui->listView);
        if (!account || account->accountType() != AccountType::Offline) {
            QMessageBox::information(this, tr("Offline account"), tr("Select an offline account first."));
            return;
        }
        const auto path = QFileDialog::getOpenFileName(this, tr("Select cape PNG"), {}, tr("PNG images (*.png)"));
        if (path.isEmpty()) return;
        QByteArray bytes;
        QSize size;
        if (!readPng(path, bytes, size) || size.width() < 2 || size.height() < 2) {
            QMessageBox::warning(this, tr("Invalid cape"), tr("The cape must be a readable PNG image."));
            return;
        }
        const auto id = localAssetId(bytes, QStringLiteral("cape"));
        Cape cape;
        cape.id = id;
        cape.alias = QFileInfo(path).completeBaseName();
        cape.data = bytes;
        account->accountData()->minecraftProfile.capes.insert(id, cape);
        account->accountData()->minecraftProfile.currentCape = id;
        emit account->changed();
    });

    connect(renameOffline, &QAction::triggered, this, [this] {
        auto account = selectedAccount(ui->listView);
        if (!account || account->accountType() != AccountType::Offline) return;
        bool ok = false;
        const auto name = QInputDialog::getText(this, tr("Rename offline account"), tr("New username:"), QLineEdit::Normal,
                                                 account->profileName(), &ok).trimmed();
        if (!ok || name.isEmpty() || name == account->profileName()) return;
        if (!QRegularExpression(QStringLiteral("^[A-Za-z0-9_]{3,16}$")).match(name).hasMatch()) {
            QMessageBox::warning(this, tr("Invalid username"), tr("Use 3-16 letters, numbers, or underscores."));
            return;
        }
        auto* data = account->accountData();
        data->minecraftProfile.name = name;
        data->minecraftProfile.id = MinecraftAccount::uuidFromUsername(name).toString(QUuid::Id128);
        data->yggdrasilToken.extra[QStringLiteral("userName")] = name;
        emit account->changed();
    });

    connect(copyOffline, &QAction::triggered, this, [this] {
        auto source = selectedAccount(ui->listView);
        if (!source || source->accountType() != AccountType::Offline) return;
        bool ok = false;
        const auto name = QInputDialog::getText(this, tr("Copy offline account"), tr("Copy as:"), QLineEdit::Normal,
                                                 source->profileName() + tr(" Copy"), &ok).trimmed();
        if (!ok || name.isEmpty()) return;
        auto copy = MinecraftAccount::createOffline(name);
        *copy->accountData() = *source->accountData();
        copy->accountData()->internalId = QUuid::createUuid().toString(QUuid::Id128);
        copy->accountData()->minecraftProfile.name = name;
        copy->accountData()->minecraftProfile.id = MinecraftAccount::uuidFromUsername(name).toString(QUuid::Id128);
        copy->accountData()->yggdrasilToken.extra[QStringLiteral("userName")] = name;
        m_accounts->addAccount(copy);
    });
}

AccountListPage::~AccountListPage()
{
    delete ui;
}

void AccountListPage::retranslate()
{
    ui->retranslateUi(this);
}

void AccountListPage::ShowContextMenu(const QPoint& pos)
{
    auto menu = ui->toolBar->createContextMenu(this, tr("Context menu"));
    menu->exec(ui->listView->mapToGlobal(pos));
    delete menu;
}

void AccountListPage::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QMainWindow::changeEvent(event);
}

QMenu* AccountListPage::createPopupMenu()
{
    QMenu* filteredMenu = QMainWindow::createPopupMenu();
    filteredMenu->removeAction(ui->toolBar->toggleViewAction());
    return filteredMenu;
}

void AccountListPage::listChanged()
{
    updateButtonStates();
}

void AccountListPage::on_actionAddMicrosoft_triggered()
{
    auto account = MSALoginDialog::newAccount(this);
    if (account) {
        m_accounts->addAccount(account);
        if (m_accounts->count() == 1) {
            m_accounts->setDefaultAccount(account);
        }
    }
}

void AccountListPage::on_actionAddOffline_triggered()
{
    ChooseOfflineNameDialog dialog(tr("Please enter your desired username to add your offline account."), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (const MinecraftAccountPtr account = MinecraftAccount::createOffline(dialog.getUsername())) {
        m_accounts->addAccount(account);
        if (m_accounts->count() == 1) {
            m_accounts->setDefaultAccount(account);
        }
    }
}

void AccountListPage::on_actionRemove_triggered()
{
    auto response = CustomMessageBox::selectable(this, tr("Remove account?"), tr("Do you really want to delete this account?"),
                                                 QMessageBox::Question, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                        ->exec();
    if (response != QMessageBox::Yes) {
        return;
    }
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();
    if (selection.size() > 0) {
        QModelIndex selected = selection.first();
        m_accounts->removeAccount(selected);
    }
}

void AccountListPage::on_actionRefresh_triggered()
{
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();
    if (selection.size() > 0) {
        QModelIndex selected = selection.first();
        MinecraftAccountPtr account = selected.data(AccountList::PointerRole).value<MinecraftAccountPtr>();
        m_accounts->requestRefresh(account->internalId());
    }
}

void AccountListPage::on_actionSetDefault_triggered()
{
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();
    if (selection.size() > 0) {
        QModelIndex selected = selection.first();
        MinecraftAccountPtr account = selected.data(AccountList::PointerRole).value<MinecraftAccountPtr>();
        m_accounts->setDefaultAccount(account);
    }
}

void AccountListPage::on_actionNoDefault_triggered()
{
    m_accounts->setDefaultAccount(nullptr);
}

void AccountListPage::updateButtonStates()
{
    // If there is no selection, disable buttons that require something selected.
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();
    bool hasSelection = !selection.empty();
    bool accountIsReady = false;
    bool accountIsOnline = false;
    bool accountCanMoveUp = false;
    bool accountCanMoveDown = false;
    if (hasSelection) {
        QModelIndex selected = selection.first();
        MinecraftAccountPtr account = selected.data(AccountList::PointerRole).value<MinecraftAccountPtr>();
        accountIsReady = !account->isActive();
        accountIsOnline = account->accountType() != AccountType::Offline;

        accountCanMoveUp = selected.row() > 0;
        int indexOfLast = m_accounts->count() - 1;
        accountCanMoveDown = selected.row() < indexOfLast;
    }
    ui->actionRemove->setEnabled(accountIsReady);
    ui->actionSetDefault->setEnabled(accountIsReady);
    ui->actionManageSkins->setEnabled(accountIsReady && accountIsOnline);
    ui->actionRefresh->setEnabled(accountIsReady && accountIsOnline);

    if (m_accounts->defaultAccount().get() == nullptr) {
        ui->actionNoDefault->setEnabled(false);
        ui->actionNoDefault->setChecked(true);
    } else {
        ui->actionNoDefault->setEnabled(true);
        ui->actionNoDefault->setChecked(false);
    }
    ui->actionMoveUp->setEnabled(accountCanMoveUp);
    ui->actionMoveDown->setEnabled(accountCanMoveDown);
    ui->listView->resizeColumnToContents(3);
}

void AccountListPage::on_actionManageSkins_triggered()
{
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();
    if (selection.size() > 0) {
        QModelIndex selected = selection.first();
        MinecraftAccountPtr account = selected.data(AccountList::PointerRole).value<MinecraftAccountPtr>();
        SkinManageDialog dialog(this, account);
        dialog.exec();
    }
}

void AccountListPage::on_actionMoveUp_triggered()
{
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();
    if (selection.size() > 0) {
        QModelIndex selected = selection.first();
        m_accounts->moveAccount(selected, -1);
    }
}

void AccountListPage::on_actionMoveDown_triggered()
{
    QModelIndexList selection = ui->listView->selectionModel()->selectedIndexes();
    if (selection.size() > 0) {
        QModelIndex selected = selection.first();
        m_accounts->moveAccount(selected, 1);
    }
}
