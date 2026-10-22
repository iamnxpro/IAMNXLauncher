// SPDX-License-Identifier: GPL-3.0-only
/*
 *  IAMNXLauncher - Minecraft Launcher
 *  Copyright (C) 2026 IAMNXLAUNCHER Contributors
 *
 *  Based on Prism Launcher themes:
 *  Copyright (C) 2024 Tayou <git@tayou.org>
 *  Copyright (C) 2024 TheKodeToad <TheKodeToad@proton.me>
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
#include "IAMNXTheme.h"

#include <QObject>

QString IAMNXTheme::id()
{
    return "iamnx";
}

QString IAMNXTheme::name()
{
    return QObject::tr("IAMNX Ember");
}

QPalette IAMNXTheme::colorScheme()
{
    QPalette palette;

    const QColor window(20, 16, 14);
    const QColor base(14, 11, 10);
    const QColor surface(27, 21, 18);
    const QColor button(36, 28, 24);
    const QColor text(246, 239, 233);

    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, QColor(22, 17, 15));
    palette.setColor(QPalette::ToolTipBase, surface);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, button);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, QColor(255, 178, 122));
    palette.setColor(QPalette::Link, QColor(255, 138, 61));
    palette.setColor(QPalette::LinkVisited, QColor(255, 94, 26));
    palette.setColor(QPalette::Highlight, QColor(255, 94, 26));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor(154, 139, 130));

    palette.setColor(QPalette::Light, QColor(58, 46, 39));
    palette.setColor(QPalette::Midlight, QColor(44, 35, 32));
    palette.setColor(QPalette::Mid, QColor(31, 25, 21));
    palette.setColor(QPalette::Dark, QColor(11, 9, 8));
    palette.setColor(QPalette::Shadow, Qt::black);

    return fadeInactive(palette, fadeAmount(), fadeColor());
}

double IAMNXTheme::fadeAmount()
{
    return 0.55;
}

QColor IAMNXTheme::fadeColor()
{
    return QColor(20, 16, 14);
}

bool IAMNXTheme::hasStyleSheet()
{
    return true;
}

QString IAMNXTheme::appStyleSheet()
{
    return QStringLiteral(R"(
QWidget {
    selection-background-color: #ff5e1a;
    selection-color: #ffffff;
}

QMainWindow, QDialog {
    background-color: #14100e;
}

QMenuBar {
    background-color: #17120f;
    color: #f6efe9;
    border-bottom: 1px solid #2e241f;
}

QMenuBar::item {
    background: transparent;
    padding: 4px 10px;
}

QMenuBar::item:selected {
    background-color: #33241b;
    border-radius: 3px;
}

QMenu {
    background-color: #1b1512;
    color: #f6efe9;
    border: 1px solid #3a2c24;
    padding: 4px;
}

QMenu::item {
    padding: 6px 24px 6px 20px;
    border-radius: 5px;
}

QMenu::item:selected {
    background-color: #ff5e1a;
    color: #ffffff;
}

QMenu::item:disabled {
    color: #7a6c63;
}

QMenu::separator {
    height: 1px;
    background: #2e241f;
    margin: 4px 8px;
}

QToolBar {
    background-color: #17120f;
    border: none;
    border-bottom: 1px solid #2e241f;
    spacing: 3px;
    padding: 3px 6px;
}

QToolBar#mainToolBar {
    border-bottom: 2px solid #ff5e1a;
}

QToolBar#newsToolBar {
    background-color: #17120f;
    border: none;
    border-top: 1px solid #2e241f;
    padding: 3px 10px;
}

QToolBar::separator {
    background: #3a2c24;
    width: 1px;
    margin: 4px 6px;
}

QToolButton {
    background: transparent;
    color: #f6efe9;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 3px 6px;
}

QToolButton:hover {
    background-color: #2a1f19;
    border-color: #4a362b;
}

QToolButton:pressed, QToolButton:checked {
    background-color: #ff5e1a;
    color: #ffffff;
}

QToolButton:disabled {
    color: #7a6c63;
}

QPushButton {
    background-color: #241c18;
    color: #f6efe9;
    border: 1px solid #4a362b;
    border-radius: 7px;
    padding: 7px 16px;
    min-height: 18px;
}

QPushButton:hover {
    background-color: #31241d;
    border-color: #ff8a3d;
}

QPushButton:pressed {
    background-color: #ff5e1a;
    color: #ffffff;
}

QPushButton:default {
    border: 1px solid #ff5e1a;
}

QPushButton:disabled {
    background-color: #191310;
    border-color: #2c2320;
    color: #7a6c63;
}

QLineEdit, QPlainTextEdit, QTextEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QDateTimeEdit {
    background-color: #0e0b0a;
    color: #f6efe9;
    border: 1px solid #3a2c24;
    border-radius: 7px;
    padding: 6px 10px;
}

QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus,
QDateEdit:focus, QDateTimeEdit:focus {
    border-color: #ff5e1a;
}

QLineEdit:disabled, QPlainTextEdit:disabled, QTextEdit:disabled, QComboBox:disabled, QSpinBox:disabled,
QDoubleSpinBox:disabled, QDateEdit:disabled, QDateTimeEdit:disabled {
    color: #7a6c63;
    border-color: #2c2320;
}

QComboBox::drop-down {
    border: none;
    width: 20px;
}

QComboBox QAbstractItemView {
    background-color: #1b1512;
    color: #f6efe9;
    border: 1px solid #3a2c24;
    selection-background-color: #ff5e1a;
    selection-color: #ffffff;
}

QGroupBox {
    border: 1px solid #2e241f;
    border-radius: 6px;
    margin-top: 10px;
    padding-top: 8px;
    background-color: #17120f;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 4px;
    color: #ff8a3d;
    font-weight: 600;
}

QTabWidget::pane {
    border: 1px solid #2e241f;
    border-radius: 10px;
    background-color: #14100e;
    top: -1px;
}

QTabBar::tab {
    background-color: #1b1512;
    color: #b7a79c;
    border: 1px solid #2e241f;
    border-radius: 8px;
    padding: 7px 16px;
    margin-right: 4px;
}

QTabBar::tab:hover {
    color: #f6efe9;
    background-color: #241c18;
}

QTabBar::tab:selected {
    background-color: #241c18;
    color: #ff8a3d;
    border-bottom: 2px solid #ff5e1a;
}

QTreeView, QTableView, QListView, QColumnView, QTreeWidget, QTableWidget, QListWidget {
    background-color: #0e0b0a;
    alternate-background-color: #16110f;
    color: #f6efe9;
    border: 1px solid #2e241f;
    border-radius: 4px;
}

QTreeView::item:hover, QTableView::item:hover, QListView::item:hover {
    background-color: #241c18;
}

QTreeView::item:selected, QTableView::item:selected, QListView::item:selected {
    background-color: #ff5e1a;
    color: #ffffff;
}

QHeaderView::section {
    background-color: #1b1512;
    color: #d9ccc3;
    padding: 5px 8px;
    border: none;
    border-right: 1px solid #2e241f;
    border-bottom: 1px solid #2e241f;
}

QHeaderView::section:hover {
    background-color: #241c18;
}

QScrollBar:vertical {
    background: #0e0b0a;
    width: 12px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background: #4a362b;
    min-height: 24px;
    border-radius: 6px;
}

QScrollBar::handle:vertical:hover {
    background: #ff5e1a;
}

QScrollBar:horizontal {
    background: #0e0b0a;
    height: 12px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background: #4a362b;
    min-width: 24px;
    border-radius: 6px;
}

QScrollBar::handle:horizontal:hover {
    background: #ff5e1a;
}

QScrollBar::add-line, QScrollBar::sub-line {
    width: 0;
    height: 0;
}

QScrollBar::add-page, QScrollBar::sub-page {
    background: transparent;
}

QStatusBar {
    background-color: #17120f;
    color: #b7a79c;
    border-top: 1px solid #2e241f;
}

QStatusBar::item {
    border: none;
}

QToolTip {
    background-color: #241c18;
    color: #f6efe9;
    border: 1px solid #ff5e1a;
    padding: 4px;
}

QProgressBar {
    background-color: #0e0b0a;
    border: 1px solid #2e241f;
    border-radius: 5px;
    min-height: 8px;
    max-height: 12px;
    text-align: center;
    color: #f6efe9;
}

QProgressBar::chunk {
    background-color: #ff5e1a;
    border-radius: 4px;
}

QSplitter::handle {
    background-color: #2e241f;
}

QSplitter::handle:hover {
    background-color: #ff5e1a;
}

QLabel#iamnxBrandName {
    color: #ff7a33;
    font-weight: 700;
    font-size: 14px;
}

QLabel#iamnxBrandTag {
    color: #b7a79c;
    font-size: 10px;
}

/* ------------------------------------------------------------------ dashboard shell */
#iamnxShell {
    background-color: #14100e;
}

#iamnxSidebar {
    background-color: #0f0c0a;
    border-right: 1px solid #271d18;
}

#iamnxContent {
    background-color: #14100e;
}

QLabel#iamnxSidebarName {
    color: #fff3ea;
    font-size: 17px;
    font-weight: 800;
}

QLabel#iamnxSidebarTag {
    color: #ff8a3d;
    font-size: 9px;
    font-weight: 700;
}

QLabel#iamnxNavSection {
    color: #7d6c61;
    font-size: 9px;
    font-weight: 700;
    padding: 8px 10px 3px 10px;
}

QLabel#iamnxVersionLabel {
    color: #6f625a;
    font-size: 9px;
    padding-top: 6px;
}

QPushButton#iamnxNavButton {
    background: transparent;
    border: none;
    border-radius: 9px;
    color: #c9bab0;
    text-align: left;
    padding: 8px 12px;
    font-size: 12px;
    font-weight: 500;
}

QPushButton#iamnxNavButton:hover {
    background-color: #1e1714;
    color: #fff3ea;
}

QPushButton#iamnxNavButton:pressed {
    background-color: #2a1f19;
}

QPushButton#iamnxNavButton:disabled {
    color: #6f625a;
}

QPushButton#iamnxNavButtonActive {
    background-color: #2a1a11;
    border: 1px solid #4a2a18;
    border-radius: 9px;
    color: #ff8a3d;
    text-align: left;
    padding: 8px 12px;
    font-size: 12px;
    font-weight: 600;
}

QPushButton#iamnxNavButtonActive:hover {
    background-color: #342014;
    color: #ffa366;
}

QLabel#iamnxPageTitle {
    color: #fff3ea;
    font-size: 26px;
    font-weight: 700;
}

QLabel#iamnxPageSubtitle {
    color: #a8978c;
    font-size: 12px;
}

QLabel#iamnxSectionTitle {
    color: #f0e5dc;
    font-size: 15px;
    font-weight: 600;
}

QToolButton#iamnxPrimaryButton,
QPushButton#iamnxPrimaryButton {
    background-color: #ff5e1a;
    border: none;
    border-radius: 10px;
    color: #ffffff;
    padding: 8px 16px;
    font-weight: 600;
}

QToolButton#iamnxPrimaryButton:hover,
QPushButton#iamnxPrimaryButton:hover {
    background-color: #ff7a33;
}

QToolButton#iamnxPrimaryButton:pressed,
QPushButton#iamnxPrimaryButton:pressed {
    background-color: #e85012;
}

QToolButton#iamnxPrimaryButton:disabled,
QPushButton#iamnxPrimaryButton:disabled {
    background-color: #3a2418;
    color: #8a766a;
}

#iamnxHero {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3b1e0d, stop:0.55 #251610, stop:1 #16110f);
    border: 1px solid #4a2a18;
    border-radius: 18px;
}

QLabel#iamnxHeroTitle {
    color: #fff3ea;
    font-size: 22px;
    font-weight: 700;
}

QLabel#iamnxHeroSubtitle {
    color: #d8c6ba;
    font-size: 12px;
}

QToolButton#iamnxHeroPrimaryButton {
    background-color: #ff5e1a;
    border: none;
    border-radius: 11px;
    color: #ffffff;
    padding: 10px 20px;
    font-size: 13px;
    font-weight: 700;
}

QToolButton#iamnxHeroPrimaryButton:hover {
    background-color: #ff7a33;
}

QToolButton#iamnxHeroPrimaryButton:pressed {
    background-color: #e85012;
}

QToolButton#iamnxHeroPrimaryButton:disabled {
    background-color: #3a2418;
    color: #8a766a;
}

QToolButton#iamnxHeroSecondaryButton {
    background-color: #201813;
    border: 1px solid #5a4030;
    border-radius: 11px;
    color: #ffe9da;
    padding: 10px 18px;
    font-size: 12px;
}

QToolButton#iamnxHeroSecondaryButton:hover {
    background-color: #2a1f19;
    border-color: #ff8a3d;
}

QToolButton#iamnxPillButton {
    background-color: #1a1411;
    border: 1px solid #2f241e;
    border-radius: 8px;
    color: #cdbfb5;
    padding: 6px 12px;
    font-size: 11px;
    font-weight: 500;
}

QToolButton#iamnxPillButton:hover {
    background-color: #241b16;
    border-color: #ff8a3d;
    color: #fff3ea;
}

QToolButton#iamnxPillButton:disabled {
    background-color: #15110f;
    border-color: #251d19;
    color: #6f625a;
}

#iamnxInstanceView {
    background: transparent;
    border: none;
}

#iamnxInstanceView QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 0;
}

#iamnxInstanceView QScrollBar::handle:vertical {
    background: #3a2c24;
    border-radius: 5px;
    min-height: 28px;
}

#iamnxInstanceView QScrollBar::handle:vertical:hover {
    background: #ff5e1a;
}
)");
}

QString IAMNXTheme::tooltip()
{
    return QObject::tr("Warm dark theme with IAMNX orange accents.");
}
