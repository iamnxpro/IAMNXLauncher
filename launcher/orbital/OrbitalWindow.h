// SPDX-License-Identifier: GPL-3.0-only
/*
 * IAMNX Launcher - the visible Qt Quick / Quick 3D orbital shell.
 */

#pragma once

#include <QObject>
#include <QPointer>

class MainWindow;
class OrbitalBackend;
class LanguageController;
class QQuickView;

class OrbitalWindow : public QObject {
    Q_OBJECT

   public:
    explicit OrbitalWindow(MainWindow* mainWindow, QObject* parent = nullptr);
    ~OrbitalWindow() override;

    void show(bool minimized = false);
    void raise();
    void close();

   signals:
    void closed();

   private:
    void failToLegacyUi(const QString& reason);

    MainWindow* m_mainWindow = nullptr;
    QQuickView* m_view = nullptr;
    OrbitalBackend* m_backend = nullptr;
    LanguageController* m_language = nullptr;
    bool m_failed = false;
};
