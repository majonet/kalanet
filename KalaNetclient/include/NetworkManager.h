#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QMap>
#include <QVector>
#include "User.h"
#include "Product.h"

class NetworkManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(NetworkManager)

public:
    static NetworkManager* instance();
    static void destroy();

    bool connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();

    // Authentication
    void login(const QString& username, const QString& password);
    void signup(const QString& username, const QString& password,
                const QString& email, const QString& phone,
                const QString& address, UserType type);

    // Product browsing
    void getApprovedProducts();
    void getPendingProducts();
    void getProductDetails(int productId);

    // Product management
    void addProduct(const QString& name, const QString& description,
                    const QString& category, double price, int stock,
                    const QString& seller);
    void approveProduct(int productId);
    void rejectProduct(int productId);

    // Cart operations
    void addToCart(const QString& username, int productId, int quantity);
    void getCart(const QString& username);
    void removeFromCart(const QString& username, int productId);
    void clearCart(const QString& username);
    void checkout(const QString& username);

    // User products
    void getMyProducts(const QString& username);

    // Wallet operations
    void getWallet(const QString& username);
    void deposit(const QString& username, double amount);

    // Profile update
    void updateProfile(const QString& username, const QString& email,
                       const QString& phone, const QString& address);

signals:
    void connected();
    void disconnected();
    void error(const QString& message);

    // Response signals
    void loginResult(bool success, User* user, const QString& error);
    void signupResult(bool success, const QString& error);
    void approvedProductsReceived(const QVector<Product*>& products);
    void pendingProductsReceived(const QVector<Product*>& products);
    void productDetailsReceived(Product* product);
    void addProductResult(bool success, const QString& error);
    void approveResult(bool success, const QString& error);
    void rejectResult(bool success, const QString& error);
    void cartReceived(const QMap<int, int>& cart, double total);
    void checkoutResult(bool success, double total, const QString& error);
    void myProductsReceived(const QVector<Product*>& products);
    void walletReceived(double balance);
    void depositResult(bool success, double newBalance, const QString& error);
    void profileUpdateResult(bool success, const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager();

    static NetworkManager* m_instance;
    QTcpSocket* m_socket;
    QString m_buffer;
};

#endif 