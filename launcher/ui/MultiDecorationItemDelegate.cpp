// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2025 TheKodeToad <TheKodeToad@proton.me>
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
 *  This file incorporates work covered by the following copyright and
 *  permission notice:
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

#include <QApplication>
#include <QIcon>
#include <QModelIndex>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <Qt>

#include "MultiDecorationItemDelegate.h"

namespace {
auto iconsAt(const QModelIndex& index)
{
    return index.data(Qt::DecorationRole).value<QList<MultiDecorationItemDelegate::Icon>>();
}

int iconSpacing(const QStyleOptionViewItem& opt)
{
    QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    return (style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, opt.widget) + 1) * 2;
}
}  // namespace

void MultiDecorationItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    const int spacing = iconSpacing(*option);

    QList<Icon> icons = iconsAt(index);
    if (icons.isEmpty()) {
        option->features &= ~QStyleOptionViewItem::HasDecoration;
        return;
    }

    int width = 0;
    for (const auto& [icon, size] : icons) {
        if (width != 0) {
            width += spacing;
        }
        width += size.width();
    }

    // HACK: offset the drawing of the text so we can do our own custom drawing
    option->decorationSize = { width, 1 };
}

void MultiDecorationItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const int spacing = iconSpacing(option);

    int x = 0;
    for (const auto& [icon, size] : iconsAt(index)) {
        if (x != 0) {
            x += spacing;
        }

        QSize actualSize = icon.actualSize(size);

        int y = (option.rect.height() - actualSize.height()) / 2;

        const QIcon::Mode mode = (option.state & QStyle::State_Selected) != 0 ? QIcon::Selected : QIcon::Normal;
        const QIcon::State state = (option.state & QStyle::State_Open) != 0 ? QIcon::On : QIcon::Off;

        icon.paint(painter, option.rect.x() + x, option.rect.y() + y, actualSize.width(), actualSize.height(), Qt::AlignLeft | Qt::AlignTop,
                   mode, state);
        x += size.width();
    }
}
