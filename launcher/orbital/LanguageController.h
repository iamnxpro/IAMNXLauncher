// SPDX-License-Identifier: GPL-3.0-only
/*
 * IAMNX Launcher - Arabic/English UI language controller for the orbital shell.
 */

#pragma once

#include <QObject>
#include <QString>

class LanguageController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString code READ code NOTIFY languageChanged)
    Q_PROPERTY(bool rtl READ rtl NOTIFY languageChanged)
    Q_PROPERTY(int revision READ revision NOTIFY languageChanged)

   public:
    explicit LanguageController(QObject* parent = nullptr);

    QString code() const { return m_code; }
    bool rtl() const { return m_code == QStringLiteral("ar"); }
    int revision() const { return m_revision; }

    Q_INVOKABLE void setLanguage(const QString& code);
    Q_INVOKABLE void toggleLanguage();

    // The revision argument is intentionally part of the QML signature. Reading it
    // during a binding makes QML re-evaluate translations when the language changes.
    Q_INVOKABLE QString t(const QString& key, int revision = 0) const;

   signals:
    void languageChanged();

   private:
    QString lookup(const QString& key) const;

    QString m_code = QStringLiteral("en");
    int m_revision = 0;
};
