#include "Product.h"
#include <QBuffer>
#include <QImageReader>
#include <QImageWriter>

Product::Product()
    : productId(0), price(0.0), stock(0),
    status(ProductStatus::PENDING_APPROVAL) {
    registrationDate = QDateTime::currentDateTime();
}

Product::Product(int id, const QString& name, const QString& description,
                 const QString& category, double price, int stock,
                 const QString& seller)
    : productId(id), name(name), description(description),
    category(category), price(price), stock(stock),
    sellerUsername(seller), status(ProductStatus::PENDING_APPROVAL) {
    registrationDate = QDateTime::currentDateTime();
}

QString Product::getStatusString() const {
    switch(status) {
    case ProductStatus::PENDING_APPROVAL:
        return "Pending Approval";
    case ProductStatus::APPROVED:
        return "Approved";
    case ProductStatus::SOLD:
        return "Sold";
    default:
        return "Unknown";
    }
}

bool Product::purchase(int quantity) {
    if (quantity <= 0 || quantity > stock) {
        return false;
    }
    stock -= quantity;
    if (stock == 0) {
        status = ProductStatus::SOLD;
    }
    return true;
}

// Image handling implementation
QImage Product::getImage() const {
    return base64ToImage(imageBase64);
}

void Product::setImage(const QImage& image) {
    imageBase64 = imageToBase64(image);
}

QString Product::imageToBase64(const QImage& image, int maxWidth, int maxHeight) {
    if (image.isNull()) {
        return QString();
    }

    // Resize if too large
    QImage resized = resizeImage(image, maxWidth, maxHeight);

    // Save to buffer as JPEG for compression
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    // Use JPEG for smaller size (85% quality for good balance)
    resized.save(&buffer, "JPEG", 85);

    // Convert to base64
    return QString::fromLatin1(byteArray.toBase64());
}

QImage Product::base64ToImage(const QString& base64) {
    if (base64.isEmpty()) {
        return QImage();
    }

    QByteArray byteArray = QByteArray::fromBase64(base64.toLatin1());
    QImage image;
    image.loadFromData(byteArray);
    return image;
}

QImage Product::resizeImage(const QImage& source, int maxWidth, int maxHeight) {
    if (source.isNull()) {
        return QImage();
    }

    // Only resize if larger than max dimensions
    if (source.width() <= maxWidth && source.height() <= maxHeight) {
        return source;
    }

    // Calculate new size maintaining aspect ratio
    QSize newSize = source.size();
    newSize.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);

    return source.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void Product::saveToStream(QDataStream& stream) const {
    stream << productId << name << description << category
           << price << stock << sellerUsername
           << static_cast<int>(status) << registrationDate << imagePath << imageBase64;
}

void Product::loadFromStream(QDataStream& stream) {
    int statusInt;
    stream >> productId >> name >> description >> category
        >> price >> stock >> sellerUsername
        >> statusInt >> registrationDate >> imagePath >> imageBase64;
    status = static_cast<ProductStatus>(statusInt);
}
