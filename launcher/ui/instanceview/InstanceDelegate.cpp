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

#include "InstanceDelegate.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QTextLayout>
#include <QTextOption>
#include <QtMath>

#include <QIcon>
#include <QTextEdit>
#include "BaseInstance.h"
#include "InstanceList.h"
#include "InstanceView.h"

// Origin: Qt
static void viewItemTextLayout(QTextLayout& textLayout, int lineWidth, qreal& height, qreal& widthUsed)
{
    height = 0;
    widthUsed = 0;
    textLayout.beginLayout();
    QString str = textLayout.text();
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        if (line.textLength() == 0)
            break;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    textLayout.endLayout();
}

ListViewDelegate::ListViewDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

namespace {
constexpr int kCardWidth = 200;
constexpr int kCardHeight = 216;
constexpr int kCardMargin = 5;
constexpr int kCardIconSize = 72;

QColor blendColor(const QColor& first, const QColor& second, qreal amount)
{
    amount = qBound(0.0, amount, 1.0);
    return QColor(qRound(first.red() * (1.0 - amount) + second.red() * amount),
                  qRound(first.green() * (1.0 - amount) + second.green() * amount),
                  qRound(first.blue() * (1.0 - amount) + second.blue() * amount),
                  qRound(first.alpha() * (1.0 - amount) + second.alpha() * amount));
}
}  // namespace

// Modern, slim progress bar at the bottom of an instance card.
void drawProgressOverlay(QPainter* painter, const QStyleOptionViewItem& option, const int value, const int maximum)
{
    if (maximum <= 0 || value <= 0 || value >= maximum) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRectF trackRect(option.rect.left() + 16, option.rect.bottom() - 15, option.rect.width() - 32, 5);
    QColor trackColor = option.palette.color(QPalette::Mid);
    trackColor.setAlphaF(0.7f);

    painter->setPen(Qt::NoPen);
    painter->setBrush(trackColor);
    painter->drawRoundedRect(trackRect, 2.5, 2.5);

    QRectF fillRect = trackRect;
    fillRect.setWidth(qMax(5.0, trackRect.width() * (qreal)value / (qreal)maximum));
    painter->setBrush(option.palette.color(QPalette::Highlight));
    painter->drawRoundedRect(fillRect, 2.5, 2.5);

    painter->restore();
}

void drawBadges(QPainter* painter, const QStyleOptionViewItem& option, BaseInstance* instance, QIcon::Mode mode, QIcon::State state)
{
    QList<QString> pixmaps;
    if (instance->isRunning()) {
        pixmaps.append("status-running");
    } else if (instance->hasCrashed() || instance->hasVersionBroken()) {
        pixmaps.append("status-bad");
    }
    if (instance->hasUpdateAvailable()) {
        pixmaps.append("checkupdate");
    }

    static const int itemSide = 24;
    static const int spacing = 1;
    const int itemsPerRow = qMax(1, qFloor(double(option.rect.width() + spacing) / double(itemSide + spacing)));
    const int rows = qCeil((double)pixmaps.size() / (double)itemsPerRow);
    QListIterator<QString> it(pixmaps);
    painter->translate(option.rect.topLeft());
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < itemsPerRow; ++x) {
            if (!it.hasNext()) {
                return;
            }
            // FIXME: inject this.
            auto icon = QIcon::fromTheme(it.next());
            // opt.icon.paint(painter, iconbox, Qt::AlignCenter, mode, state);
            const QPixmap pixmap;
            // itemSide
            QRect badgeRect(option.rect.width() - x * itemSide + qMax(x - 1, 0) * spacing - itemSide,
                            y * itemSide + qMax(y - 1, 0) * spacing, itemSide, itemSide);
            icon.paint(painter, badgeRect, Qt::AlignCenter, mode, state);
        }
    }
    painter->translate(-option.rect.topLeft());
}

void ListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.features |= QStyleOptionViewItem::WrapText;
    opt.text = index.data().toString();
    opt.textElideMode = Qt::ElideRight;
    opt.displayAlignment = Qt::AlignTop | Qt::AlignHCenter;

    painter->save();
    painter->setClipRect(opt.rect);
    painter->setRenderHint(QPainter::Antialiasing, true);

    const bool selected = opt.state & QStyle::State_Selected;
    const bool hovered = opt.state & QStyle::State_MouseOver;
    const QPalette& pal = opt.palette;

    const QColor window = pal.color(QPalette::Window);
    const QColor base = pal.color(QPalette::Base);
    const QColor accent = pal.color(QPalette::Highlight);
    const QColor text = pal.color(QPalette::Text);

    QColor cardColor = blendColor(base, pal.color(QPalette::Button), 0.55);
    if (hovered)
        cardColor = blendColor(cardColor, accent, 0.08);
    if (selected)
        cardColor = blendColor(cardColor, accent, 0.14);

    const QRectF cardRect = QRectF(opt.rect).adjusted(kCardMargin, kCardMargin, -kCardMargin, -kCardMargin);

    // Soft drop shadow.
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 70));
    painter->drawRoundedRect(cardRect.translated(0, 2), 14, 14);

    painter->setBrush(cardColor);
    QPen border(blendColor(window, text, 0.12));
    border.setWidthF(1.0);
    if (selected) {
        border = QPen(accent, 2.0);
    } else if (hovered) {
        border = QPen(blendColor(accent, window, 0.45), 1.0);
    }
    painter->setPen(border);
    painter->drawRoundedRect(cardRect.adjusted(0.5, 0.5, -0.5, -0.5), 14, 14);

    // Rounded tile behind the instance icon.
    const QRectF iconTile(cardRect.center().x() - 44, cardRect.top() + 16, 88, 88);
    painter->setBrush(blendColor(cardColor, pal.color(QPalette::Button), 0.35));
    painter->setPen(QPen(blendColor(window, text, 0.08), 1.0));
    painter->drawRoundedRect(iconTile, 18, 18);

    QIcon::Mode mode = QIcon::Normal;
    if (!(opt.state & QStyle::State_Enabled))
        mode = QIcon::Disabled;
    else if (selected)
        mode = QIcon::Selected;
    QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;

    const QRectF iconRect(iconTile.center().x() - kCardIconSize / 2.0, iconTile.center().y() - kCardIconSize / 2.0, kCardIconSize,
                          kCardIconSize);
    if (opt.icon.isNull()) {
        QFont fallbackFont = opt.font;
        fallbackFont.setBold(true);
        painter->setFont(fallbackFont);
        painter->setPen(text);
        painter->drawText(iconRect, Qt::AlignCenter, opt.text.left(1).toUpper());
    } else {
        opt.icon.paint(painter, iconRect.toRect(), Qt::AlignCenter, mode, state);
    }

    // Instance title: up to two centered lines.
    QFont titleFont = opt.font;
    titleFont.setWeight(QFont::DemiBold);
    if (titleFont.pointSizeF() > 0)
        titleFont.setPointSizeF(qMin(titleFont.pointSizeF() + 0.5, 12.0));
    painter->setFont(titleFont);

    QColor titleColor = text;
    if (!(opt.state & QStyle::State_Enabled))
        titleColor = pal.color(QPalette::Disabled, QPalette::Text);
    painter->setPen(titleColor);

    const QRectF titleRect(cardRect.left() + 14, iconTile.bottom() + 12, cardRect.width() - 28, 38);
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textOption.setTextDirection(opt.direction);
    textOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    QTextLayout textLayout(opt.text, titleFont);
    textLayout.setTextOption(textOption);

    qreal height = 0;
    qreal widthUsed = 0;
    viewItemTextLayout(textLayout, qRound(titleRect.width()), height, widthUsed);
    const QPointF textPosition(titleRect.left(), titleRect.top());
    for (int i = 0; i < textLayout.lineCount() && i < 2; ++i)
        textLayout.lineAt(i).draw(painter, textPosition);

    QStyleOptionViewItem cardOption = opt;
    cardOption.rect = cardRect.toRect();

    auto instance = (BaseInstance*)index.data(InstanceList::InstancePointerRole).value<void*>();
    if (instance)
        drawBadges(painter, cardOption, instance, mode, state);

    drawProgressOverlay(painter, cardOption, index.data(InstanceViewRoles::ProgressValueRole).toInt(),
                        index.data(InstanceViewRoles::ProgressMaximumRole).toInt());

    painter->restore();
}

QSize ListViewDelegate::sizeHint([[maybe_unused]] const QStyleOptionViewItem& option, [[maybe_unused]] const QModelIndex& index) const
{
    return QSize(kCardWidth, kCardHeight);
}

class NoReturnTextEdit : public QTextEdit {
    Q_OBJECT
   public:
    explicit NoReturnTextEdit(QWidget* parent) : QTextEdit(parent)
    {
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    }
    bool event(QEvent* event) override
    {
        auto eventType = event->type();
        if (eventType == QEvent::KeyPress || eventType == QEvent::KeyRelease) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            auto key = keyEvent->key();
            if ((key == Qt::Key_Return || key == Qt::Key_Enter) && eventType == QEvent::KeyPress) {
                emit editingDone();
                return true;
            }
            if (key == Qt::Key_Tab) {
                return true;
            }
        }
        return QTextEdit::event(event);
    }
   signals:
    void editingDone();
};

void ListViewDelegate::updateEditorGeometry(QWidget* editor,
                                            const QStyleOptionViewItem& option,
                                            [[maybe_unused]] const QModelIndex& index) const
{
    const int editorTop = kCardMargin + 16 + 88 + 12;
    QRect textRect = option.rect.adjusted(kCardMargin + 14, editorTop, -(kCardMargin + 14), -50);
    editor->setGeometry(textRect);
}

void ListViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto text = index.data(Qt::EditRole).toString();
    QTextEdit* realEditor = qobject_cast<NoReturnTextEdit*>(editor);
    realEditor->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    realEditor->append(text);
    realEditor->selectAll();
    realEditor->document()->clearUndoRedoStacks();
}

void ListViewDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QTextEdit* realEditor = qobject_cast<NoReturnTextEdit*>(editor);
    QString text = realEditor->toPlainText();
    text.replace(QChar('\n'), QChar(' '));
    text = text.trimmed();
    // Prevent instance names longer than 128 chars
    text.truncate(128);
    if (text.size() != 0) {
        const auto before = model->data(index).toString();
        model->setData(index, text);
        emit textChanged(before, text);
    }
}

QWidget* ListViewDelegate::createEditor(QWidget* parent,
                                        [[maybe_unused]] const QStyleOptionViewItem& option,
                                        [[maybe_unused]] const QModelIndex& index) const
{
    auto editor = new NoReturnTextEdit(parent);
    connect(editor, &NoReturnTextEdit::editingDone, this, &ListViewDelegate::editingDone);
    return editor;
}

void ListViewDelegate::editingDone()
{
    NoReturnTextEdit* editor = qobject_cast<NoReturnTextEdit*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

#include "InstanceDelegate.moc"
