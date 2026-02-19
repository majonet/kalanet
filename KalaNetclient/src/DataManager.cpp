#include "DataManager.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QTextStream>

DataManager* DataManager::instance = nullptr;
QMutex DataManager::instanceMutex;

DataManager::DataManager(QObject* parent)
    : QObject(parent), nextProductId(1) {

    dataDir = QDir::currentPath() + "/data";
    usersFile = dataDir + "/users.csv";
    productsFile = dataDir + "/products.csv";

    QDir dir;
    if (!dir.exists(dataDir)) {
        if (!dir.mkpath(dataDir)) {
            qDebug() << "Failed to create data directory:" << dataDir;
        }
    }

    loadAllData();
}

DataManager* DataManager::getInstance() {
    QMutexLocker locker(&instanceMutex);
    if (instance == nullptr) {
        instance = new DataManager();
    }
    return instance;
}

void DataManager::destroyInstance() {
    QMutexLocker locker(&instanceMutex);
    if (instance != nullptr) {
        instance->saveAllData();
        delete instance;
        instance = nullptr;
    }
}

QString DataManager::escapeCSV(const QString& str) {
    QString result = str;
    result.replace("\"", "\"\"");
    if (result.contains(",") || result.contains("\"") || result.contains("\n") || result.contains("\r")) {
        result = "\"" + result + "\"";
    }
    return result;
}

QString DataManager::unescapeCSV(const QString& str) {
    QString result = str;
    if (result.startsWith("\"") && result.endsWith("\"")) {
        result = result.mid(1, result.length() - 2);
        result.replace("\"\"", "\"");
    }
    return result;
}

// User Management
bool DataManager::addUser(User* user) {
    QMutexLocker locker(&dataMutex);
    if (users.contains(user->getUsername())) {
        return false;
    }
    users[user->getUsername()] = user;
    saveUsersToCSV();
    emit dataChanged();
    return true;
}

User* DataManager::getUser(const QString& username) {
    QMutexLocker locker(&dataMutex);
    return users.value(username, nullptr);
}

bool DataManager::userExists(const QString& username) const {
    QMutexLocker locker(&dataMutex);
    return users.contains(username);
}

bool DataManager::validateLogin(const QString& username, const QString& password) const {
    QMutexLocker locker(&dataMutex);
    if (!users.contains(username)) {
        return false;
    }
    QString hashedPass = User::hashPassword(password);
    return users[username]->getHashedPassword() == hashedPass;
}

QVector<User*> DataManager::getAllUsers() const {
    QMutexLocker locker(&dataMutex);
    QVector<User*> result;
    for (auto it = users.begin(); it != users.end(); ++it) {
        result.append(it.value());
    }
    return result;
}

// Product Management
bool DataManager::addProduct(Product* product) {
    QMutexLocker locker(&dataMutex);
    if (products.contains(product->getProductId())) {
        return false;
    }
    products[product->getProductId()] = product;
    if (product->getProductId() >= nextProductId) {
        nextProductId = product->getProductId() + 1;
    }
    saveProductsToCSV();
    emit dataChanged();
    return true;
}

Product* DataManager::getProduct(int productId) {
    QMutexLocker locker(&dataMutex);
    return products.value(productId, nullptr);
}

bool DataManager::removeProduct(int productId) {
    QMutexLocker locker(&dataMutex);
    if (!products.contains(productId)) {
        return false;
    }
    Product* product = products[productId];
    products.remove(productId);
    delete product;
    saveProductsToCSV();
    emit dataChanged();
    return true;
}

QVector<Product*> DataManager::getAllProducts() const {
    QMutexLocker locker(&dataMutex);
    QVector<Product*> result;
    for (auto it = products.begin(); it != products.end(); ++it) {
        result.append(it.value());
    }
    return result;
}

QVector<Product*> DataManager::getApprovedProducts() const {
    QMutexLocker locker(&dataMutex);
    QVector<Product*> result;
    for (auto it = products.begin(); it != products.end(); ++it) {
        if (it.value()->isApproved()) {
            result.append(it.value());
        }
    }
    return result;
}

QVector<Product*> DataManager::getPendingProducts() const {
    QMutexLocker locker(&dataMutex);
    QVector<Product*> result;
    for (auto it = products.begin(); it != products.end(); ++it) {
        if (it.value()->isPending()) {
            result.append(it.value());
        }
    }
    return result;
}

QVector<Product*> DataManager::getProductsByCategory(const QString& category) const {
    QMutexLocker locker(&dataMutex);
    QVector<Product*> result;
    for (auto it = products.begin(); it != products.end(); ++it) {
        if (it.value()->getCategory() == category && it.value()->isApproved()) {
            result.append(it.value());
        }
    }
    return result;
}

QVector<Product*> DataManager::searchProducts(const QString& searchTerm) const {
    QMutexLocker locker(&dataMutex);
    QVector<Product*> result;
    QString lowerTerm = searchTerm.toLower();
    for (auto it = products.begin(); it != products.end(); ++it) {
        Product* p = it.value();
        if (p->isApproved() &&
            (p->getName().toLower().contains(lowerTerm) ||
             p->getDescription().toLower().contains(lowerTerm) ||
             p->getCategory().toLower().contains(lowerTerm))) {
            result.append(p);
        }
    }
    return result;
}

int DataManager::getPendingCount() const {
    QMutexLocker locker(&dataMutex);
    int count = 0;
    for (auto it = products.begin(); it != products.end(); ++it) {
        if (it.value()->isPending()) count++;
    }
    return count;
}

int DataManager::getNextProductId() {
    QMutexLocker locker(&dataMutex);
    return nextProductId++;
}

// Approval system – direct map access (avoids calling getProduct)
bool DataManager::approveProduct(int productId) {
    QMutexLocker locker(&dataMutex);
    if (!products.contains(productId)) {
        return false;
    }
    Product* product = products[productId];
    if (!product || !product->isPending()) {
        return false;
    }
    product->setStatus(ProductStatus::APPROVED);
    saveProductsToCSV();
    emit productApproved(productId);
    emit dataChanged();
    return true;
}

bool DataManager::rejectProduct(int productId) {
    QMutexLocker locker(&dataMutex);
    if (!products.contains(productId)) {
        return false;
    }
    Product* product = products[productId];
    if (!product || !product->isPending()) {
        return false;
    }
    // Remove from map before deleting
    products.remove(productId);
    delete product;
    saveProductsToCSV();
    emit productRejected(productId);
    emit dataChanged();
    return true;
}

// CSV Save/Load Implementation
bool DataManager::saveAllData() {
    bool success = true;
    success &= saveUsersToCSV();
    success &= saveProductsToCSV();
    success &= saveTransactionsToCSV();
    success &= saveCartToCSV();
    return success;
}

bool DataManager::loadAllData() {
    bool success = true;
    success &= loadUsersFromCSV();
    success &= loadProductsFromCSV();
    success &= loadTransactionsFromCSV();
    success &= loadCartFromCSV();
    return success;
}

bool DataManager::saveUsersToCSV() {
    QFile file(usersFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open users file for writing:" << usersFile;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    // Updated header to include registered_products
    stream << "username,password_hash,email,phone,address,wallet_balance,user_type,registered_products\n";

    for (auto it = users.begin(); it != users.end(); ++it) {
        User* user = it.value();

        // Build registered products string for customers (semicolon-separated)
        QString registeredProductsStr;
        Customer* customer = dynamic_cast<Customer*>(user);
        if (customer) {
            QVector<int> regIds = customer->getRegisteredProductIds();
            QStringList idList;
            for (int id : regIds) {
                idList << QString::number(id);
            }
            registeredProductsStr = idList.join(";");
        }

        stream << escapeCSV(user->getUsername()) << ","
               << escapeCSV(user->getHashedPassword()) << ","
               << escapeCSV(user->getEmail()) << ","
               << escapeCSV(user->getPhone()) << ","
               << escapeCSV(user->getAddress()) << ","
               << user->getWalletBalance() << ","
               << (user->getUserType() == UserType::ADMIN ? "admin" : "customer") << ","
               << escapeCSV(registeredProductsStr)
               << "\n";
    }
    file.close();
    return true;
}

bool DataManager::loadUsersFromCSV() {
    QFile file(usersFile);
    if (!file.exists()) {
        Admin* admin = new Admin("admin", User::hashPassword("Admin123"),
                                 "admin@kalanet.com", "09123456789", "IUT");
        admin->addFunds(10000);
        users["admin"] = admin;
        return saveUsersToCSV();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open users file for reading:" << usersFile;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // Clear existing users
    for (auto it = users.begin(); it != users.end(); ++it) {
        delete it.value();
    }
    users.clear();

    QString header = stream.readLine(); // skip header

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() >= 7) {
            QString username = unescapeCSV(parts[0]);
            QString password = unescapeCSV(parts[1]);
            QString email = unescapeCSV(parts[2]);
            QString phone = unescapeCSV(parts[3]);
            QString address = unescapeCSV(parts[4]);
            double wallet = parts[5].toDouble();
            QString type = parts[6];

            User* user = nullptr;
            if (type == "admin") {
                user = new Admin(username, password, email, phone, address);
            } else {
                user = new Customer(username, password, email, phone, address);
            }
            user->setWalletBalance(wallet);

            // Load registered products for customers
            if (parts.size() >= 8 && type != "admin") {
                Customer* customer = dynamic_cast<Customer*>(user);
                if (customer) {
                    QString regStr = unescapeCSV(parts[7]);
                    if (!regStr.isEmpty()) {
                        QStringList ids = regStr.split(";");
                        for (const QString& sid : ids) {
                            bool ok;
                            int pid = sid.toInt(&ok);
                            if (ok && pid > 0) {
                                customer->addRegisteredProduct(pid);
                            }
                        }
                    }
                }
            }

            users[username] = user;
        }
    }
    file.close();

    if (users.isEmpty()) {
        Admin* admin = new Admin("admin", User::hashPassword("Admin123"),
                                 "admin@kalanet.com", "09123456789", "IUT");
        admin->addFunds(10000);
        users["admin"] = admin;
        saveUsersToCSV();
    }
    return true;
}

// bool DataManager::saveProductsToCSV() {
//     QFile file(productsFile);
//     if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         qDebug() << "Failed to open products file for writing:" << productsFile;
//         return false;
//     }

//     QTextStream stream(&file);
//     stream.setEncoding(QStringConverter::Utf8);
//     stream << "product_id,name,description,category,price,stock,seller,status,next_id\n";

//     for (auto it = products.begin(); it != products.end(); ++it) {
//         Product* p = it.value();
//         QString statusStr;
//         switch(p->getStatus()) {
//         case ProductStatus::PENDING_APPROVAL: statusStr = "pending"; break;
//         case ProductStatus::APPROVED: statusStr = "approved"; break;
//         case ProductStatus::SOLD: statusStr = "sold"; break;
//         }
//         stream << p->getProductId() << ","
//                << escapeCSV(p->getName()) << ","
//                << escapeCSV(p->getDescription()) << ","
//                << escapeCSV(p->getCategory()) << ","
//                << p->getPrice() << ","
//                << p->getStock() << ","
//                << escapeCSV(p->getSellerUsername()) << ","
//                << statusStr << ","
//                << nextProductId
//                << "\n";
//     }
//     file.close();
//     return true;
// }
bool DataManager::saveProductsToCSV() {
    QFile file(productsFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open products file for writing:" << productsFile;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    // Updated header to include image_base64
    stream << "product_id,name,description,category,price,stock,seller,status,next_id,image_base64\n";

    for (auto it = products.begin(); it != products.end(); ++it) {
        Product* p = it.value();
        QString statusStr;
        switch(p->getStatus()) {
        case ProductStatus::PENDING_APPROVAL: statusStr = "pending"; break;
        case ProductStatus::APPROVED: statusStr = "approved"; break;
        case ProductStatus::SOLD: statusStr = "sold"; break;
        }
        stream << p->getProductId() << ","
               << escapeCSV(p->getName()) << ","
               << escapeCSV(p->getDescription()) << ","
               << escapeCSV(p->getCategory()) << ","
               << p->getPrice() << ","
               << p->getStock() << ","
               << escapeCSV(p->getSellerUsername()) << ","
               << statusStr << ","
               << nextProductId << ","
               << escapeCSV(p->getImageBase64())  // Save image as base64
               << "\n";
    }
    file.close();
    return true;
}
// bool DataManager::loadProductsFromCSV() {
//     QFile file(productsFile);
//     if (!file.exists()) return true;

//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         qDebug() << "Failed to open products file for reading:" << productsFile;
//         return false;
//     }

//     QTextStream stream(&file);
//     stream.setEncoding(QStringConverter::Utf8);

//     // Clear existing products
//     for (auto it = products.begin(); it != products.end(); ++it) {
//         delete it.value();
//     }
//     products.clear();

//     QString header = stream.readLine(); // skip header

//     while (!stream.atEnd()) {
//         QString line = stream.readLine();
//         if (line.trimmed().isEmpty()) continue;

//         QStringList parts = line.split(",");
//         if (parts.size() >= 8) {
//             int id = parts[0].toInt();
//             QString name = unescapeCSV(parts[1]);
//             QString desc = unescapeCSV(parts[2]);
//             QString category = unescapeCSV(parts[3]);
//             double price = parts[4].toDouble();
//             int stock = parts[5].toInt();
//             QString seller = unescapeCSV(parts[6]);
//             QString statusStr = parts[7];

//             if (parts.size() >= 9) {
//                 nextProductId = parts[8].toInt();
//             }

//             ProductStatus status = ProductStatus::PENDING_APPROVAL;
//             if (statusStr == "approved") status = ProductStatus::APPROVED;
//             else if (statusStr == "sold") status = ProductStatus::SOLD;

//             Product* product = new Product(id, name, desc, category, price, stock, seller);
//             product->setStatus(status);
//             products[id] = product;
//         }
//     }
//     file.close();
//     return true;
// }
bool DataManager::loadProductsFromCSV() {
    QFile file(productsFile);
    if (!file.exists()) return true;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open products file for reading:" << productsFile;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // Clear existing products
    for (auto it = products.begin(); it != products.end(); ++it) {
        delete it.value();
    }
    products.clear();

    QString header = stream.readLine(); // skip header

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() >= 8) {
            int id = parts[0].toInt();
            QString name = unescapeCSV(parts[1]);
            QString desc = unescapeCSV(parts[2]);
            QString category = unescapeCSV(parts[3]);
            double price = parts[4].toDouble();
            int stock = parts[5].toInt();
            QString seller = unescapeCSV(parts[6]);
            QString statusStr = parts[7];

            if (parts.size() >= 9) {
                nextProductId = parts[8].toInt();
            }

            ProductStatus status = ProductStatus::PENDING_APPROVAL;
            if (statusStr == "approved") status = ProductStatus::APPROVED;
            else if (statusStr == "sold") status = ProductStatus::SOLD;

            Product* product = new Product(id, name, desc, category, price, stock, seller);
            product->setStatus(status);

            // Load image if present (field 9)
            if (parts.size() >= 10) {
                product->setImageBase64(unescapeCSV(parts[9]));
            }

            products[id] = product;
        }
    }
    file.close();
    return true;
}

bool DataManager::saveTransactionsToCSV() {
    QString filename = dataDir + "/transactions.csv";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "username,product_id,product_name,seller,buyer,quantity,total_price,date\n";

    for (auto it = users.begin(); it != users.end(); ++it) {
        Customer* customer = dynamic_cast<Customer*>(it.value());
        if (customer) {
            for (const Transaction& trans : customer->getPurchaseHistory()) {
                stream << escapeCSV(customer->getUsername()) << ","
                       << trans.productId << ","
                       << escapeCSV(trans.productName) << ","
                       << escapeCSV(trans.sellerUsername) << ","
                       << escapeCSV(trans.buyerUsername) << ","
                       << trans.quantity << ","
                       << trans.totalPrice << ","
                       << trans.date.toString("yyyy-MM-dd hh:mm:ss")
                       << "\n";
            }
        }
    }
    file.close();
    return true;
}

bool DataManager::loadTransactionsFromCSV() {
    QString filename = dataDir + "/transactions.csv";
    QFile file(filename);
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    if (!stream.atEnd()) stream.readLine(); // skip header

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() >= 8) {
            QString username = unescapeCSV(parts[0]);
            Customer* customer = dynamic_cast<Customer*>(getUser(username));
            if (customer) {
                Transaction trans;
                trans.productId = parts[1].toInt();
                trans.productName = unescapeCSV(parts[2]);
                trans.sellerUsername = unescapeCSV(parts[3]);
                trans.buyerUsername = unescapeCSV(parts[4]);
                trans.quantity = parts[5].toInt();
                trans.totalPrice = parts[6].toDouble();
                trans.date = QDateTime::fromString(parts[7], "yyyy-MM-dd hh:mm:ss");
                customer->addTransaction(trans);
            }
        }
    }
    file.close();
    return true;
}

bool DataManager::saveCartToCSV() {
    QString filename = dataDir + "/carts.csv";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "username,product_id,quantity\n";

    for (auto it = users.begin(); it != users.end(); ++it) {
        Customer* customer = dynamic_cast<Customer*>(it.value());
        if (customer) {
            QMap<int, int> cart = customer->getCart();
            for (auto cartIt = cart.begin(); cartIt != cart.end(); ++cartIt) {
                stream << escapeCSV(customer->getUsername()) << ","
                       << cartIt.key() << ","
                       << cartIt.value()
                       << "\n";
            }
        }
    }
    file.close();
    return true;
}
bool DataManager::userHasProductWithName(const QString& username, const QString& productName, int excludeProductId) const {
    QMutexLocker locker(&dataMutex);

    QString lowerName = productName.toLower().trimmed();

    for (auto it = products.begin(); it != products.end(); ++it) {
        Product* p = it.value();
        // Check if same seller, same name (case-insensitive), and not the product we're editing
        if (p->getSellerUsername() == username &&
            p->getName().toLower().trimmed() == lowerName &&
            p->getProductId() != excludeProductId) {
            return true;  // Found duplicate
        }
    }
    return false;  // No duplicate found
}
bool DataManager::loadCartFromCSV() {
    QString filename = dataDir + "/carts.csv";
    QFile file(filename);
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    if (!stream.atEnd()) stream.readLine(); // skip header

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() >= 3) {
            QString username = unescapeCSV(parts[0]);
            int productId = parts[1].toInt();
            int quantity = parts[2].toInt();

            Customer* customer = dynamic_cast<Customer*>(getUser(username));
            if (customer) {
                customer->addToCart(productId, quantity);
            }
        }
    }
    file.close();
    return true;
}
