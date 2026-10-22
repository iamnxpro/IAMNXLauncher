# IAMNX Launcher

**IAMNX Launcher** هو لانشر شخصي مخصص لماينكرافت، مبني على كود Prism Launcher ومعدّل لإدارة الحسابات المحلية وتحسين تجربة التشغيل على الأجهزة الضعيفة.

> هذا المشروع نسخة شخصية مستقلة وغير تابعة أو معتمدة من Prism Launcher أو MultiMC.

## الميزات الحالية

- إنشاء وإدارة حسابات Offline محلية دون اشتراط تسجيل دخول Microsoft.
- إعادة تسمية الحساب المحلي ونسخه وحذفه وتعيين الحساب الافتراضي.
- اختيار سكن PNG محلي لكل حساب Offline، مع حفظه داخل ملف الحسابات.
- اختيار كيب PNG محلي للحساب، لاستخدامه مع المود أو طبقة العرض المناسبة.
- تحسينات Windows محدودة بنطاق عملية Java مثل أولوية العملية والمؤقت منخفض التأخير.
- بناء تلقائي لنسخة Windows x64 عبر GitHub Actions.

## تنبيه مهم حول الحسابات المحلية

الحسابات المحلية مناسبة للعب الفردي أو الخوادم التي تسمح بالوضع غير المتصل. لا تمنح هذه الحسابات ملكية Minecraft ولا تتجاوز مصادقة الخوادم الرسمية أو متطلبات Microsoft.

## رفع المشروع إلى GitHub عبر Git Bash

لا ترفع ملف ZIP نفسه إلى GitHub. يجب فك ضغطه ثم رفع محتوياته ومجلداته مع الحفاظ على بنية المشروع.

بعد إنشاء مستودع فارغ على GitHub، افتح Git Bash داخل مجلد المشروع ونفّذ الأوامر التالية، مع استبدال الرابط برابط مستودعك:

```bash
git init
git branch -M main
git add .
git commit -m "Initial IAMNX Launcher source"
git remote add origin https://github.com/USERNAME/REPOSITORY.git
git push -u origin main
```

يجب أن يكون شكل المستودع من الداخل مشابهًا لما يلي:

```text
IAMNXLauncher/
├── .github/
│   └── workflows/
│       └── windows-build.yml
├── launcher/
├── libraries/
├── cmake/
├── program_info/
├── scripts/
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json
└── vcpkg-configuration.json
```

يجب أن يكون `CMakeLists.txt` في جذر المستودع مباشرة، وليس داخل مجلد إضافي ناتج عن فك الضغط.

## تحويل المشروع إلى EXE باستخدام GitHub Actions

يوجد Workflow جاهز في:

```text
.github/workflows/windows-build.yml
```

بعد تنفيذ `git push`:

1. افتح صفحة المستودع على GitHub.
2. انتقل إلى تبويب **Actions**.
3. اختر **Build IAMNX Launcher (Windows)**.
4. اضغط **Run workflow** إذا لم يبدأ البناء تلقائيًا.
5. انتظر حتى تنتهي العملية بعلامة خضراء.
6. افتح عملية البناء الناجحة.
7. من قسم **Artifacts** حمّل:

```text
IAMNXLauncher-windows-x64
```

سيكون الملف المضغوط الناتج قابلًا للفك على Windows، وبعد فك الضغط شغّل ملف `iamnxlauncher.exe` إن وُجد بهذا الاسم.

يستخدم Workflow بيئة Windows ويثبت تلقائيًا:

- Qt 6.8.
- CMake وNinja المتوفرين في بيئة GitHub.
- مكتبات المشروع عبر vcpkg.
- إعداد Release مع تفعيل LTO.

## البناء المحلي على Windows

للبناء محليًا تحتاج إلى Visual Studio 2022 مع Desktop development with C++، وQt 6.8، وCMake، وGit، وvcpkg. البناء عبر GitHub Actions هو الأسهل إذا لم تكن هذه الأدوات مثبتة لديك.

## التراخيص والاعتمادات

IAMNX Launcher مبني على مشروع Prism Launcher. يجب الاحتفاظ بملف `LICENSE` واحترام شروط GPL-3.0-only وأي تراخيص للمكونات التابعة. هذا المشروع ليس إصدارًا رسميًا من Prism Launcher.

إذا أعدت توزيع نسخة عامة، راجع مفاتيح API وشروط الخدمات الموجودة في `CMakeLists.txt`، واستبدل المفاتيح الخاصة بالمشاريع الأصلية أو عطّل الميزات التي لا تحتاجها.

## المساهمة والدعم

للمشاكل البرمجية، افتح Issue داخل مستودع IAMNX الخاص بك مع إرفاق سجل GitHub Actions أو رسالة الخطأ كاملة. لا تضع كلمات مرور أو رموز وصول أو مفاتيح API سرية داخل Issues أو ملفات المستودع.
# IAMNXLauncher
