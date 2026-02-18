#include "Server.h"
#include <QDebug>

Server::Server(QObject* parent) : QTcpServer(parent) {
    m_dataManager = DataManager::getInstance();
}

bool Server::start(quint16 port) {
    return listen(QHostAddress::Any, port);
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    ClientHandler* handler = new ClientHandler(socket, m_dataManager, this);
    m_clients.append(handler);
    connect(socket, &QTcpSocket::disconnected, [this, handler]() {
        m_clients.removeOne(handler);
        handler->deleteLater();
    });
}

// ClientHandler implementation
ClientHandler::ClientHandler(QTcpSocket* socket, DataManager* dm, QObject* parent)
    : QObject(parent), m_socket(socket), m_dataManager(dm), m_currentUser(nullptr) {
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);
}

void ClientHandler::onReadyRead() {
    m_buffer += m_socket->readAll();
    while (m_buffer.contains('\n')) {
        int pos = m_buffer.indexOf('\n');
        QString line = m_buffer.left(pos).trimmed();
        m_buffer = m_buffer.mid(pos + 1);
        processCommand(line);
    }
}

void ClientHandler::processCommand(const QString& cmd) {
    QStringList parts = cmd.split(' ');
    if (parts.isEmpty()) return;

    QString command = parts[0].toUpper();

    if (command == "LOGIN" && parts.size() >= 3) {
        QString username = parts[1];
        QString password = parts[2];
        if (m_dataManager->validateLogin(username, password)) {
            m_currentUser = m_dataManager->getUser(username);
            QString userType = (m_currentUser->getUserType() == UserType::ADMIN) ? "Admin" : "Customer";
            QString response = QString("OK LOGIN %1|%2|%3\n")
                    .arg(username)
                    .arg(m_currentUser->getWalletBalance())
                    .arg(userType);
            sendResponse(response);
        } else {
            sendError("Invalid username or password");
        }
    }
    else if (command == "SIGNUP" && parts.size() >= 7) {
        QString username = parts[1];
        QString password = parts[2];
        QString email = parts[3];
        QString phone = parts[4];
        QString address = parts[5];
        QString typeStr = parts[6];
        UserType type = (typeStr == "admin") ? UserType::ADMIN : UserType::CUSTOMER;

        if (m_dataManager->userExists(username)) {
            sendError("Username already exists");
            return;
        }

        QString hashed = User::hashPassword(password);
        User* user = nullptr;
        if (type == UserType::ADMIN)
            user = new Admin(username, hashed, email, phone, address);
        else
            user = new Customer(username, hashed, email, phone, address);

        if (m_dataManager->addUser(user)) {
            sendResponse("OK SIGNUP\n");
        } else {
            sendError("Failed to create account");
            delete user;
        }
    }
    else if (command == "GET_APPROVED_PRODUCTS") {
        QVector<Product*> products = m_dataManager->getApprovedProducts();
        QString response = "OK APPROVED_PRODUCTS\n";
        for (Product* p : products) {
            response += QString("%1|%2|%3|%4|%5|%6|%7\n")
                    .arg(p->getProductId())
                    .arg(p->getName())
                    .arg(p->getCategory())
                    .arg(p->getPrice())
                    .arg(p->getStock())
                    .arg(p->getSellerUsername())
                    .arg(p->getStatusString());
        }
        sendResponse(response);
    }
    else if (command == "GET_PENDING_PRODUCTS") {
        QVector<Product*> products = m_dataManager->getPendingProducts();
        QString response = "OK PENDING_PRODUCTS\n";
        for (Product* p : products) {
            response += QString("%1|%2|%3|%4|%5|%6|%7\n")
                    .arg(p->getProductId())
                    .arg(p->getName())
                    .arg(p->getCategory())
                    .arg(p->getPrice())
                    .arg(p->getStock())
                    .arg(p->getSellerUsername())
                    .arg(p->getStatusString());
        }
        sendResponse(response);
    }
    else if (command == "ADD_PRODUCT" && parts.size() >= 2) {
        // Format: ADD_PRODUCT name|desc|category|price|stock|seller
        QString data = parts[1];
        QStringList fields = data.split('|');
        if (fields.size() >= 6) {
            QString name = fields[0];
            QString desc = fields[1];
            QString category = fields[2];
            double price = fields[3].toDouble();
            int stock = fields[4].toInt();
            QString seller = fields[5];

            int id = m_dataManager->getNextProductId();
            Product* p = new Product(id, name, desc, category, price, stock, seller);
            if (m_dataManager->addProduct(p)) {
                sendResponse("OK ADD_PRODUCT\n");
            } else {
                sendError("Failed to add product");
                delete p;
            }
        } else {
            sendError("Invalid product data");
        }
    }
    else if (command == "APPROVE" && parts.size() >= 2) {
        int id = parts[1].toInt();
        if (m_dataManager->approveProduct(id))
            sendResponse("OK APPROVE\n");
        else
            sendError("Approval failed");
    }
    else if (command == "REJECT" && parts.size() >= 2) {
        int id = parts[1].toInt();
        if (m_dataManager->rejectProduct(id))
            sendResponse("OK REJECT\n");
        else
            sendError("Rejection failed");
    }
    else if (command == "ADD_TO_CART" && parts.size() >= 4) {
        QString username = parts[1];
        int productId = parts[2].toInt();
        int quantity = parts[3].toInt();
        User* user = m_dataManager->getUser(username);
        Customer* cust = dynamic_cast<Customer*>(user);
        if (cust) {
            cust->addToCart(productId, quantity);
            m_dataManager->saveUsers();
            sendResponse("OK ADD_TO_CART\n");
        } else {
            sendError("User not found or not a customer");
        }
    }
    else if (command == "GET_CART" && parts.size() >= 2) {
        QString username = parts[1];
        User* user = m_dataManager->getUser(username);
        Customer* cust = dynamic_cast<Customer*>(user);
        if (cust) {
            const QMap<int, int>& cart = cust->getCart();
            double total = 0;
            QString response = "OK CART\n";
            for (auto it = cart.begin(); it != cart.end(); ++it) {
                Product* p = m_dataManager->getProduct(it.key());
                if (p) {
                    response += QString("%1|%2|%3|%4\n")
                            .arg(it.key())
                            .arg(p->getName())
                            .arg(p->getPrice())
                            .arg(it.value());
                    total += p->getPrice() * it.value();
                }
            }
            response += QString("TOTAL|%1\n").arg(total);
            sendResponse(response);
        } else {
            sendError("User not found or not a customer");
        }
    }
    else if (command == "REMOVE_FROM_CART" && parts.size() >= 3) {
        QString username = parts[1];
        int productId = parts[2].toInt();
        Customer* cust = dynamic_cast<Customer*>(m_dataManager->getUser(username));
        if (cust) {
            cust->removeFromCart(productId);
            m_dataManager->saveUsers();
            sendResponse("OK REMOVE_FROM_CART\n");
        } else {
            sendError("User not found or not a customer");
        }
    }
    else if (command == "CLEAR_CART" && parts.size() >= 2) {
        QString username = parts[1];
        Customer* cust = dynamic_cast<Customer*>(m_dataManager->getUser(username));
        if (cust) {
            cust->clearCart();
            m_dataManager->saveUsers();
            sendResponse("OK CLEAR_CART\n");
        } else {
            sendError("User not found or not a customer");
        }
    }
    else if (command == "CHECKOUT" && parts.size() >= 2) {
        QString username = parts[1];
        Customer* cust = dynamic_cast<Customer*>(m_dataManager->getUser(username));
        if (!cust) {
            sendError("User not found or not a customer");
            return;
        }
        if (cust->getCart().isEmpty()) {
            sendError("Cart is empty");
            return;
        }

        double total = 0;
        QMap<int, int> cart = cust->getCart();
        for (auto it = cart.begin(); it != cart.end(); ++it) {
            Product* p = m_dataManager->getProduct(it.key());
            if (p) total += p->getPrice() * it.value();
        }

        if (cust->getWalletBalance() < total) {
            sendError("Insufficient funds");
            return;
        }

        // Process each item
        for (auto it = cart.begin(); it != cart.end(); ++it) {
            Product* p = m_dataManager->getProduct(it.key());
            if (!p) continue;
            int qty = it.value();
            double itemTotal = p->getPrice() * qty;

            cust->deductFunds(itemTotal);
            User* seller = m_dataManager->getUser(p->getSellerUsername());
            if (seller) seller->addFunds(itemTotal);
            p->purchase(qty);

            Transaction trans;
            trans.productId = p->getProductId();
            trans.productName = p->getName();
            trans.sellerUsername = p->getSellerUsername();
            trans.buyerUsername = cust->getUsername();
            trans.quantity = qty;
            trans.totalPrice = itemTotal;
            trans.date = QDateTime::currentDateTime();
            cust->addTransaction(trans);
            if (Customer* sellerCust = dynamic_cast<Customer*>(seller))
                sellerCust->addTransaction(trans);
        }

        cust->clearCart();
        m_dataManager->saveAllData();

        sendResponse(QString("OK CHECKOUT %1\n").arg(total));
    }
    else if (command == "GET_MY_PRODUCTS" && parts.size() >= 2) {
        QString username = parts[1];
        User* user = m_dataManager->getUser(username);
        if (!user) {
            sendError("User not found");
            return;
        }
        QVector<Product*> myProducts;
        for (Product* p : m_dataManager->getAllProducts()) {
            if (p->getSellerUsername() == username)
                myProducts.append(p);
        }
        QString response = "OK MY_PRODUCTS\n";
        for (Product* p : myProducts) {
            response += QString("%1|%2|%3|%4|%5|%6|%7\n")
                    .arg(p->getProductId())
                    .arg(p->getName())
                    .arg(p->getCategory())
                    .arg(p->getPrice())
                    .arg(p->getStock())
                    .arg(p->getStatusString())
                    .arg(p->getSellerUsername());
        }
        sendResponse(response);
    }
    else if (command == "GET_WALLET" && parts.size() >= 2) {
        QString username = parts[1];
        User* user = m_dataManager->getUser(username);
        if (user) {
            sendResponse(QString("OK WALLET %1\n").arg(user->getWalletBalance()));
        } else {
            sendError("User not found");
        }
    }
    else if (command == "DEPOSIT" && parts.size() >= 3) {
        QString username = parts[1];
        double amount = parts[2].toDouble();
        User* user = m_dataManager->getUser(username);
        if (user) {
            user->addFunds(amount);
            m_dataManager->saveUsers();
            sendResponse(QString("OK DEPOSIT %1\n").arg(user->getWalletBalance()));
        } else {
            sendError("User not found");
        }
    }
    else {
        sendError("Unknown command");
    }
}

void ClientHandler::sendResponse(const QString& response) {
    m_socket->write(response.toUtf8());
}

void ClientHandler::sendError(const QString& msg) {
    sendResponse("ERROR " + msg + "\n");
}

void ClientHandler::onDisconnected() {
    m_socket->deleteLater();
}