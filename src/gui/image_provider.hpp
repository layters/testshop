#ifndef IMAGE_PROVIDER_H
#define IMAGE_PROVIDER_H

#include <QCache>
#include <QImage>
#include <QQuickImageProvider>

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider(const QString &name);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    int cacheSize() const;
    void setCacheSize(int cacheSize);

    const QString &name() const;

private:
    QString mName;
    QCache<QString, QImage> mCache;
};

#endif // IMAGEPROVIDER_H
