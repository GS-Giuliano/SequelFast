#include "connection.h"
#include "connectionbackend.h"
#include "omarchypalette.h"

#include <QQuickWidget>
#include <QQmlContext>
#include <QVBoxLayout>
#include <QUrl>

Connection::Connection(QString selectedHost, QWidget* parent)
    : QDialog(parent)
    , backend(new ConnectionBackend(selectedHost, this))
    , quickWidget(new QQuickWidget(this))
{
    setWindowTitle(tr("Connection"));
    setFixedSize(420, 420);

    quickWidget->rootContext()->setContextProperty("backend", backend);
    quickWidget->rootContext()->setContextProperty("omarchyPalette", new OmarchyPalette(this));
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl(QStringLiteral("qrc:/qml/Connection.qml")));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(quickWidget);

    connect(backend, &ConnectionBackend::accepted, this, &QDialog::accept);
    connect(backend, &ConnectionBackend::rejected, this, &QDialog::reject);
}

Connection::~Connection()
{
}
