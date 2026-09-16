#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    // Em Qt 5, habilitar alta-DPI antes de QApplication
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);

    // Mantém a família de fonte padrão do sistema (via QPA platform theme /
    // fontconfig), mas fixa um tamanho compacto: o tamanho de fonte da
    // interface do desktop é pensado para textos de janelas comuns, e fica
    // grande demais numa UI densa de tabelas/grades como a da SequelFast.
    QFont appFont = QApplication::font();
    appFont.setPointSizeF(9.0);
    a.setFont(appFont);

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
