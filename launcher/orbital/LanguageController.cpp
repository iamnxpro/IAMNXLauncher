// SPDX-License-Identifier: GPL-3.0-only
/*
 * IAMNX Launcher - Arabic/English UI language controller for the orbital shell.
 */

#include "LanguageController.h"

#include "Application.h"
#include "settings/SettingsObject.h"

#include <QHash>

namespace {
struct Entry {
    const char* en;
    const char* ar;
};

const QHash<QString, Entry>& entries()
{
    static const QHash<QString, Entry> table = {
        // Shell / navigation
        { "app.name", { "IAMNX", "IAMNX" } },
        { "app.tagline", { "ORBITAL LAUNCHER", "المشغّل المداري" } },
        { "nav.add", { "Add instance", "إضافة نسخة" } },
        { "nav.accounts", { "Accounts", "الحسابات" } },
        { "nav.skins", { "Skins", "المظاهر" } },
        { "nav.folders", { "Instance folder", "مجلد النسخة" } },
        { "nav.settings", { "Settings", "الإعدادات" } },
        { "nav.help", { "Help", "المساعدة" } },
        { "nav.manage", { "Manage instance", "إدارة النسخة" } },
        { "nav.security", { "Launcher lock", "قفل المشغّل" } },
        // Core
        { "core.play", { "PLAY", "العب" } },
        { "core.launching", { "LAUNCHING", "جارٍ التشغيل" } },
        { "core.running", { "RUNNING", "قيد التشغيل" } },
        { "core.noInstance", { "NO INSTANCE", "لا توجد نسخة" } },
        { "core.ready", { "Reactor online", "النواة جاهزة" } },
        { "core.select", { "Select an instance", "اختر نسخة" } },
        { "core.hint", { "Hover the core. She will try to help.", "مرّر فوق النواة. ستحاول مساعدتك." } },
        { "library.instances", { "ORBITAL INSTANCES", "النسخ المدارية" } },
        { "library.empty", { "No instances yet", "لا توجد نسخ بعد" } },
        { "library.emptyHint", { "Add your first instance to wake the core.", "أضف أول نسخة لإيقاظ النواة." } },
        { "library.create", { "Create instance", "إنشاء نسخة" } },
        // Accounts
        { "account.none", { "No account", "لا يوجد حساب" } },
        { "account.offline", { "Offline", "دون اتصال" } },
        { "account.microsoft", { "Microsoft", "مايكروسوفت" } },
        { "account.manage", { "Manage accounts", "إدارة الحسابات" } },
        // Status / errors
        { "status.ready", { "Ready to launch", "جاهز للتشغيل" } },
        { "status.launching", { "Opening the portal...", "جارٍ فتح البوابة..." } },
        { "status.starting", { "Waiting for Minecraft...", "بانتظار ماينكرافت..." } },
        { "status.timeout", { "The launch is taking too long.", "استغرق التشغيل وقتاً طويلاً." } },
        { "error.noInstance", { "Create or select an instance first.", "أنشئ نسخة أو اختر واحدة أولاً." } },
        { "error.noAccount", { "Add an account before launching.", "أضف حساباً قبل التشغيل." } },
        { "error.cannotLaunch", { "This instance cannot be launched right now.", "لا يمكن تشغيل هذه النسخة الآن." } },
        { "error.launchFailed", { "The launch failed. Check the logs.", "فشل التشغيل. تحقّق من السجلات." } },
        { "error.addFailed", { "Could not open the instance creator.", "تعذّر فتح منشئ النسخ." } },
        { "error.dialogFailed", { "Could not open that window.", "تعذّر فتح تلك النافذة." } },
        { "error.pinLocked", { "Too many attempts. Wait a moment.", "محاولات كثيرة. انتظر قليلاً." } },
        // PIN
        { "pin.title", { "Welcome back", "مرحباً بعودتك" } },
        { "pin.subtitle", { "Enter your IAMNX launcher PIN", "أدخل رمز مشغّل IAMNX" } },
        { "pin.placeholder", { "PIN or password", "الرمز أو كلمة المرور" } },
        { "pin.unlock", { "Unlock", "فتح" } },
        { "pin.show", { "Show password", "إظهار كلمة المرور" } },
        { "pin.hide", { "Hide password", "إخفاء كلمة المرور" } },
        { "pin.error", { "That PIN is not right. Try again.", "هذا الرمز غير صحيح. حاول مرة أخرى." } },
        { "pin.locked", { "Locked for a few seconds.", "مقفل لبضع ثوانٍ." } },
        { "pin.localOnly", { "Local launcher lock only. Your Microsoft account is never stored here.", "قفل محلي للمشغّل فقط. لا يُخزَّن حساب مايكروسوفت هنا أبداً." } },
        { "pin.enter", { "Unlock the launcher", "فتح المشغّل" } },
        // Security panel
        { "security.title", { "Launcher lock", "قفل المشغّل" } },
        { "security.desc", { "Protect this launcher with a local PIN. This does not change your Minecraft account password.", "احمِ هذا المشغّل برمز محلي. هذا لا يغيّر كلمة مرور حساب ماينكرافت." } },
        { "security.new", { "New PIN", "رمز جديد" } },
        { "security.confirm", { "Confirm PIN", "تأكيد الرمز" } },
        { "security.save", { "Save PIN", "حفظ الرمز" } },
        { "security.remove", { "Remove PIN", "إزالة الرمز" } },
        { "security.close", { "Close", "إغلاق" } },
        { "security.set", { "Set launcher PIN", "تعيين رمز المشغّل" } },
        { "security.short", { "Use at least 4 characters.", "استخدم 4 أحرف على الأقل." } },
        { "security.mismatch", { "The PINs do not match.", "الرمزان غير متطابقين." } },
        { "security.saved", { "Launcher PIN saved.", "تم حفظ رمز المشغّل." } },
        { "security.removed", { "Launcher PIN removed.", "تمت إزالة رمز المشغّل." } },
        { "security.enabled", { "A launcher PIN is active. Enter a new PIN to replace it.", "رمز المشغّل مفعّل. أدخل رمزاً جديداً لاستبداله." } },
        { "security.none", { "No PIN is set. The launcher opens directly.", "لا يوجد رمز. يفتح المشغّل مباشرة." } },
        // Companion speech
        { "bubble.wave", { "Hey! Welcome back.", "مرحباً! أهلاً بعودتك." } },
        { "bubble.shy", { "I did not see anything!", "لم أرَ شيئاً!" } },
        { "bubble.facepalm", { "Ouch. That was painful.", "آخ. كان ذلك مؤلماً." } },
        { "bubble.reach", { "Let me press it for you...", "دعني أضغطه عنك..." } },
        { "bubble.fail", { "So close!", "كدت أصل!" } },
        { "bubble.portal", { "Hold on tight!", "تمسّك جيداً!" } },
        { "bubble.ready", { "The core is ready.", "النواة جاهزة." } },
        // Misc
        { "common.version", { "Version", "الإصدار" } },
        { "common.language", { "Language", "اللغة" } },
        { "common.quit", { "Quit", "خروج" } },
        { "common.logs", { "Logs", "السجلات" } },
    };
    return table;
}
}  // namespace

LanguageController::LanguageController(QObject* parent) : QObject(parent)
{
    if (APPLICATION) {
        const QString saved = APPLICATION->settings()->get("OrbitalLanguage").toString();
        if (saved == QStringLiteral("ar") || saved == QStringLiteral("en")) {
            m_code = saved;
        }
    }
}

void LanguageController::setLanguage(const QString& code)
{
    const QString normalized = code.startsWith(QStringLiteral("ar"), Qt::CaseInsensitive) ? QStringLiteral("ar") : QStringLiteral("en");
    if (normalized == m_code) {
        return;
    }
    m_code = normalized;
    ++m_revision;
    if (APPLICATION) {
        APPLICATION->settings()->set("OrbitalLanguage", m_code);
    }
    emit languageChanged();
}

void LanguageController::toggleLanguage()
{
    setLanguage(m_code == QStringLiteral("ar") ? QStringLiteral("en") : QStringLiteral("ar"));
}

QString LanguageController::lookup(const QString& key) const
{
    const auto it = entries().constFind(key);
    if (it == entries().constEnd()) {
        return key;
    }
    return m_code == QStringLiteral("ar") ? QString::fromUtf8(it->ar) : QString::fromUtf8(it->en);
}

QString LanguageController::t(const QString& key, int) const
{
    return lookup(key);
}
