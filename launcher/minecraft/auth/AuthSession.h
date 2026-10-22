#pragma once

#include <QByteArray>
#include <QString>
#include <memory>

#include "LaunchMode.h"

class MinecraftAccount;

/**
 * The mechanism that is used to hand a custom skin to the game.
 *
 * Vanilla Minecraft only knows two ways of getting a skin into the client:
 * the "textures" property of the game profile and the authlib-injector java
 * agent, which replaces the Yggdrasil server the client talks to.
 */
enum class SkinDelivery {
    None,              //!< The Minecraft version does not support custom skins.
    ProfileProperties, //!< The "--profileProperties" argument, Minecraft 1.8.1 - 1.21.8.
    AuthlibInjector,   //!< The authlib-injector java agent, Minecraft 1.21.9 and newer.
};

struct AuthSession {
    bool MakeOffline(QString offline_playername);
    void MakeDemo(QString name, QString uuid);

    QString serializeUserProperties();

    // combined session ID
    QString session;
    // volatile auth token
    QString access_token;
    // profile name
    QString player_name;
    // profile ID
    QString uuid;
    // 'msa' or 'offline', depending on account type
    QString user_type;
    // the actual launch mode for this session
    LaunchMode launchMode;

    ///// Custom skin of an offline account /////

    // The raw PNG data of the skin, empty when the account has no custom skin.
    QByteArray skinData;
    // The model of the skin, either "classic" or "slim".
    QString skinModel = QStringLiteral("classic");
    // The raw PNG data of the cape, empty when the account has no custom cape.
    QByteArray capeData;

    // How the custom skin is handed to the game.
    SkinDelivery skinDelivery = SkinDelivery::None;
    // The value of the "--profileProperties" argument, e.g. {"textures":"<base64>"}.
    QString profileProperties;
    // The base URL of the local authentication server, e.g. "http://127.0.0.1:1234/".
    QString authServerUrl;
    // The base64 encoded API metadata for "authlibinjector.yggdrasil.prefetched".
    QString authlibMetadata;
};

using AuthSessionPtr = std::shared_ptr<AuthSession>;
