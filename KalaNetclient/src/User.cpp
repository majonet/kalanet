#include "User.h"
#include <QCryptographicHash>
#include <QRegularExpression>

// Transaction implementation
void Transaction::saveToStream(QDataStream& stream) const {
    stream << productId << productName << sellerUsername << buyerUsername 
           << quantity << totalPrice << date;
}

void Transaction::loadFromStream(QDataStream& stream) {
    stream >> productId >> productName >> sellerUsername >> buyerUsername 
           >> quantity >> totalPrice >> date;
}

// User base class implementation
User::User() 
    : walletBalance(0.0), userType(UserType::CUSTOMER) {
}

User::User(const QString& user, const QString& pass, const QString& mail, 
           const QString& ph, const QString& addr, UserType type)
    : username(user), hashedPassword(pass), email(mail), 
      phone(ph), address(addr), walletBalance(0.0), userType(type) {
}

bool User::deductFunds(double amount) {
    if (amount <= 0 || amount > walletBalance) {
        return false;
    }
    walletBalance -= amount;
    return true;
}

void User::saveToStream(QDataStream& stream) const {
    stream << username << hashedPassword << email << phone << address 
           << walletBalance << static_cast<int>(userType);
}

void User::loadFromStream(QDataStream& stream) {
    int typeInt;
    stream >> username >> hashedPassword >> email >> phone >> address 
           >> walletBalance >> typeInt;
    userType = static_cast<UserType>(typeInt);
}

QString User::hashPassword(const QString& password) {
    QByteArray hash = QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool User::validatePassword(const QString& password) {
    if (password.length() < 8) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false;
    for (const QChar& c : password) {
        if (c.isUpper()) hasUpper = true;
        if (c.isLower()) hasLower = true;
        if (c.isDigit()) hasDigit = true;
    }

    return hasUpper && hasLower && hasDigit;
}

// Admin implementation
Admin::Admin() : User() {
    userType = UserType::ADMIN;
}

Admin::Admin(const QString& user, const QString& pass, const QString& mail, 
             const QString& ph, const QString& addr)
    : User(user, pass, mail, ph, addr, UserType::ADMIN) {
}

void Admin::saveToStream(QDataStream& stream) const {
    User::saveToStream(stream);
}

void Admin::loadFromStream(QDataStream& stream) {
    User::loadFromStream(stream);
}

// Customer implementation
Customer::Customer() : User() {
    userType = UserType::CUSTOMER;
}

Customer::Customer(const QString& user, const QString& pass, const QString& mail, 
                   const QString& ph, const QString& addr)
    : User(user, pass, mail, ph, addr, UserType::CUSTOMER) {
}

void Customer::addToCart(int productId, int quantity) {
    if (cart.contains(productId)) {
        cart[productId] += quantity;
    } else {
        cart[productId] = quantity;
    }
}

