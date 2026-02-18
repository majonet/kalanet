#ifndef USER_H
#define USER_H

#include "qdatetime.h"
#include <QString>
#include <QDataStream>
#include <QVector>
#include <QMap>

// Forward declarations
class Product;

enum class UserType {
    ADMIN,
    CUSTOMER
};

// Transaction record for purchase history
struct Transaction {
    int productId;
    QString productName;
    QString sellerUsername;
    QString buyerUsername;
    int quantity;
    double totalPrice;
    QDateTime date;

    void saveToStream(QDataStream& stream) const;
    void loadFromStream(QDataStream& stream);
};

class User {
protected:
    QString username;
    QString hashedPassword;
    QString email;
    QString phone;
    QString address;
    double walletBalance;
    UserType userType;

public:
    User();
    User(const QString& user, const QString& pass, const QString& mail, 
         const QString& ph, const QString& addr, UserType type);
    virtual ~User() = default;

    // Getters
    QString getUsername() const { return username; }
    QString getHashedPassword() const { return hashedPassword; }
    QString getEmail() const { return email; }
    QString getPhone() const { return phone; }
    QString getAddress() const { return address; }
    double getWalletBalance() const { return walletBalance; }
    UserType getType() const { return userType; }

    // Setters
    void setUsername(const QString& user) { username = user; }
    void setHashedPassword(const QString& pass) { hashedPassword = pass; }
    void setEmail(const QString& mail) { email = mail; }
    void setPhone(const QString& ph) { phone = ph; }
    void setAddress(const QString& addr) { address = addr; }
    void setWalletBalance(double balance) { walletBalance = balance; }

    // Wallet operations
    void addFunds(double amount) { walletBalance += amount; }
    bool deductFunds(double amount);

    // Virtual methods
    virtual UserType getUserType() const = 0;
    virtual QString getUserTypeString() const = 0;
    virtual bool canRegisterProducts() const { return false; }

    // Serialization
    virtual void saveToStream(QDataStream& stream) const;
    virtual void loadFromStream(QDataStream& stream);

    // Static password hashing
    static QString hashPassword(const QString& password);
    static bool validatePassword(const QString& password);
};

class Admin : public User {
public:
    Admin();
    Admin(const QString& user, const QString& pass, const QString& mail, 
          const QString& ph, const QString& addr);

    UserType getUserType() const override { return UserType::ADMIN; }
    QString getUserTypeString() const override { return "Admin"; }
    bool canRegisterProducts() const override { return true; }

    void saveToStream(QDataStream& stream) const override;
    void loadFromStream(QDataStream& stream) override;
};

class Customer : public User {
private:
    QMap<int, int> cart; // productId -> quantity
    QVector<Transaction> purchaseHistory;
    QVector<int> registeredProductIds; // Products this customer registered

public:
    Customer();
    Customer(const QString& user, const QString& pass, const QString& mail, 
             const QString& ph, const QString& addr);

    UserType getUserType() const override { return UserType::CUSTOMER; }
    QString getUserTypeString() const override { return "Customer"; }
    bool canRegisterProducts() const override { return true; }

    // Cart operations
    void addToCart(int productId, int quantity);
    void removeFromCart(int productId);
    void clearCart() { cart.clear(); }
    QMap<int, int>& getCart() { return cart; }
    const QMap<int, int>& getCart() const { return cart; }
    bool isInCart(int productId) const { return cart.contains(productId); }

    // Purchase history
    void addTransaction(const Transaction& trans);
    QVector<Transaction>& getPurchaseHistory() { return purchaseHistory; }
    const QVector<Transaction>& getPurchaseHistory() const { return purchaseHistory; }

    // Registered products
    void addRegisteredProduct(int productId);
    QVector<int>& getRegisteredProductIds() { return registeredProductIds; }
    const QVector<int>& getRegisteredProductIds() const { return registeredProductIds; }

    void saveToStream(QDataStream& stream) const override;
    void loadFromStream(QDataStream& stream) override;
};

#endif // USER_H