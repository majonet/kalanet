#include "NetworkManager.h"
#include <QDataStream>
#include <QDebug>

NetworkManager* NetworkManager::m_instance = nullptr;

NetworkManager* NetworkManager::instance() {
    if (!m_instance)
        m_instance = new NetworkManager();
    return m_instance;
}

void NetworkManager::destroy() {
    delete m_instance;
    m_instance = nullptr;
}

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkManager::onError);
}

NetworkManager::~NetworkManager() {
    disconnectFromServer();
}

bool NetworkManager::connectToServer(const QString& host, quint16 port) {
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        return true;

    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(3000);
}

void NetworkManager::disconnectFromServer() {
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        m_socket->disconnectFromHost();
}

void NetworkManager::login(const QString& username, const QString& password) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    QString cmd = QString("LOGIN %1 %2\n").arg(username, password);
    m_socket->write(cmd.toUtf8());
}

void NetworkManager::signup(const QString& username, const QString& password,
                            const QString& email, const QString& phone,
                            const QString& address, UserType type)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    QString typeStr = (type == UserType::ADMIN) ? "admin" : "customer";
    QString cmd = QString("SIGNUP %1 %2 %3 %4 %5 %6\n")
                      .arg(username, password, email, phone, address, typeStr);
    m_socket->write(cmd.toUtf8());
}

void NetworkManager::getApprovedProducts() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write("GET_APPROVED_PRODUCTS\n");
}

void NetworkManager::getPendingProducts() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write("GET_PENDING_PRODUCTS\n");
}

void NetworkManager::getProductDetails(int productId) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("GET_PRODUCT %1\n").arg(productId).toUtf8());
}

void NetworkManager::addProduct(const QString& name, const QString& description,
                                const QString& category, double price, int stock,
                                const QString& seller)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    QString cmd = QString("ADD_PRODUCT %1|%2|%3|%4|%5|%6\n")
                      .arg(name, description, category)
                      .arg(price).arg(stock).arg(seller);
    m_socket->write(cmd.toUtf8());
}

void NetworkManager::approveProduct(int productId) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("APPROVE %1\n").arg(productId).toUtf8());
}

void NetworkManager::rejectProduct(int productId) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("REJECT %1\n").arg(productId).toUtf8());
}

void NetworkManager::addToCart(const QString& username, int productId, int quantity) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("ADD_TO_CART %1 %2 %3\n")
                        .arg(username).arg(productId).arg(quantity).toUtf8());
}

void NetworkManager::getCart(const QString& username) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("GET_CART %1\n").arg(username).toUtf8());
}

void NetworkManager::removeFromCart(const QString& username, int productId) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("REMOVE_FROM_CART %1 %2\n")
                        .arg(username).arg(productId).toUtf8());
}

void NetworkManager::clearCart(const QString& username) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("CLEAR_CART %1\n").arg(username).toUtf8());
}

void NetworkManager::checkout(const QString& username) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("CHECKOUT %1\n").arg(username).toUtf8());
}

void NetworkManager::getMyProducts(const QString& username) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("GET_MY_PRODUCTS %1\n").arg(username).toUtf8());
}

void NetworkManager::getWallet(const QString& username) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("GET_WALLET %1\n").arg(username).toUtf8());
}

void NetworkManager::deposit(const QString& username, double amount) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    m_socket->write(QString("DEPOSIT %1 %2\n").arg(username).arg(amount).toUtf8());
}

void NetworkManager::updateProfile(const QString& username, const QString& email,
                                   const QString& phone, const QString& address)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    QString cmd = QString("UPDATE_PROFILE %1|%2|%3|%4\n")
                      .arg(username, email, phone, address);
    m_socket->write(cmd.toUtf8());
}

void NetworkManager::onConnected() {
    emit connected();
}

void NetworkManager::onDisconnected() {
    emit disconnected();
}

void NetworkManager::onError(QAbstractSocket::SocketError) {
    emit error(m_socket->errorString());
}

void NetworkManager::onReadyRead() {
    m_buffer += m_socket->readAll();

    while (m_buffer.contains('\n')) {
        int pos = m_buffer.indexOf('\n');
        QString line = m_buffer.left(pos).trimmed();
        m_buffer = m_buffer.mid(pos + 1);

        if (line.isEmpty())
            continue;

        if (line.startsWith("OK ")) {
            QString data = line.mid(3);

            if (data.startsWith("LOGIN ")) {
                QStringList parts = data.mid(6).split('|');
                if (parts.size() >= 3) {
                    QString username = parts[0];
                    double wallet = parts[1].toDouble();
                    QString type = parts[2];
                    User* user = nullptr;
                    if (type == "Admin")
                        user = new Admin(username, "", "", "", "");
                    else
                        user = new Customer(username, "", "", "", "");
                    user->setWalletBalance(wallet);
                    emit loginResult(true, user, "");
                } else {
                    emit loginResult(false, nullptr, "Invalid login data");
                }
            }
            else if (data.startsWith("SIGNUP")) {
                emit signupResult(true, "");
            }
            else if (data.startsWith("APPROVED_PRODUCTS")) {
                QVector<Product*> products;
                QStringList lines = data.split('\n');
                // Skip the first line (header) and parse subsequent lines
                for (int i = 1; i < lines.size(); ++i) {
                    QStringList fields = lines[i].split('|');
                    if (fields.size() >= 7) {
                        Product* p = new Product();
                        p->setProductId(fields[0].toInt());
                        p->setName(fields[1]);
                        p->setCategory(fields[2]);
                        p->setPrice(fields[3].toDouble());
                        p->setStock(fields[4].toInt());
                        p->setSellerUsername(fields[5]);
                        p->setStatus(fields[6] == "Approved" ? ProductStatus::APPROVED
                                                             : ProductStatus::PENDING_APPROVAL);
                        products.append(p);
                    }
                }
                emit approvedProductsReceived(products);
            }
            else if (data.startsWith("PENDING_PRODUCTS")) {
                QVector<Product*> products;
                QStringList lines = data.split('\n');
                for (int i = 1; i < lines.size(); ++i) {
                    QStringList fields = lines[i].split('|');
                    if (fields.size() >= 7) {
                        Product* p = new Product();
                        p->setProductId(fields[0].toInt());
                        p->setName(fields[1]);
                        p->setCategory(fields[2]);
                        p->setPrice(fields[3].toDouble());
                        p->setStock(fields[4].toInt());
                        p->setSellerUsername(fields[5]);
                        p->setStatus(ProductStatus::PENDING_APPROVAL);
                        products.append(p);
                    }
                }
                emit pendingProductsReceived(products);
            }
            else if (data.startsWith("ADD_PRODUCT")) {
                emit addProductResult(true, "");
            }
            else if (data.startsWith("APPROVE")) {
                emit approveResult(true, "");
            }
            else if (data.startsWith("CART")) {
                QMap<int, int> cart;
                double total = 0;
                QStringList lines = data.split('\n');
                for (const QString& l : lines) {
                    if (l.startsWith("TOTAL|")) {
                        total = l.mid(6).toDouble();
                    } else {
                        QStringList fields = l.split('|');
                        if (fields.size() >= 4) {
                            cart[fields[0].toInt()] = fields[3].toInt();
                        }
                    }
                }
                emit cartReceived(cart, total);
            }
            else if (data.startsWith("CHECKOUT ")) {
                double total = data.mid(9).toDouble();
                emit checkoutResult(true, total, "");
            }
            else if (data.startsWith("MY_PRODUCTS")) {
                QVector<Product*> products;
                QStringList lines = data.split('\n');
                for (int i = 1; i < lines.size(); ++i) {
                    QStringList fields = lines[i].split('|');
                    if (fields.size() >= 7) {
                        Product* p = new Product();
                        p->setProductId(fields[0].toInt());
                        p->setName(fields[1]);
                        p->setCategory(fields[2]);
                        p->setPrice(fields[3].toDouble());
                        p->setStock(fields[4].toInt());
                        p->setStatus(fields[5] == "Approved" ? ProductStatus::APPROVED :
                                     (fields[5] == "Pending Approval" ? ProductStatus::PENDING_APPROVAL
                                                                      : ProductStatus::SOLD));
                        p->setSellerUsername(fields[6]);
                        products.append(p);
                    }
                }
                emit myProductsReceived(products);
            }
            else if (data.startsWith("WALLET ")) {
                double balance = data.mid(7).toDouble();
                emit walletReceived(balance);
            }
            else if (data.startsWith("DEPOSIT ")) {
                double balance = data.mid(8).toDouble();
                emit depositResult(true, balance, "");
            }
            // Add other response types as needed
        }
        else if (line.startsWith("ERROR ")) {
            QString errorMsg = line.mid(6);
            emit error(errorMsg);
        }
        else {
            qDebug() << "Unhandled response:" << line;
        }
    }
}