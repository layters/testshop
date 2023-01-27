#ifndef WALLETQRPROVIDER_H
#define WALLETQRPROVIDER_H

#include <QCache>
#include <QImage>
#include <QQuickImageProvider>

class WalletQrProvider : public QQuickImageProvider
{
public:
    WalletQrProvider(const QString &name);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    int cacheSize() const;
    void setCacheSize(int cacheSize);

    const QString &name() const;

private:
    QString mName;
    QCache<QString, QImage> mCache;
};

#endif // WALLETQRPROVIDER_H
