#include "wallet_qr_provider.h"

#include <qrcodegen.hpp>

#include <QGuiApplication>

using qrcodegen::QrCode;
using qrcodegen::QrSegment;

WalletQrProvider::WalletQrProvider(const QString &name)
    : QQuickImageProvider(QQuickImageProvider::ImageType::Image), mName{name}
{
    mCache.setMaxCost(1024 * 1024 * 10);
}

QImage WalletQrProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage result;

    if (!mCache.contains(id)) {
        const auto qrcode = QrCode::encodeText(id.toUtf8().data(), QrCode::Ecc::LOW);
        auto *image = new QImage(qrcode.getSize(), qrcode.getSize(), QImage::Format_Mono);
        image->fill(1);
        for (int y = 0; y < qrcode.getSize(); y++) {
            for (int x = 0; x < qrcode.getSize(); x++) {
                if (qrcode.getModule(x, y)) {
                    image->setPixel(x, y, 0);
                }
            }
        }
        mCache.insert(id, image, image->bytesPerLine() * image->height());
    }
    result = *mCache.object(id);

    if (!requestedSize.isEmpty()) {
        result = result.scaled(requestedSize, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    if (size) {
        *size = result.size();
    }

    return result;
}

int WalletQrProvider::cacheSize() const
{
    return mCache.maxCost();
}

void WalletQrProvider::setCacheSize(int cacheSize)
{
    mCache.setMaxCost(cacheSize);
}

const QString &WalletQrProvider::name() const
{
    return mName;
}
