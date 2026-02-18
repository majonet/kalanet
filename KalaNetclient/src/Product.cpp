#include "Product.h"

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

void Product::saveToStream(QDataStream& stream) const {
    stream << productId << name << description << category 
           << price << stock << sellerUsername 
           << static_cast<int>(status) << registrationDate << imagePath;
}

void Product::loadFromStream(QDataStream& stream) {
    int statusInt;
    stream >> productId >> name >> description >> category 
           >> price >> stock >> sellerUsername 
           >> statusInt >> registrationDate >> imagePath;
    status = static_cast<ProductStatus>(statusInt);
}