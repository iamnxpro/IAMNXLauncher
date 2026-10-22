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

#include "LocalAuthServer.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr auto JSON_CONTENT_TYPE = "application/json; charset=utf-8";
constexpr auto PNG_CONTENT_TYPE = "image/png";

QByteArray statusText(int status)
{
    switch (status) {
        case 200:
            return "OK";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        default:
            return "Internal Server Error";
    }
}

/// Strips the query string from a request target.
QString requestPath(const QString& target)
{
    const auto query = target.indexOf('?');
    return query < 0 ? target : target.left(query);
}

/// The hash of a texture, as used by Minecraft to identify and cache it.
QString textureHash(const QByteArray& png)
{
    return QString::fromLatin1(QCryptographicHash::hash(png, QCryptographicHash::Sha256).toHex());
}
}  // namespace

LocalAuthServer::LocalAuthServer(QObject* parent) : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &LocalAuthServer::onNewConnection);
}

LocalAuthServer::~LocalAuthServer()
{
    stop();
}

QString LocalAuthServer::host()
{
    // This is a constant on purpose: the texture URLs have to be stable for
    // the lifetime of the game process and the host ends up in the whitelist
    // of the authlib-injector.
    return QStringLiteral("127.0.0.1");
}

bool LocalAuthServer::start(QString* error)
{
    if (m_server.isListening()) {
        return true;
    }
    if (!m_server.listen(QHostAddress::LocalHost, 0)) {
        if (error) {
            *error = m_server.errorString();
        }
        return false;
    }
    return true;
}

void LocalAuthServer::stop()
{
    if (m_server.isListening()) {
        m_server.close();
    }
    const auto sockets = m_buffers.keys();
    m_buffers.clear();
    for (auto* socket : sockets) {
        socket->abort();
        socket->deleteLater();
    }
}

bool LocalAuthServer::isRunning() const
{
    return m_server.isListening();
}

quint16 LocalAuthServer::port() const
{
    return m_server.serverPort();
}

QString LocalAuthServer::baseUrl() const
{
    if (!m_server.isListening()) {
        return QString();
    }
    return QStringLiteral("http://%1:%2/").arg(host()).arg(m_server.serverPort());
}

void LocalAuthServer::setProfile(const QString& uuid, const QString& name)
{
    m_uuid = uuid;
    m_name = name;
}

void LocalAuthServer::setAccessToken(const QString& accessToken)
{
    m_accessToken = accessToken;
}

void LocalAuthServer::setSkin(const QByteArray& png, const QString& model)
{
    m_skin = png;
    m_skinHash = textureHash(png);
    m_skinModel = (model == QStringLiteral("slim")) ? QStringLiteral("slim") : QStringLiteral("classic");
}

void LocalAuthServer::setCape(const QByteArray& png)
{
    m_cape = png;
    m_capeHash = textureHash(png);
}

QString LocalAuthServer::texturesProperty() const
{
    return QString::fromLatin1(texturesPayloadJson().toBase64());
}

QString LocalAuthServer::prefetchedMetadata() const
{
    return QString::fromLatin1(metadataJson().toBase64());
}

QString LocalAuthServer::profilePropertiesArgument() const
{
    return QString::fromLatin1(profilePropertiesJson());
}

QString LocalAuthServer::skinUrl() const
{
    return QStringLiteral("http://%1:%2/textures/%3").arg(host()).arg(m_server.serverPort()).arg(m_skinHash);
}

QString LocalAuthServer::capeUrl() const
{
    return QStringLiteral("http://%1:%2/textures/%3").arg(host()).arg(m_server.serverPort()).arg(m_capeHash);
}

QByteArray LocalAuthServer::metadataJson() const
{
    QJsonObject meta;
    meta.insert(QStringLiteral("serverName"), QStringLiteral("IAMNX Launcher"));
    meta.insert(QStringLiteral("implementationName"), QStringLiteral("iamnxlauncher"));
    meta.insert(QStringLiteral("implementationVersion"), QStringLiteral("1.0"));

    QJsonObject root;
    root.insert(QStringLiteral("meta"), meta);
    // The textures are served from the loopback interface, so the client has
    // to be told that this domain is allowed.
    QJsonArray skinDomains;
    skinDomains.append(host());
    root.insert(QStringLiteral("skinDomains"), skinDomains);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray LocalAuthServer::profileJson() const
{
    QJsonObject textureProperty;
    textureProperty.insert(QStringLiteral("name"), QStringLiteral("textures"));
    textureProperty.insert(QStringLiteral("value"), texturesProperty());

    QJsonArray properties;
    properties.append(textureProperty);

    QJsonObject profile;
    profile.insert(QStringLiteral("id"), m_uuid);
    profile.insert(QStringLiteral("name"), m_name);
    profile.insert(QStringLiteral("properties"), properties);
    return QJsonDocument(profile).toJson(QJsonDocument::Compact);
}

QByteArray LocalAuthServer::profilePropertiesJson() const
{
    QJsonObject root;
    root.insert(QStringLiteral("textures"), texturesProperty());
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray LocalAuthServer::authenticateJson() const
{
    const auto profile = QJsonDocument::fromJson(profileJson()).object();

    QJsonArray availableProfiles;
    availableProfiles.append(profile);

    QJsonObject user;
    user.insert(QStringLiteral("id"), m_uuid);
    user.insert(QStringLiteral("name"), m_name);
    user.insert(QStringLiteral("properties"), QJsonArray());

    QJsonObject root;
    root.insert(QStringLiteral("accessToken"), m_accessToken);
    root.insert(QStringLiteral("clientToken"), m_accessToken);
    root.insert(QStringLiteral("availableProfiles"), availableProfiles);
    root.insert(QStringLiteral("selectedProfile"), profile);
    root.insert(QStringLiteral("user"), user);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray LocalAuthServer::texturesPayloadJson() const
{
    QJsonObject textures;
    if (!m_skin.isEmpty()) {
        QJsonObject metadata;
        metadata.insert(QStringLiteral("model"), m_skinModel);

        QJsonObject skin;
        skin.insert(QStringLiteral("url"), skinUrl());
        skin.insert(QStringLiteral("metadata"), metadata);
        textures.insert(QStringLiteral("SKIN"), skin);
    }
    if (!m_cape.isEmpty()) {
        QJsonObject cape;
        cape.insert(QStringLiteral("url"), capeUrl());
        textures.insert(QStringLiteral("CAPE"), cape);
    }

    QJsonObject root;
    root.insert(QStringLiteral("timestamp"), QDateTime::currentMSecsSinceEpoch());
    root.insert(QStringLiteral("profileId"), m_uuid);
    root.insert(QStringLiteral("profileName"), m_name);
    root.insert(QStringLiteral("textures"), textures);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

void LocalAuthServer::onNewConnection()
{
    while (m_server.hasPendingConnections()) {
        auto* socket = m_server.nextPendingConnection();
        m_buffers.insert(socket, QByteArray());
        connect(socket, &QTcpSocket::readyRead, this, &LocalAuthServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &LocalAuthServer::onDisconnected);
    }
}

void LocalAuthServer::onReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    auto& buffer = m_buffers[socket];
    buffer += socket->readAll();

    const auto headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        return;
    }

    const auto header = buffer.left(headerEnd);
    const auto lines = header.split('\n');

    int contentLength = 0;
    for (auto line : lines) {
        line = line.trimmed();
        if (line.toLower().startsWith("content-length:")) {
            contentLength = line.mid(15).trimmed().toInt();
        }
    }

    const auto bodyStart = headerEnd + 4;
    if (buffer.size() < bodyStart + contentLength) {
        // The body is not complete yet, wait for more data.
        return;
    }

    const auto body = buffer.mid(bodyStart, contentLength);
    buffer.remove(0, bodyStart + contentLength);

    const auto requestLine = lines.value(0).trimmed();
    const auto parts = requestLine.split(' ');
    if (parts.size() < 2) {
        respond(socket, 400, JSON_CONTENT_TYPE, QByteArray("{\"error\":\"Malformed request\"}"));
        return;
    }

    const auto method = QString::fromLatin1(parts[0]);
    const auto target = QString::fromLatin1(parts[1]);
    handleRequest(socket, method, requestPath(target), body);
}

void LocalAuthServer::onDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }
    m_buffers.remove(socket);
    socket->deleteLater();
}

void LocalAuthServer::handleRequest(QTcpSocket* socket, const QString& method, const QString& path, const QByteArray& body)
{
    Q_UNUSED(body);

    // API metadata
    if (path == "/" || path.isEmpty()) {
        respond(socket, 200, JSON_CONTENT_TYPE, metadataJson());
        return;
    }

    // Authentication
    if (path == "/authserver/authenticate" || path == "/authserver/refresh") {
        respond(socket, 200, JSON_CONTENT_TYPE, authenticateJson());
        return;
    }
    if (path == "/authserver/validate" || path == "/authserver/invalidate") {
        respondEmpty(socket, 204);
        return;
    }
    if (path == "/authserver/signout") {
        respondEmpty(socket, 204);
        return;
    }

    // Sessions
    if (path == "/sessionserver/session/minecraft/join") {
        respondEmpty(socket, 204);
        return;
    }
    if (path == "/sessionserver/session/minecraft/hasJoined") {
        respond(socket, 200, JSON_CONTENT_TYPE, profileJson());
        return;
    }
    if (path.startsWith("/sessionserver/session/minecraft/profile/")) {
        respond(socket, 200, JSON_CONTENT_TYPE, profileJson());
        return;
    }

    // Textures
    if (path.startsWith("/textures/")) {
        const auto hash = path.mid(QStringLiteral("/textures/").size());
        if (!m_skin.isEmpty() && hash == m_skinHash) {
            respond(socket, 200, PNG_CONTENT_TYPE, m_skin);
            return;
        }
        if (!m_cape.isEmpty() && hash == m_capeHash) {
            respond(socket, 200, PNG_CONTENT_TYPE, m_cape);
            return;
        }
        respondEmpty(socket, 404);
        return;
    }

    if (method != "GET" && method != "POST") {
        respondEmpty(socket, 405);
        return;
    }

    respondEmpty(socket, 404);
}

void LocalAuthServer::respond(QTcpSocket* socket, int status, const QByteArray& contentType, const QByteArray& body)
{
    if (!socket) {
        return;
    }
    QByteArray response;
    response += "HTTP/1.1 " + QByteArray::number(status) + " " + statusText(status) + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    response += "Cache-Control: no-store\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;

    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void LocalAuthServer::respondEmpty(QTcpSocket* socket, int status)
{
    respond(socket, status, JSON_CONTENT_TYPE, QByteArray());
}
