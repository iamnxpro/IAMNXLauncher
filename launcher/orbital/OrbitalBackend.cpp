// SPDX-License-Identifier: GPL-3.0-only
/*
 * IAMNX Launcher - QML backend for the orbital launcher.
 */

#include "OrbitalBackend.h"

#include "Application.h"
#include "BuildConfig.h"
#include "BaseInstance.h"
#include "InstanceList.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/auth/AccountList.h"
#include "minecraft/auth/MinecraftAccount.h"
#include "ui/MainWindow.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QSettings>
#include <QUrl>
#include <QVariantMap>

namespace {
constexpr int kPinIterations = 160000;
constexpr int kPinSaltBytes = 16;
constexpr int kPinLockSeconds = 20;
constexpr int kPinMaxAttempts = 5;

QString pinSettingsPath()
{
    return QDir(APPLICATION->dataRoot()).filePath(QStringLiteral("iamnx-security.ini"));
}
}  // namespace

OrbitalBackend::OrbitalBackend(MainWindow* mainWindow, QObject* parent) : QObject(parent), m_mainWindow(mainWindow)
{
    m_launchTimeout.setSingleShot(true);
    m_launchTimeout.setInterval(60000);
    connect(&m_launchTimeout, &QTimer::timeout, this, [this]() {
        if (!m_waitingForGame) {
            return;
        }
        m_waitingForGame = false;
        setBusy(false);
        setLaunchError(QStringLiteral("status.timeout"));
        emit launchFailed(QStringLiteral("status.timeout"));
    });

    if (APPLICATION->instances()) {
        connect(APPLICATION->instances(), &InstanceList::instancesChanged, this, &OrbitalBackend::refresh);
        connect(APPLICATION->instances(), &QAbstractItemModel::modelReset, this, &OrbitalBackend::refresh);
        connect(APPLICATION->instances(), &QAbstractItemModel::rowsInserted, this, &OrbitalBackend::refresh);
        connect(APPLICATION->instances(), &QAbstractItemModel::rowsRemoved, this, &OrbitalBackend::refresh);
        connect(APPLICATION->instances(), &QAbstractItemModel::dataChanged, this, &OrbitalBackend::refresh);
    }

    if (APPLICATION->accounts()) {
        connect(APPLICATION->accounts(), &AccountList::defaultAccountChanged, this, &OrbitalBackend::refreshAccount);
        connect(APPLICATION->accounts(), &AccountList::listChanged, this, &OrbitalBackend::refreshAccount);
        connect(APPLICATION->accounts(), &QAbstractItemModel::rowsInserted, this, &OrbitalBackend::refreshAccount);
        connect(APPLICATION->accounts(), &QAbstractItemModel::rowsRemoved, this, &OrbitalBackend::refreshAccount);
    }

    rebuildInstances();
    refreshAccount();
}

OrbitalBackend::~OrbitalBackend()
{
    disconnectFromInstance();
}

QString OrbitalBackend::selectedInstanceName() const
{
    return m_selected ? m_selected->name() : QString();
}

QString OrbitalBackend::selectedInstanceStatus() const
{
    if (!m_selected) {
        return QStringLiteral("none");
    }
    if (m_selected->isRunning()) {
        return QStringLiteral("running");
    }
    return m_selected->canLaunch() ? QStringLiteral("ready") : QStringLiteral("unavailable");
}

bool OrbitalBackend::canLaunchSelected() const
{
    return m_selected && m_selected->canLaunch() && !m_selected->isRunning();
}

QString OrbitalBackend::version() const
{
    return BuildConfig.printableVersionString();
}

bool OrbitalBackend::hasPin() const
{
    QSettings settings(pinSettingsPath(), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("launcherPin"));
    const bool configured = !settings.value(QStringLiteral("hash")).toString().isEmpty() &&
                            !settings.value(QStringLiteral("salt")).toString().isEmpty();
    settings.endGroup();
    return configured;
}

void OrbitalBackend::refresh()
{
    rebuildInstances();
    refreshAccount();
}

void OrbitalBackend::rebuildInstances()
{
    QVariantList next;
    if (APPLICATION->instances()) {
        const int count = APPLICATION->instances()->count();
        next.reserve(count);
        for (int i = 0; i < count; ++i) {
            auto* instance = APPLICATION->instances()->at(i);
            if (!instance) {
                continue;
            }
            QVariantMap item;
            item.insert(QStringLiteral("id"), instance->id());
            item.insert(QStringLiteral("name"), instance->name());
            item.insert(QStringLiteral("letter"), instance->name().isEmpty() ? QStringLiteral("?") : instance->name().left(1).toUpper());
            item.insert(QStringLiteral("iconKey"), instance->iconKey());
            item.insert(QStringLiteral("canLaunch"), instance->canLaunch());
            item.insert(QStringLiteral("running"), instance->isRunning());
            item.insert(QStringLiteral("status"), instance->isRunning() ? QStringLiteral("running")
                                                                        : (instance->canLaunch() ? QStringLiteral("ready") : QStringLiteral("unavailable")));
            item.insert(QStringLiteral("hue"), static_cast<int>(qHash(instance->id()) % 360));
            next.push_back(item);
        }
    }

    const bool changed = next != m_instances;
    m_instances = next;
    if (changed) {
        emit instancesChanged();
    }

    if (m_selected) {
        if (!APPLICATION->instances() || !APPLICATION->instances()->getInstanceById(m_selected->id())) {
            selectPrivate(nullptr, false);
        } else {
            emit selectionChanged();
        }
        return;
    }

    QString preferred = APPLICATION->settings()->get("SelectedInstance").toString();
    MinecraftInstance* instance = nullptr;
    if (!preferred.isEmpty() && APPLICATION->instances()) {
        instance = APPLICATION->instances()->getInstanceById(preferred);
    }
    if (!instance && APPLICATION->instances() && APPLICATION->instances()->count() > 0) {
        instance = APPLICATION->instances()->at(0);
    }
    selectPrivate(instance, false);
}

void OrbitalBackend::selectPrivate(MinecraftInstance* instance, bool persist)
{
    if (instance == m_selected) {
        if (instance && persist) {
            APPLICATION->settings()->set("SelectedInstance", instance->id());
        }
        return;
    }

    disconnectFromInstance();
    m_selected = instance;
    m_selectedId = instance ? instance->id() : QString();
    if (instance) {
        connectToInstance(instance);
        if (persist) {
            APPLICATION->settings()->set("SelectedInstance", instance->id());
        }
    }
    emit selectionChanged();
}

void OrbitalBackend::connectToInstance(MinecraftInstance* instance)
{
    if (instance) {
        m_runningConnection = connect(instance, &BaseInstance::runningStatusChanged, this, &OrbitalBackend::onInstanceRunningChanged);
    }
}

void OrbitalBackend::disconnectFromInstance()
{
    if (m_runningConnection) {
        disconnect(m_runningConnection);
        m_runningConnection = {};
    }
}

bool OrbitalBackend::selectInstance(const QString& id)
{
    if (!APPLICATION->instances()) {
        return false;
    }
    auto* instance = APPLICATION->instances()->getInstanceById(id);
    if (!instance) {
        return false;
    }
    selectPrivate(instance, true);
    return true;
}

void OrbitalBackend::onInstanceRunningChanged(bool running)
{
    if (running && m_waitingForGame) {
        m_waitingForGame = false;
        m_launchTimeout.stop();
        setBusy(false);
        setLaunchError(QString());
        emit gameStarted();
    }
    emit selectionChanged();
    rebuildInstances();
}

void OrbitalBackend::refreshAccount()
{
    QString name;
    QString type;
    if (APPLICATION->accounts()) {
        auto account = APPLICATION->accounts()->defaultAccount();
        if (account) {
            name = account->profileName().isEmpty() ? account->displayName() : account->profileName();
            type = account->accountType() == AccountType::Offline ? QStringLiteral("offline") : QStringLiteral("microsoft");
        }
    }
    if (name != m_accountName || type != m_accountType) {
        m_accountName = name;
        m_accountType = type;
        emit accountChanged();
    }
}

void OrbitalBackend::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }
    m_busy = busy;
    emit busyChanged();
}

void OrbitalBackend::setLaunchError(const QString& code)
{
    if (m_launchError == code) {
        return;
    }
    m_launchError = code;
    emit launchErrorChanged();
}

void OrbitalBackend::launchSelected()
{
    if (m_busy) {
        return;
    }
    setLaunchError(QString());
    if (!m_selected) {
        setLaunchError(QStringLiteral("error.noInstance"));
        emit launchFailed(QStringLiteral("error.noInstance"));
        return;
    }
    if (m_selected->isRunning()) {
        emit gameStarted();
        return;
    }
    if (!m_selected->canLaunch()) {
        setLaunchError(QStringLiteral("error.cannotLaunch"));
        emit launchFailed(QStringLiteral("error.cannotLaunch"));
        return;
    }
    if (!APPLICATION->accounts() || !APPLICATION->accounts()->defaultAccount()) {
        setLaunchError(QStringLiteral("error.noAccount"));
        emit launchFailed(QStringLiteral("error.noAccount"));
        return;
    }

    setBusy(true);
    m_waitingForGame = true;
    emit launchStarted();
    if (!APPLICATION->launch(m_selected)) {
        m_waitingForGame = false;
        setBusy(false);
        setLaunchError(QStringLiteral("error.launchFailed"));
        emit launchFailed(QStringLiteral("error.launchFailed"));
        return;
    }
    m_launchTimeout.start();
}

void OrbitalBackend::quitLauncher()
{
    QTimer::singleShot(120, qApp, []() { QCoreApplication::quit(); });
}

void OrbitalBackend::addInstance()
{
    if (!m_mainWindow) {
        emit toast(QStringLiteral("error.addFailed"));
        return;
    }
    m_mainWindow->requestAddInstance();
}

void OrbitalBackend::openAccounts()
{
    if (m_mainWindow) {
        m_mainWindow->requestManageAccounts();
    }
}

void OrbitalBackend::openSkins()
{
    if (m_mainWindow) {
        m_mainWindow->requestManageSkins();
    }
}

void OrbitalBackend::openSettings()
{
    if (m_mainWindow) {
        m_mainWindow->requestSettings();
    }
}

void OrbitalBackend::openHelp()
{
    if (m_mainWindow) {
        m_mainWindow->requestHelp();
    }
}

void OrbitalBackend::openInstanceSettings()
{
    if (m_selected) {
        APPLICATION->showInstanceWindow(m_selected);
    }
}

void OrbitalBackend::openInstanceFolder()
{
    if (m_selected) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_selected->gameRoot()));
    }
}

bool OrbitalBackend::validatePin(const QString& pin) const
{
    return pin.trimmed().length() >= 4 && pin.length() <= 64;
}

QByteArray OrbitalBackend::pinHash(const QString& pin, const QByteArray& salt, int iterations) const
{
    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, pin.toUtf8(), salt, iterations, 32);
}

int OrbitalBackend::pinRetrySeconds() const
{
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    return m_pinLockedUntil > now ? static_cast<int>(m_pinLockedUntil - now) : 0;
}

bool OrbitalBackend::verifyPin(const QString& pin)
{
    if (!hasPin()) {
        emit pinAccepted();
        return true;
    }
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    if (m_pinLockedUntil > now) {
        emit pinRejected(pinRetrySeconds());
        return false;
    }

    QSettings settings(pinSettingsPath(), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("launcherPin"));
    const QByteArray salt = QByteArray::fromBase64(settings.value(QStringLiteral("salt")).toString().toLatin1());
    const QByteArray expected = QByteArray::fromBase64(settings.value(QStringLiteral("hash")).toString().toLatin1());
    const int iterations = settings.value(QStringLiteral("iterations"), kPinIterations).toInt();
    settings.endGroup();

    const QByteArray actual = pinHash(pin, salt, iterations);
    quint8 diff = actual.size() == expected.size() ? 0 : 1;
    const int length = static_cast<int>(qMin(actual.size(), expected.size()));
    for (int i = 0; i < length; ++i) {
        diff |= static_cast<quint8>(actual.at(i) ^ expected.at(i));
    }

    if (diff == 0) {
        m_failedPinAttempts = 0;
        m_pinLockedUntil = 0;
        emit pinAccepted();
        return true;
    }

    ++m_failedPinAttempts;
    if (m_failedPinAttempts >= kPinMaxAttempts) {
        m_failedPinAttempts = 0;
        m_pinLockedUntil = QDateTime::currentSecsSinceEpoch() + kPinLockSeconds;
        emit pinRejected(kPinLockSeconds);
    } else {
        emit pinRejected(0);
    }
    return false;
}

bool OrbitalBackend::setPin(const QString& pin)
{
    if (!validatePin(pin)) {
        return false;
    }
    QByteArray salt(kPinSaltBytes, '\0');
    for (int i = 0; i < salt.size(); ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::system()->bounded(256));
    }
    const QByteArray hash = pinHash(pin, salt, kPinIterations);

    QSettings settings(pinSettingsPath(), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("launcherPin"));
    settings.setValue(QStringLiteral("salt"), QString::fromLatin1(salt.toBase64()));
    settings.setValue(QStringLiteral("hash"), QString::fromLatin1(hash.toBase64()));
    settings.setValue(QStringLiteral("iterations"), kPinIterations);
    settings.setValue(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    settings.endGroup();
    settings.sync();

    m_failedPinAttempts = 0;
    m_pinLockedUntil = 0;
    emit pinChanged();
    emit toast(QStringLiteral("security.saved"));
    return true;
}

void OrbitalBackend::clearPin()
{
    QSettings settings(pinSettingsPath(), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("launcherPin"));
    settings.remove(QString());
    settings.endGroup();
    settings.sync();
    m_failedPinAttempts = 0;
    m_pinLockedUntil = 0;
    emit pinChanged();
    emit toast(QStringLiteral("security.removed"));
}
