#include "Product.h"

Product::Product() 
    : productId(0), price(0.0), stock(0), 
      status(ProductStatus::PENDING_APPROVAL) {
    registrationDate = QDateTime::currentDateTime();
}

