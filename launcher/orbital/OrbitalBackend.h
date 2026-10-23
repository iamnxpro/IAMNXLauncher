// SPDX-License-Identifier: GPL-3.0-only
/*
 * IAMNX Launcher - QML backend for the orbital launcher.
 *
 * The visible shell is Qt Quick 3D; the existing MainWindow stays alive as a
 * hidden host for the launcher's mature instance/account/settings dialogs.
 */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVariantList>

class MainWindow;
class MinecraftInstance;

class OrbitalBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList instances READ instances NOTIFY instancesChanged)
    Q_PROPERTY(QString selectedInstanceId READ selectedInstanceId NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedInstanceName READ selectedInstanceName NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedInstanceStatus READ selectedInstanceStatus NOTIFY selectionChanged)
    Q_PROPERTY(bool canLaunchSelected READ canLaunchSelected NOTIFY selectionChanged)
    Q_PROPERTY(QString accountName READ accountName NOTIFY accountChanged)
    Q_PROPERTY(QString accountType READ accountType NOTIFY accountChanged)
    Q_PROPERTY(bool hasPin READ hasPin NOTIFY pinChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString launchError READ launchError NOTIFY launchErrorChanged)
    Q_PROPERTY(QString version READ version CONSTANT)

   public:
    explicit OrbitalBackend(MainWindow* mainWindow, QObject* parent = nullptr);
    ~OrbitalBackend() override;

    QVariantList instances() const { return m_instances; }
    QString selectedInstanceId() const { return m_selectedId; }
    QString selectedInstanceName() const;
    QString selectedInstanceStatus() const;
    bool canLaunchSelected() const;
    QString accountName() const { return m_accountName; }
    QString accountType() const { return m_accountType; }
    bool hasPin() const;
    bool busy() const { return m_busy; }
    QString launchError() const { return m_launchError; }
    QString version() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshAccount();
    Q_INVOKABLE bool selectInstance(const QString& id);
    Q_INVOKABLE void launchSelected();
    Q_INVOKABLE void quitLauncher();

    Q_INVOKABLE void addInstance();
    Q_INVOKABLE void openAccounts();
    Q_INVOKABLE void openSkins();
    Q_INVOKABLE void openSettings();
    Q_INVOKABLE void openInstanceSettings();
    Q_INVOKABLE void openInstanceFolder();
    Q_INVOKABLE void openHelp();

    Q_INVOKABLE bool setPin(const QString& pin);
    Q_INVOKABLE void clearPin();
    Q_INVOKABLE bool verifyPin(const QString& pin);
    Q_INVOKABLE bool validatePin(const QString& pin) const;
    Q_INVOKABLE int pinRetrySeconds() const;

   signals:
    void instancesChanged();
    void selectionChanged();
    void accountChanged();
    void pinChanged();
    void busyChanged();
    void launchErrorChanged();

    void launchStarted();
    void gameStarted();
    void launchFailed(const QString& code);
    void pinRejected(int remainingSeconds);
    void pinAccepted();
    void toast(const QString& key);

   private:
    void rebuildInstances();
    void selectPrivate(MinecraftInstance* instance, bool persist);
    void setBusy(bool busy);
    void setLaunchError(const QString& code);
    void onInstanceRunningChanged(bool running);
    void connectToInstance(MinecraftInstance* instance);
    void disconnectFromInstance();

    QByteArray pinHash(const QString& pin, const QByteArray& salt, int iterations) const;

    MainWindow* m_mainWindow = nullptr;
    QVariantList m_instances;
    QString m_selectedId;
    QPointer<MinecraftInstance> m_selected;
    QMetaObject::Connection m_runningConnection;
    QString m_accountName;
    QString m_accountType;
    bool m_busy = false;
    bool m_waitingForGame = false;
    QString m_launchError;
    QTimer m_launchTimeout;

    int m_failedPinAttempts = 0;
    qint64 m_pinLockedUntil = 0;
};
