// SPDX-License-Identifier: GPL-3.0-only
/*
 * IAMNX Launcher - the visible Qt Quick / Quick 3D orbital shell.
 */

#include "OrbitalWindow.h"

#include "Application.h"
#include "LanguageController.h"
#include "OrbitalBackend.h"
#include "ui/MainWindow.h"

#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QMessageBox>
#include <QQmlError>
#include <QQuickView>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <QUrl>

OrbitalWindow::OrbitalWindow(MainWindow* mainWindow, QObject* parent) : QObject(parent), m_mainWindow(mainWindow)
{
    m_backend = new OrbitalBackend(mainWindow, this);
    m_language = new LanguageController(this);

    m_view = new QQuickView();
    m_view->setTitle(QStringLiteral("IAMNX Launcher"));
    m_view->setIcon(APPLICATION->logo());
    m_view->setColor(QColor(QStringLiteral("#0B0806")));
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->setMinimumSize(QSize(980, 640));
    m_view->resize(1360, 860);

    if (auto* screen = QGuiApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        m_view->setPosition(available.center() - QPoint(m_view->width() / 2, m_view->height() / 2));
    }

    m_view->rootContext()->setContextProperty(QStringLiteral("Backend"), m_backend);
    m_view->rootContext()->setContextProperty(QStringLiteral("Lang"), m_language);
    m_view->setSource(QUrl(QStringLiteral("qrc:/orbital/qml/main.qml")));

    if (m_view->status() == QQuickView::Error) {
        QStringList reasons;
        for (const auto& error : m_view->errors()) {
            reasons << error.toString();
            qWarning() << "Orbital QML error:" << error;
        }
        failToLegacyUi(reasons.join(QLatin1Char('\n')));
        return;
    }

    connect(m_view, &QQuickWindow::sceneGraphError, this, [this](QQuickWindow::SceneGraphError, const QString& message) {
        qCritical() << "Orbital scene graph error:" << message;
        QTimer::singleShot(0, this, [this, message]() { failToLegacyUi(message); });
    });
    connect(m_view, &QQuickWindow::closing, this, [this](QQuickCloseEvent*) {
        emit closed();
        QCoreApplication::quit();
    });
}

OrbitalWindow::~OrbitalWindow()
{
    if (m_view) {
        delete m_view;
        m_view = nullptr;
    }
}

void OrbitalWindow::show(bool minimized)
{
    if (m_failed || !m_view || m_view->status() == QQuickView::Error) {
        return;
    }
    if (minimized) {
        m_view->showMinimized();
    } else {
        m_view->show();
        m_view->requestActivate();
    }
}

void OrbitalWindow::raise()
{
    if (m_failed || !m_view || m_view->status() == QQuickView::Error) {
        if (m_mainWindow) {
            m_mainWindow->showNormal();
            m_mainWindow->raise();
            m_mainWindow->activateWindow();
        }
        return;
    }
    m_view->showNormal();
    m_view->raise();
    m_view->requestActivate();
}

void OrbitalWindow::close()
{
    if (m_view) {
        m_view->close();
    }
}

void OrbitalWindow::failToLegacyUi(const QString& reason)
{
    if (m_failed) {
        return;
    }
    m_failed = true;
    qCritical() << "Orbital UI could not be created. Falling back to the classic launcher window.";
    if (m_view) {
        m_view->hide();
    }
    if (m_mainWindow) {
        m_mainWindow->show();
    }
    QMessageBox::critical(nullptr, QStringLiteral("IAMNX Launcher"),
                          QStringLiteral("The orbital Qt Quick 3D interface could not start, so the classic interface was opened instead.\n\n%1")
                              .arg(reason));
}
