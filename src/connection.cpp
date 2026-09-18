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
    // 420x420 was too tight for the Name/tabs/fields/color-swatches/checkbox/
    // buttons column, clipping and misaligning the bottom rows; give it more
    // room and let the user resize instead of locking the dialog's size.
    resize(480, 560);
    setMinimumSize(460, 520);

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
