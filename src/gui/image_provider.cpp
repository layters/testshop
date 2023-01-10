#include "image_provider.h"

#include "image_loader.h"
#include <QGuiApplication>

ImageProvider::ImageProvider(const QString &name)
    : QQuickImageProvider(QQuickImageProvider::ImageType::Image), mName{name}
{
    mCache.setMaxCost(1024 * 1024 * 10);
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage result;

    if (!mCache.contains(id)) {
        auto *image = new QImage(ImageLoader::instance()->load(QString("%1/%2").arg(mName, id)));
        mCache.insert(id, image, image->bytesPerLine() * image->height());
    }
    result = *mCache.object(id);

    if (!requestedSize.isEmpty()) {
        result = result.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (size) {
        *size = result.size();
    }

    return result;
}

int ImageProvider::cacheSize() const
{
    return mCache.maxCost();
}

void ImageProvider::setCacheSize(int cacheSize)
{
    mCache.setMaxCost(cacheSize);
}

const QString &ImageProvider::name() const
{
    return mName;
}
