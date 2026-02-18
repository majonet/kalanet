#include <QCoreApplication>
#include <QDebug>
#include "DataManager.h"
#include "User.h"
#include "Product.h"

// Simple test to verify data persistence
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "=== KalaNet Data Persistence Test ===";

    // Initialize DataManager
    DataManager* dm = DataManager::getInstance();

    qDebug() << "Current users:" << dm->getTotalUsers();
    qDebug() << "Current products:" << dm->getTotalProducts();

    // Test: Create a test user if it doesn't exist
    if (!dm->userExists("testuser")) {
        qDebug() << "Creating test user...";
        Customer* testUser = new Customer(
            "testuser",
            User::hashPassword("Test1234"),
            "test@test.com",
            "1234567890",
            "Test Address"
        );
        testUser->addFunds(1000);

        if (dm->addUser(testUser)) {
            qDebug() << "Test user created successfully!";
        } else {
            qDebug() << "Failed to create test user!";
            delete testUser;
        }
    } else {
        qDebug() << "Test user already exists";
        User* user = dm->getUser("testuser");
        if (user) {
            qDebug() << "Test user wallet:" << user->getWalletBalance();
        }
    }

    // Test: Create a test product if none exist
    if (dm->getTotalProducts() == 0) {
        qDebug() << "Creating test product...";
        Product* product = new Product(
            dm->getNextProductId(),
            "Test Product",
            "This is a test product",
            "Electronics",
            99.99,
            10,
            "admin"
        );
        product->setStatus(ProductStatus::APPROVED);

        if (dm->addProduct(product)) {
            qDebug() << "Test product created successfully!";
        } else {
            qDebug() << "Failed to create test product!";
            delete product;
        }
    }

    // Save all data
    qDebug() << "Saving all data...";
    if (dm->saveAllData()) {
        qDebug() << "Data saved successfully!";
    } else {
        qDebug() << "Failed to save data!";
    }

    qDebug() << "";
    qDebug() << "=== Test Complete ===";
    qDebug() << "Data directory:" << QCoreApplication::applicationDirPath() + "/data";
    qDebug() << "Check if users.dat and products.dat were created.";

    DataManager::destroyInstance();
    return 0;
}
