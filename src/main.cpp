#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>
#include <QFont>

static QFont loadAppFont(const QString& resourcePath, int pointSize = 10, int weight = QFont::Normal)
{
    const int id = QFontDatabase::addApplicationFont(resourcePath);
    if (id < 0) {
        // fallback: usa fonte padrão do sistema
        return QApplication::font();
    }

    const QStringList families = QFontDatabase::applicationFontFamilies(id);
    // Tipicamente a primeira é a família base do arquivo
    const QString family = families.isEmpty() ? QApplication::font().family() : families.first();

    QFont f;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    f.setFamilies({ family }); // Qt 6: aceita lista de famílias (para fallbacks, ver seção 4)
#else
    f.setFamily(family);
#endif
    f.setPointSize(10);          // escolha padrão “visual” da sua app
    f.setWeight(QFont::Light);                // QFont::Medium, QFont::DemiBold, etc.
    f.setHintingPreference(QFont::PreferDefaultHinting);
    return f;
}


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    // Em Qt 5, habilitar alta-DPI antes de QApplication
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);

    // Carrega regular e bold (opcional: outros pesos ou itálicos)
    // QFont base = loadAppFont(":/fonts/Gilroy.otf", 10, QFont::Light);
    // QFontDatabase::addApplicationFont(":/fonts/Gilroy.otf");
    QFont base = loadAppFont(":/fonts/PathwayExtreme.ttf", 10, QFont::Light);
    QFontDatabase::addApplicationFont(":/fonts/PathwayExtreme.ttf");

    // Define globalmente para TODOS os widgets
    a.setFont(base);


    QCoreApplication::setOrganizationName("SequelFastTeam");
    QCoreApplication::setApplicationName("SequelFast");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    qDebug() << "Versão:" << QCoreApplication::applicationVersion();

    a.setWindowIcon(QIcon(":/icons/SequelFast.png")); // ou qualquer tamanho desejado

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SequelFast_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;

    w.show();

    return a.exec();
}
