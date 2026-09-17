#include "statistics.h"
#include "statisticsbackend.h"
#include "omarchypalette.h"

#include <QQuickWidget>
#include <QQmlContext>
#include <QVBoxLayout>
#include <QUrl>

Statistics::Statistics(QString& host, QString& schema, QWidget* parent)
    : QDialog(parent)
    , backend(new StatisticsBackend(host, schema, this))
    , quickWidget(new QQuickWidget(this))
{
    setWindowTitle(host + " • " + schema);
    resize(480, 600);

    quickWidget->rootContext()->setContextProperty("backend", backend);
    quickWidget->rootContext()->setContextProperty("omarchyPalette", new OmarchyPalette(this));
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl(QStringLiteral("qrc:/qml/Statistics.qml")));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(quickWidget);

    connect(backend, &StatisticsBackend::closed, this, &QDialog::accept);
}

Statistics::~Statistics()
{
}
