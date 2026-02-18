#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include "User.h"
#include "Product.h"

class DataManager : public QObject {
    Q_OBJECT

private:
    static DataManager* instance;
    static QMutex instanceMutex;

    QMap<QString, User*> users;
    QMap<int, Product*> products;

    int nextProductId;
    mutable QMutex dataMutex;

    QString dataDir;
    QString usersFile;
    QString productsFile;

    DataManager(QObject* parent = nullptr);

    // CSV helpers
    QString escapeCSV(const QString& str);
    QString unescapeCSV(const QString& str);

public:
    static DataManager* getInstance();
    static void destroyInstance();

    // User management
    bool addUser(User* user);
    User* getUser(const QString& username);
    bool userExists(const QString& username) const;
    bool validateLogin(const QString& username, const QString& password) const;
    QVector<User*> getAllUsers() const;

    // Product management
    bool addProduct(Product* product);
    Product* getProduct(int productId);
    bool removeProduct(int productId);
    QVector<Product*> getAllProducts() const;
    QVector<Product*> getApprovedProducts() const;
    QVector<Product*> getPendingProducts() const;
    QVector<Product*> getProductsByCategory(const QString& category) const;
    QVector<Product*> searchProducts(const QString& searchTerm) const;

    // Approval system
    bool approveProduct(int productId);
    bool rejectProduct(int productId);

    // ID generation
    int getNextProductId();

    // CSV Data persistence
    bool saveAllData();
    bool loadAllData();

    // Legacy method names for compatibility (call CSV versions)
    bool saveUsers() { return saveUsersToCSV(); }
    bool loadUsers() { return loadUsersFromCSV(); }
    bool saveProducts() { return saveProductsToCSV(); }
    bool loadProducts() { return loadProductsFromCSV(); }

    bool saveUsersToCSV();
    bool loadUsersFromCSV();
    bool saveProductsToCSV();
    bool loadProductsFromCSV();
    bool saveTransactionsToCSV();
    bool loadTransactionsFromCSV();
    bool saveCartToCSV();
    bool loadCartFromCSV();

    // Statistics
    int getTotalUsers() const { return users.size(); }
    int getTotalProducts() const { return products.size(); }
    int getPendingCount() const;

signals:
    void dataChanged();
    void productApproved(int productId);
    void productRejected(int productId);
};

#endif // DATAMANAGER_H
