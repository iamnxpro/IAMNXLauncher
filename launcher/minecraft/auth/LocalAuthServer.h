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

#pragma once

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>

/**
 * A tiny HTTP server bound to the loopback interface that acts as a
 * Yggdrasil (authlib) compatible server for a single offline profile.
 *
 * It serves the skin and cape images of that profile and exposes the small
 * subset of the Yggdrasil API that the vanilla client talks to when the
 * authlib-injector java agent is attached to the game.
 *
 * The implementation follows the "Yggdrasil server technical specification"
 * of the authlib-injector project:
 * https://yushijinhun.github.io/authlib-injector/en/yggdrasil-server-technical-specification.html
 */
class LocalAuthServer : public QObject {
    Q_OBJECT
   public:
    explicit LocalAuthServer(QObject* parent = nullptr);
    ~LocalAuthServer() override;

    /// The host the server is reachable at. This is a constant on purpose so
    /// that the texture URLs stay predictable.
    static QString host();

    /// Start listening on a free port of the loopback interface.
    /// Returns false and fills in @a error when the server could not start.
    bool start(QString* error = nullptr);
    void stop();
    bool isRunning() const;

    /// The port the server is listening on.
    quint16 port() const;
    /// Base URL of the server, for example "http://127.0.0.1:53421".
    QString baseUrl() const;

    /// Set the profile this server acts as. The UUID is expected to be
    /// unhyphenated, as required by the Yggdrasil specification.
    void setProfile(const QString& uuid, const QString& name);
    /// Set the access token that is handed out by the authenticate endpoint.
    void setAccessToken(const QString& accessToken);
    /// Set the skin image. @a model is either "classic" or "slim".
    void setSkin(const QByteArray& png, const QString& model);
    /// Set the cape image.
    void setCape(const QByteArray& png);

    bool hasSkin() const { return !m_skin.isEmpty(); }
    bool hasCape() const { return !m_cape.isEmpty(); }

    /// The base64 encoded value of the "textures" profile property.
    QString texturesProperty() const;
    /// The base64 encoded API metadata, used for
    /// "-Dauthlibinjector.yggdrasil.prefetched".
    QString prefetchedMetadata() const;
    /// The value of the "--profileProperties" argument, for example
    /// {"textures":"<base64>"}.
    QString profilePropertiesArgument() const;

   private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

   private:
    void handleRequest(QTcpSocket* socket, const QString& method, const QString& path, const QByteArray& body);
    void respond(QTcpSocket* socket, int status, const QByteArray& contentType, const QByteArray& body);
    void respondEmpty(QTcpSocket* socket, int status);

    QByteArray metadataJson() const;
    QByteArray authenticateJson() const;
    QByteArray profileJson() const;
    QByteArray texturesPayloadJson() const;
    QByteArray profilePropertiesJson() const;

    QString skinUrl() const;
    QString capeUrl() const;

    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;

    QString m_uuid;
    QString m_name;
    QString m_accessToken;
    QByteArray m_skin;
    QString m_skinModel = QStringLiteral("classic");
    QString m_skinHash;
    QByteArray m_cape;
    QString m_capeHash;
};
