#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <QImage>
#include <QObject>

class ImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit ImageLoader(QObject *parent = nullptr);
    static ImageLoader *instance();

    QImage load(const QString &id) const;
    
    QString getProductImagePath(const QString& listing_key, const QString& image_name/*or image id/index? index wont need a parameter*/) const;
    QString getAvatarImagePath(const QString& user_key, const QString& image_name) const;
};

#endif // IMAGELOADER_H
