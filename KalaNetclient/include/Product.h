#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>
#include <QDateTime>
#include <QDataStream>
#include <QImage>

enum class ProductStatus {
    PENDING_APPROVAL,
    APPROVED,
    SOLD
};

class Product {
private:
    int productId;
    QString name;
    QString description;
    QString category;
    double price;
    int stock;
    QString sellerUsername;
    ProductStatus status;
    QDateTime registrationDate;
    QString imagePath;
    QString imageBase64;  // Store image as base64 string

public:
    Product();
    Product(int id, const QString& name, const QString& description,
            const QString& category, double price, int stock,
            const QString& seller);

    // Getters
    int getProductId() const { return productId; }
    QString getName() const { return name; }
    QString getDescription() const { return description; }
    QString getCategory() const { return category; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    QString getSellerUsername() const { return sellerUsername; }
    ProductStatus getStatus() const { return status; }
    QDateTime getRegistrationDate() const { return registrationDate; }
    QString getImagePath() const { return imagePath; }
    QString getImageBase64() const { return imageBase64; }

    // Setters
    void setProductId(int id) { productId = id; }
    void setName(const QString& newName) { name = newName; }
    void setDescription(const QString& desc) { description = desc; }
    void setCategory(const QString& cat) { category = cat; }
    void setPrice(double newPrice) { price = newPrice; }
    void setStock(int newStock) { stock = newStock; }
    void setSellerUsername(const QString& seller) { sellerUsername = seller; }
    void setStatus(ProductStatus newStatus) { status = newStatus; }
    void setRegistrationDate(const QDateTime& date) { registrationDate = date; }
    void setImagePath(const QString& path) { imagePath = path; }
    void setImageBase64(const QString& base64) { imageBase64 = base64; }

    // Status helpers
    bool isApproved() const { return status == ProductStatus::APPROVED; }
    bool isPending() const { return status == ProductStatus::PENDING_APPROVAL; }
    bool isSold() const { return status == ProductStatus::SOLD; }
    bool hasImage() const { return !imageBase64.isEmpty(); }

    QString getStatusString() const;

    // Image handling
    QImage getImage() const;
    void setImage(const QImage& image);
    void clearImage() { imageBase64.clear(); }

    // Static helper for image processing
    static QString imageToBase64(const QImage& image, int maxWidth = 400, int maxHeight = 400);
    static QImage base64ToImage(const QString& base64);
    static QImage resizeImage(const QImage& source, int maxWidth, int maxHeight);

    // Serialization
    void saveToStream(QDataStream& stream) const;
    void loadFromStream(QDataStream& stream);

    // Reduce stock when purchased
    bool purchase(int quantity);
};

#endif // PRODUCT_H
