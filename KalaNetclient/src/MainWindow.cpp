#include "MainWindow.h"
#include "DataManager.h"
#include "Product.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QCloseEvent>
#include <QInputDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QApplication>
#include <QRegularExpression>

MainWindow::MainWindow(User* user, QWidget* parent)
    : QMainWindow(parent), currentUser(user) {
    isAdmin = (user->getUserType() == UserType::ADMIN);
    setupUI();
    updateProfileInfo();
    refreshProductList();

    if (!isAdmin) {
        refreshCart();
        refreshMyProducts();  // This will now work correctly
        refreshWallet();
    } else {
        refreshAdminProducts();
        refreshPendingProducts();
    }

    setWindowTitle("KalaNet - " + QString(isAdmin ? "Admin Dashboard" : "Shopping"));
    setMinimumSize(1000, 700);
    resize(1200, 800);

    // Style (same as original)
    setStyleSheet(R"(
        QMainWindow { background-color: #f5f5f5; }
        QTabWidget::pane { border: 1px solid #ddd; background-color: white; border-radius: 5px; }
        QTabBar::tab { background-color: #ecf0f1; padding: 12px 24px; margin-right: 2px;
                       border-top-left-radius: 5px; border-top-right-radius: 5px; }
        QTabBar::tab:selected { background-color: #e74c3c; color: white; }
        QTabBar::tab:hover:!selected { background-color: #d5dbdb; }
        QPushButton { padding: 10px 20px; background-color: #3498db; color: white; border: none;
                      border-radius: 5px; font-weight: bold; }
        QPushButton:hover { background-color: #2980b9; }
        QPushButton:pressed { background-color: #1f618d; }
        QPushButton#danger { background-color: #e74c3c; }
        QPushButton#danger:hover { background-color: #c0392b; }
        QPushButton#success { background-color: #27ae60; }
        QPushButton#success:hover { background-color: #1e8449; }
        QTableWidget { border: 1px solid #ddd; gridline-color: #ddd;
                       selection-background-color: #3498db; selection-color: white; }
        QHeaderView::section { background-color: #34495e; color: white; padding: 10px;
                               border: none; font-weight: bold; }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QTextEdit {
            padding: 8px; border: 2px solid #ddd; border-radius: 4px; background-color: white; }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QTextEdit:focus {
            border-color: #3498db; }
        QLabel { color: #2c3e50; }
        QGroupBox { font-weight: bold; border: 2px solid #ddd; border-radius: 5px;
                    margin-top: 10px; padding-top: 10px; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px;
                           color: #e74c3c; }
    )");

    // Connect tab change to refresh admin data
    if (isAdmin) {
        connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
            if (tabWidget->tabText(index) == "Admin") {
                refreshAdminProducts();
                refreshPendingProducts();
            }
        });
        connect(adminTabWidget, &QTabWidget::currentChanged, this, [this](int index) {
            if (adminTabWidget->tabText(index) == "Pending Approvals")
                refreshPendingProducts();
            else if (adminTabWidget->tabText(index) == "All Products")
                refreshAdminProducts();
        });
    }
}

MainWindow::~MainWindow() {
    DataManager::getInstance()->saveAllData();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    DataManager::getInstance()->saveAllData();
    event->accept();
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel("KalaNet");
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #e74c3c;");
    headerLayout->addWidget(titleLabel);

    QLabel* welcomeLabel = new QLabel("Welcome, " + currentUser->getUsername() +
                                      " (" + currentUser->getUserTypeString() + ")");
    welcomeLabel->setStyleSheet("font-size: 14px; color: #666;");
    headerLayout->addStretch();
    headerLayout->addWidget(welcomeLabel);
    mainLayout->addLayout(headerLayout);

    tabWidget = new QTabWidget();
    setupProfileTab();
    setupProductsTab();

    if (!isAdmin) {
        setupCartTab();
        setupWalletTab();
        setupMyProductsTab();
    } else {
        setupAdminTab();
    }

    mainLayout->addWidget(tabWidget);
}

// ---------- Profile Tab ----------
void MainWindow::setupProfileTab() {
    profileTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(profileTab);
    layout->setSpacing(20);

    QGroupBox* infoGroup = new QGroupBox("Profile Information");
    QGridLayout* infoLayout = new QGridLayout(infoGroup);

    usernameLabel = new QLabel();
    userTypeLabel = new QLabel();
    walletLabel = new QLabel();

    infoLayout->addWidget(new QLabel("Username:"), 0, 0);
    infoLayout->addWidget(usernameLabel, 0, 1);
    infoLayout->addWidget(new QLabel("Account Type:"), 1, 0);
    infoLayout->addWidget(userTypeLabel, 1, 1);
    infoLayout->addWidget(new QLabel("Wallet Balance:"), 2, 0);
    infoLayout->addWidget(walletLabel, 2, 1);

    layout->addWidget(infoGroup);

    QGroupBox* editGroup = new QGroupBox("Edit Profile");
    QFormLayout* formLayout = new QFormLayout(editGroup);

    profileEmailEdit = new QLineEdit(currentUser->getEmail());
    profilePhoneEdit = new QLineEdit(currentUser->getPhone());
    profileAddressEdit = new QLineEdit(currentUser->getAddress());

    formLayout->addRow("Email:", profileEmailEdit);
    formLayout->addRow("Phone:", profilePhoneEdit);
    formLayout->addRow("Address:", profileAddressEdit);

    updateProfileButton = new QPushButton("Update Profile");
    connect(updateProfileButton, &QPushButton::clicked, this, &MainWindow::onUpdateProfile);
    formLayout->addRow(updateProfileButton);

    layout->addWidget(editGroup);

    QGroupBox* actionsGroup = new QGroupBox("Actions");
    QVBoxLayout* actionsLayout = new QVBoxLayout(actionsGroup);

    addFundsButton = new QPushButton("Add Funds to Wallet");
    addFundsButton->setObjectName("success");
    connect(addFundsButton, &QPushButton::clicked, this, &MainWindow::onAddFunds);
    actionsLayout->addWidget(addFundsButton);

    logoutButton = new QPushButton("Logout");
    logoutButton->setObjectName("danger");
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::onLogout);
    actionsLayout->addWidget(logoutButton);

    layout->addWidget(actionsGroup);
    // Password Change Group
    QGroupBox* passwordGroup = new QGroupBox("Change Password");
    QFormLayout* passwordLayout = new QFormLayout(passwordGroup);

    currentPasswordEdit = new QLineEdit();
    currentPasswordEdit->setEchoMode(QLineEdit::Password);
    currentPasswordEdit->setPlaceholderText("Enter current password");

    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("Min 8 chars, upper, lower, number");

    confirmNewPasswordEdit = new QLineEdit();
    confirmNewPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmNewPasswordEdit->setPlaceholderText("Confirm new password");

    changePasswordButton = new QPushButton("Change Password");
    changePasswordButton->setObjectName("success");
    connect(changePasswordButton, &QPushButton::clicked, this, &MainWindow::onChangePassword);

    passwordLayout->addRow("Current Password:", currentPasswordEdit);
    passwordLayout->addRow("New Password:", newPasswordEdit);
    passwordLayout->addRow("Confirm New:", confirmNewPasswordEdit);
    passwordLayout->addRow(changePasswordButton);

    layout->addWidget(passwordGroup);
    layout->addStretch();

    tabWidget->addTab(profileTab, "Profile");
}

void MainWindow::updateProfileInfo() {
    usernameLabel->setText(currentUser->getUsername());
    userTypeLabel->setText(currentUser->getUserTypeString());
    walletLabel->setText("$" + QString::number(currentUser->getWalletBalance(), 'f', 2));
}

void MainWindow::onUpdateProfile() {
    QString phone = profilePhoneEdit->text().trimmed();

    // Phone validation: exactly 11 digits
    QRegularExpression phoneRegex("^\\d{11}$");
    if (!phoneRegex.match(phone).hasMatch()) {
        showError("Phone number must be exactly 11 digits");
        return;
    }

    currentUser->setEmail(profileEmailEdit->text().trimmed());
    currentUser->setPhone(phone);
    currentUser->setAddress(profileAddressEdit->text().trimmed());
    DataManager::getInstance()->saveUsers();
    showSuccess("Profile updated successfully!");
}

void MainWindow::onAddFunds() {
    bool ok;
    double amount = QInputDialog::getDouble(this, "Add Funds",
                                            "Enter amount to add:",
                                            100, 1, 10000, 2, &ok);
    if (ok && amount > 0) {
        currentUser->addFunds(amount);
        DataManager::getInstance()->saveUsers();
        updateProfileInfo();
        if (!isAdmin) refreshWallet();
        showSuccess("$" + QString::number(amount, 'f', 2) + " added to your wallet!");
    }
}

// void MainWindow::onLogout() {
//     DataManager::getInstance()->saveAllData();
//     emit logoutRequested();
//     close();
// }
void MainWindow::onLogout() {
    DataManager::getInstance()->saveAllData();
    emit logoutRequested();
    close();
    deleteLater();  // Ensure window is destroyed so event loop exits
}

// ---------- Products Tab ----------
void MainWindow::setupProductsTab() {
    productsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(productsTab);
    layout->setSpacing(15);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search products...");
    searchLayout->addWidget(searchEdit);

    categoryCombo = new QComboBox();
    categoryCombo->addItem("All Categories");
    categoryCombo->addItem("Electronics");
    categoryCombo->addItem("Clothing");
    categoryCombo->addItem("Books");
    categoryCombo->addItem("Home");
    categoryCombo->addItem("Sports");
    categoryCombo->addItem("Other");
    connect(categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCategoryChanged);
    searchLayout->addWidget(categoryCombo);

    searchButton = new QPushButton("Search");
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::onSearchProducts);
    searchLayout->addWidget(searchButton);

    layout->addLayout(searchLayout);

    productsTable = new QTableWidget();
    productsTable->setColumnCount(6);
    productsTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Name" << "Category" << "Price" << "Stock" << "Seller");
    productsTable->horizontalHeader()->setStretchLastSection(true);
    productsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    productsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    productsTable->setAlternatingRowColors(true);
    layout->addWidget(productsTable);

    productDetailsLabel = new QLabel("Select a product to view details");
    productDetailsLabel->setWordWrap(true);
    productDetailsLabel->setStyleSheet("padding: 10px; background-color: #ecf0f1; border-radius: 5px;");
    layout->addWidget(productDetailsLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    if (!isAdmin) {
        addToCartButton = new QPushButton("Add to Cart");
        addToCartButton->setObjectName("success");
        connect(addToCartButton, &QPushButton::clicked, this, &MainWindow::onAddToCart);
        buttonLayout->addWidget(addToCartButton);
    }

    viewDetailsButton = new QPushButton("View Details");
    connect(viewDetailsButton, &QPushButton::clicked, this, &MainWindow::onViewProductDetails);
    buttonLayout->addWidget(viewDetailsButton);

    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    tabWidget->addTab(productsTab, "Products");
}

void MainWindow::refreshProductList() {
    DataManager* dm = DataManager::getInstance();
    QVector<Product*> products = dm->getApprovedProducts();

    productsTable->setRowCount(products.size());

    for (int i = 0; i < products.size(); ++i) {
        Product* p = products[i];
        productsTable->setItem(i, 0, new QTableWidgetItem(QString::number(p->getProductId())));
        productsTable->setItem(i, 1, new QTableWidgetItem(p->getName()));
        productsTable->setItem(i, 2, new QTableWidgetItem(p->getCategory()));
        productsTable->setItem(i, 3, new QTableWidgetItem("$" + QString::number(p->getPrice(), 'f', 2)));
        productsTable->setItem(i, 4, new QTableWidgetItem(QString::number(p->getStock())));
        productsTable->setItem(i, 5, new QTableWidgetItem(p->getSellerUsername()));
    }

    productsTable->resizeColumnsToContents();
}

void MainWindow::onSearchProducts() {
    QString searchTerm = searchEdit->text();
    DataManager* dm = DataManager::getInstance();

    QVector<Product*> products;
    if (searchTerm.isEmpty())
        products = dm->getApprovedProducts();
    else
        products = dm->searchProducts(searchTerm);

    QString category = categoryCombo->currentText();
    if (category != "All Categories") {
        QVector<Product*> filtered;
        for (Product* p : products)
            if (p->getCategory() == category)
                filtered.append(p);
        products = filtered;
    }

    productsTable->setRowCount(products.size());
    for (int i = 0; i < products.size(); ++i) {
        Product* p = products[i];
        productsTable->setItem(i, 0, new QTableWidgetItem(QString::number(p->getProductId())));
        productsTable->setItem(i, 1, new QTableWidgetItem(p->getName()));
        productsTable->setItem(i, 2, new QTableWidgetItem(p->getCategory()));
        productsTable->setItem(i, 3, new QTableWidgetItem("$" + QString::number(p->getPrice(), 'f', 2)));
        productsTable->setItem(i, 4, new QTableWidgetItem(QString::number(p->getStock())));
        productsTable->setItem(i, 5, new QTableWidgetItem(p->getSellerUsername()));
    }
}

void MainWindow::onCategoryChanged(int) {
    onSearchProducts();
}

void MainWindow::onAddToCart() {
    int row = productsTable->currentRow();
    if (row < 0) {
        showError("Please select a product first");
        return;
    }

    int productId = productsTable->item(row, 0)->text().toInt();
    DataManager* dm = DataManager::getInstance();
    Product* product = dm->getProduct(productId);

    if (!product) {
        showError("Product not found");
        return;
    }

    if (product->getStock() <= 0) {
        showError("Product is out of stock");
        return;
    }

    bool ok;
    int quantity = QInputDialog::getInt(this, "Add to Cart",
                                        "Enter quantity:", 1, 1, product->getStock(), 1, &ok);
    if (!ok) return;

    Customer* customer = dynamic_cast<Customer*>(currentUser);
    if (customer) {
        customer->addToCart(productId, quantity);
        dm->saveUsers();
        refreshCart();
        showSuccess("Added " + QString::number(quantity) + " x " + product->getName() + " to cart");
    }
}

void MainWindow::onViewProductDetails() {
    int row = productsTable->currentRow();
    if (row < 0) {
        showError("Please select a product first");
        return;
    }

    int productId = productsTable->item(row, 0)->text().toInt();
    DataManager* dm = DataManager::getInstance();
    Product* product = dm->getProduct(productId);

    if (product) {
        QString details = QString("<h3>%1</h3>"
                                  "<p><b>Description:</b> %2</p>"
                                  "<p><b>Category:</b> %3</p>"
                                  "<p><b>Price:</b> $%4</p>"
                                  "<p><b>Stock:</b> %5</p>"
                                  "<p><b>Seller:</b> %6</p>"
                                  "<p><b>Status:</b> %7</p>")
                              .arg(product->getName())
                              .arg(product->getDescription())
                              .arg(product->getCategory())
                              .arg(product->getPrice(), 0, 'f', 2)
                              .arg(product->getStock())
                              .arg(product->getSellerUsername())
                              .arg(product->getStatusString());

        productDetailsLabel->setText(details);
    }
}

// ---------- Cart Tab ----------
void MainWindow::setupCartTab() {
    cartTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(cartTab);
    layout->setSpacing(15);

    cartTable = new QTableWidget();
    cartTable->setColumnCount(5);
    cartTable->setHorizontalHeaderLabels(
        QStringList() << "Product ID" << "Name" << "Price" << "Quantity" << "Total");
    cartTable->horizontalHeader()->setStretchLastSection(true);
    cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cartTable->setAlternatingRowColors(true);
    layout->addWidget(cartTable);

    cartTotalLabel = new QLabel("Total: $0.00");
    cartTotalLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");
    layout->addWidget(cartTotalLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    removeFromCartButton = new QPushButton("Remove Selected");
    removeFromCartButton->setObjectName("danger");
    connect(removeFromCartButton, &QPushButton::clicked, this, &MainWindow::onRemoveFromCart);
    buttonLayout->addWidget(removeFromCartButton);

    clearCartButton = new QPushButton("Clear Cart");
    connect(clearCartButton, &QPushButton::clicked, this, &MainWindow::onClearCart);
    buttonLayout->addWidget(clearCartButton);

    buttonLayout->addStretch();

    checkoutButton = new QPushButton("Checkout");
    checkoutButton->setObjectName("success");
    checkoutButton->setStyleSheet("padding: 15px 30px; font-size: 16px;");
    connect(checkoutButton, &QPushButton::clicked, this, &MainWindow::onCheckout);
    buttonLayout->addWidget(checkoutButton);

    layout->addLayout(buttonLayout);
    tabWidget->addTab(cartTab, "Shopping Cart");
}

void MainWindow::refreshCart() {
    Customer* customer = dynamic_cast<Customer*>(currentUser);
    if (!customer) return;

    DataManager* dm = DataManager::getInstance();
    const QMap<int, int>& cart = customer->getCart();

    cartTable->setRowCount(cart.size());
    double total = 0;

    int row = 0;
    for (auto it = cart.begin(); it != cart.end(); ++it, ++row) {
        Product* product = dm->getProduct(it.key());
        if (product) {
            double itemTotal = product->getPrice() * it.value();
            total += itemTotal;

            cartTable->setItem(row, 0, new QTableWidgetItem(QString::number(product->getProductId())));
            cartTable->setItem(row, 1, new QTableWidgetItem(product->getName()));
            cartTable->setItem(row, 2, new QTableWidgetItem("$" + QString::number(product->getPrice(), 'f', 2)));
            cartTable->setItem(row, 3, new QTableWidgetItem(QString::number(it.value())));
            cartTable->setItem(row, 4, new QTableWidgetItem("$" + QString::number(itemTotal, 'f', 2)));
        }
    }

    cartTotalLabel->setText("Total: $" + QString::number(total, 'f', 2));
    cartTable->resizeColumnsToContents();
}

void MainWindow::onRemoveFromCart() {
    int row = cartTable->currentRow();
    if (row < 0) {
        showError("Please select an item to remove");
        return;
    }

    int productId = cartTable->item(row, 0)->text().toInt();

    Customer* customer = dynamic_cast<Customer*>(currentUser);
    if (customer) {
        customer->removeFromCart(productId);
        DataManager::getInstance()->saveUsers();
        refreshCart();
    }
}

void MainWindow::onClearCart() {
    Customer* customer = dynamic_cast<Customer*>(currentUser);
    if (customer) {
        customer->clearCart();
        DataManager::getInstance()->saveUsers();
        refreshCart();
        showSuccess("Cart cleared");
    }
}

void MainWindow::onCheckout() {
    Customer* customer = dynamic_cast<Customer*>(currentUser);
    if (!customer) return;

    if (customer->getCart().isEmpty()) {
        showError("Your cart is empty");
        return;
    }

    DataManager* dm = DataManager::getInstance();

    // Calculate total
    double total = 0;
    for (auto it = customer->getCart().begin(); it != customer->getCart().end(); ++it) {
        Product* product = dm->getProduct(it.key());
        if (product)
            total += product->getPrice() * it.value();
    }

    if (customer->getWalletBalance() < total) {
        showError("Insufficient funds. Please add money to your wallet.");
        return;
    }

    int reply = QMessageBox::question(this, "Confirm Purchase",
                                      "Total: $" + QString::number(total, 'f', 2) +
                                          "\nProceed with checkout?",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    // Process purchase
    for (auto it = customer->getCart().begin(); it != customer->getCart().end(); ++it) {
        Product* product = dm->getProduct(it.key());
        if (product) {
            int quantity = it.value();
            double itemTotal = product->getPrice() * quantity;

            customer->deductFunds(itemTotal);
            User* seller = dm->getUser(product->getSellerUsername());
            if (seller) seller->addFunds(itemTotal);
            product->purchase(quantity);

            Transaction trans;
            trans.productId = product->getProductId();
            trans.productName = product->getName();
            trans.sellerUsername = product->getSellerUsername();
            trans.buyerUsername = customer->getUsername();
            trans.quantity = quantity;
            trans.totalPrice = itemTotal;
            trans.date = QDateTime::currentDateTime();
            customer->addTransaction(trans);

            Customer* sellerCustomer = dynamic_cast<Customer*>(seller);
            if (sellerCustomer)
                sellerCustomer->addTransaction(trans);
        }
    }

    customer->clearCart();
    dm->saveAllData();

    refreshCart();
    refreshWallet();
    updateProfileInfo();
    refreshProductList();

    showSuccess("Purchase completed successfully!\n$" + QString::number(total, 'f', 2) + " deducted from your wallet.");
}

// ---------- Wallet Tab ----------
void MainWindow::setupWalletTab() {
    walletTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(walletTab);
    layout->setSpacing(20);

    QGroupBox* balanceGroup = new QGroupBox("Wallet Balance");
    QVBoxLayout* balanceLayout = new QVBoxLayout(balanceGroup);
    walletBalanceLabel = new QLabel("$0.00");
    walletBalanceLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #27ae60;");
    walletBalanceLabel->setAlignment(Qt::AlignCenter);
    balanceLayout->addWidget(walletBalanceLabel);
    layout->addWidget(balanceGroup);

    QGroupBox* depositGroup = new QGroupBox("Add Funds");
    QHBoxLayout* depositLayout = new QHBoxLayout(depositGroup);
    depositSpinBox = new QDoubleSpinBox();
    depositSpinBox->setRange(1, 10000);
    depositSpinBox->setPrefix("$");
    depositSpinBox->setDecimals(2);
    depositSpinBox->setValue(100);
    depositLayout->addWidget(depositSpinBox);
    depositButton = new QPushButton("Deposit");
    depositButton->setObjectName("success");
    connect(depositButton, &QPushButton::clicked, this, &MainWindow::onDepositFunds);
    depositLayout->addWidget(depositButton);
    layout->addWidget(depositGroup);

    QGroupBox* historyGroup = new QGroupBox("Transaction History");
    QVBoxLayout* historyLayout = new QVBoxLayout(historyGroup);
    transactionTable = new QTableWidget();
    transactionTable->setColumnCount(6);
    transactionTable->setHorizontalHeaderLabels(
        QStringList() << "Date" << "Product" << "Seller" << "Buyer" << "Qty" << "Amount");
    transactionTable->horizontalHeader()->setStretchLastSection(true);
    transactionTable->setAlternatingRowColors(true);
    historyLayout->addWidget(transactionTable);
    layout->addWidget(historyGroup);
    layout->addStretch();

    tabWidget->addTab(walletTab, "Wallet");
}

void MainWindow::refreshWallet() {
    if (!walletBalanceLabel) return;
    walletBalanceLabel->setText("$" + QString::number(currentUser->getWalletBalance(), 'f', 2));
    refreshTransactionHistory();
}

void MainWindow::refreshTransactionHistory() {
    Customer* customer = dynamic_cast<Customer*>(currentUser);
    if (!customer) return;

    const QVector<Transaction>& history = customer->getPurchaseHistory();
    transactionTable->setRowCount(history.size());

    for (int i = 0; i < history.size(); ++i) {
        const Transaction& trans = history[i];
        transactionTable->setItem(i, 0, new QTableWidgetItem(trans.date.toString("yyyy-MM-dd hh:mm")));
        transactionTable->setItem(i, 1, new QTableWidgetItem(trans.productName));
        transactionTable->setItem(i, 2, new QTableWidgetItem(trans.sellerUsername));
        transactionTable->setItem(i, 3, new QTableWidgetItem(trans.buyerUsername));
        transactionTable->setItem(i, 4, new QTableWidgetItem(QString::number(trans.quantity)));
        transactionTable->setItem(i, 5, new QTableWidgetItem("$" + QString::number(trans.totalPrice, 'f', 2)));
    }
    transactionTable->resizeColumnsToContents();
}

void MainWindow::onDepositFunds() {
    double amount = depositSpinBox->value();
    if (amount > 0) {
        currentUser->addFunds(amount);
        DataManager::getInstance()->saveUsers();
        refreshWallet();
        updateProfileInfo();
        showSuccess("$" + QString::number(amount, 'f', 2) + " deposited successfully!");
    }
}

void MainWindow::onWithdrawFunds() {
    bool ok;
    double amount = QInputDialog::getDouble(this, "Withdraw Funds",
                                            "Enter amount to withdraw:",
                                            0, 0, currentUser->getWalletBalance(), 2, &ok);
    if (ok && amount > 0) {
        if (currentUser->deductFunds(amount)) {
            DataManager::getInstance()->saveUsers();
            refreshWallet();
            updateProfileInfo();
            showSuccess("$" + QString::number(amount, 'f', 2) + " withdrawn successfully!");
        }
    }
}

// ---------- Admin Tab ----------
void MainWindow::setupAdminTab() {
    adminTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(adminTab);

    adminTabWidget = new QTabWidget();

    // All Products sub‑tab
    adminProductsTab = new QWidget();
    QVBoxLayout* productsLayout = new QVBoxLayout(adminProductsTab);
    adminProductsTable = new QTableWidget();
    adminProductsTable->setColumnCount(7);
    adminProductsTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Name" << "Category" << "Price" << "Stock" << "Status" << "Seller");
    adminProductsTable->horizontalHeader()->setStretchLastSection(true);
    adminProductsTable->setAlternatingRowColors(true);
    adminProductsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    adminProductsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    productsLayout->addWidget(adminProductsTable);

    QHBoxLayout* adminBtnLayout = new QHBoxLayout();
    addProductButton = new QPushButton("Add Product (Direct)");
    connect(addProductButton, &QPushButton::clicked, this, &MainWindow::onAddProduct);
    adminBtnLayout->addWidget(addProductButton);

    registerProductButton = new QPushButton("Register Product (Pending)");
    connect(registerProductButton, &QPushButton::clicked, this, &MainWindow::onRegisterProduct);
    adminBtnLayout->addWidget(registerProductButton);

    editProductButton = new QPushButton("Edit Selected");
    connect(editProductButton, &QPushButton::clicked, this, &MainWindow::onEditProduct);
    adminBtnLayout->addWidget(editProductButton);

    deleteProductButton = new QPushButton("Delete Selected");
    deleteProductButton->setObjectName("danger");
    connect(deleteProductButton, &QPushButton::clicked, this, &MainWindow::onDeleteProduct);
    adminBtnLayout->addWidget(deleteProductButton);

    adminBtnLayout->addStretch();
    productsLayout->addLayout(adminBtnLayout);
    adminTabWidget->addTab(adminProductsTab, "All Products");

    // Pending Approvals sub‑tab
    pendingTab = new QWidget();
    QVBoxLayout* pendingLayout = new QVBoxLayout(pendingTab);
    pendingTable = new QTableWidget();
    pendingTable->setColumnCount(6);
    pendingTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Name" << "Category" << "Price" << "Seller" << "Date");
    pendingTable->horizontalHeader()->setStretchLastSection(true);
    pendingTable->setAlternatingRowColors(true);
    pendingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    pendingTable->setSelectionMode(QAbstractItemView::SingleSelection);
    pendingLayout->addWidget(pendingTable);

    QHBoxLayout* pendingBtnLayout = new QHBoxLayout();
    approveButton = new QPushButton("Approve Selected");
    approveButton->setObjectName("success");
    connect(approveButton, &QPushButton::clicked, this, &MainWindow::onApproveProduct);
    pendingBtnLayout->addWidget(approveButton);

    rejectButton = new QPushButton("Reject Selected");
    rejectButton->setObjectName("danger");
    connect(rejectButton, &QPushButton::clicked, this, &MainWindow::onRejectProduct);
    pendingBtnLayout->addWidget(rejectButton);

    pendingBtnLayout->addStretch();
    pendingLayout->addLayout(pendingBtnLayout);
    adminTabWidget->addTab(pendingTab, "Pending Approvals");

    mainLayout->addWidget(adminTabWidget);
    tabWidget->addTab(adminTab, "Admin");
}

void MainWindow::refreshAdminProducts() {
    DataManager* dm = DataManager::getInstance();
    QVector<Product*> products = dm->getAllProducts();

    adminProductsTable->setRowCount(products.size());

    for (int i = 0; i < products.size(); ++i) {
        Product* p = products[i];
        adminProductsTable->setItem(i, 0, new QTableWidgetItem(QString::number(p->getProductId())));
        adminProductsTable->setItem(i, 1, new QTableWidgetItem(p->getName()));
        adminProductsTable->setItem(i, 2, new QTableWidgetItem(p->getCategory()));
        adminProductsTable->setItem(i, 3, new QTableWidgetItem("$" + QString::number(p->getPrice(), 'f', 2)));
        adminProductsTable->setItem(i, 4, new QTableWidgetItem(QString::number(p->getStock())));
        adminProductsTable->setItem(i, 5, new QTableWidgetItem(p->getStatusString()));
        adminProductsTable->setItem(i, 6, new QTableWidgetItem(p->getSellerUsername()));
    }
    adminProductsTable->resizeColumnsToContents();
}

void MainWindow::refreshPendingProducts() {
    if (!pendingTable) return;
    DataManager* dm = DataManager::getInstance();
    QVector<Product*> pending = dm->getPendingProducts();

    pendingTable->setRowCount(pending.size());

    for (int i = 0; i < pending.size(); ++i) {
        Product* p = pending[i];
        pendingTable->setItem(i, 0, new QTableWidgetItem(QString::number(p->getProductId())));
        pendingTable->setItem(i, 1, new QTableWidgetItem(p->getName()));
        pendingTable->setItem(i, 2, new QTableWidgetItem(p->getCategory()));
        pendingTable->setItem(i, 3, new QTableWidgetItem("$" + QString::number(p->getPrice(), 'f', 2)));
        pendingTable->setItem(i, 4, new QTableWidgetItem(p->getSellerUsername()));
        pendingTable->setItem(i, 5, new QTableWidgetItem(p->getRegistrationDate().toString("yyyy-MM-dd")));
    }
    pendingTable->resizeColumnsToContents();
}

void MainWindow::onApproveProduct() {
    if (!pendingTable) {
        showError("Internal error: Pending table not initialized");
        return;
    }

    int row = pendingTable->currentRow();
    if (row < 0) {
        showError("Please select a product to approve");
        return;
    }

    QTableWidgetItem* item = pendingTable->item(row, 0);
    if (!item) {
        showError("Could not get product ID from selected row");
        return;
    }

    bool ok;
    int productId = item->text().toInt(&ok);
    if (!ok) {
        showError("Invalid product ID");
        return;
    }

    DataManager* dm = DataManager::getInstance();
    Product* product = dm->getProduct(productId);
    if (!product) {
        showError("Product not found");
        return;
    }

    if (!product->isPending()) {
        showError("Product is not in pending status");
        refreshPendingProducts();
        return;
    }

    int reply = QMessageBox::question(this, "Confirm Approval",
                                      "Approve product '" + product->getName() + "' for sale?",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    if (dm->approveProduct(productId)) {
        refreshPendingProducts();
        refreshAdminProducts();
        refreshProductList();
        showSuccess("Product approved and is now visible to customers!");
    } else {
        showError("Failed to approve product");
    }
}

void MainWindow::onRejectProduct() {
    if (!pendingTable) {
        showError("Internal error: Pending table not initialized");
        return;
    }

    int row = pendingTable->currentRow();
    if (row < 0) {
        showError("Please select a product to reject");
        return;
    }

    QTableWidgetItem* item = pendingTable->item(row, 0);
    if (!item) {
        showError("Could not get product ID from selected row");
        return;
    }

    bool ok;
    int productId = item->text().toInt(&ok);
    if (!ok) {
        showError("Invalid product ID");
        return;
    }

    QString productName = pendingTable->item(row, 1) ? pendingTable->item(row, 1)->text() : "Unknown";

    int reply = QMessageBox::question(this, "Confirm Rejection",
                                      "Reject and delete '" + productName + "'?",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    DataManager* dm = DataManager::getInstance();
    Product* product = dm->getProduct(productId);
    if (!product) {
        showError("Product not found");
        refreshPendingProducts();
        return;
    }

    if (!product->isPending()) {
        showError("Product is not in pending status");
        refreshPendingProducts();
        return;
    }

    if (dm->rejectProduct(productId)) {
        refreshPendingProducts();
        refreshAdminProducts();
        showSuccess("Product rejected and removed");
    } else {
        showError("Failed to reject product");
    }
}

void MainWindow::onAddProduct() {
    QDialog dialog(this);
    dialog.setWindowTitle("Add Product (Direct - Approved)");
    dialog.setMinimumWidth(400);

    QFormLayout form(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QTextEdit* descEdit = new QTextEdit(&dialog);
    descEdit->setMaximumHeight(100);
    QComboBox* catCombo = new QComboBox(&dialog);
    catCombo->addItems(QStringList() << "Electronics" << "Clothing" << "Books" << "Home" << "Sports" << "Other");
    QDoubleSpinBox* priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setRange(0.01, 100000);
    priceSpin->setPrefix("$");
    priceSpin->setDecimals(2);
    QSpinBox* stockSpin = new QSpinBox(&dialog);
    stockSpin->setRange(1, 10000);

    form.addRow("Name:*", nameEdit);
    form.addRow("Description:", descEdit);
    form.addRow("Category:*", catCombo);
    form.addRow("Price:*", priceSpin);
    form.addRow("Stock:*", stockSpin);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit->text().isEmpty()) {
            showError("Product name is required");
            return;
        }

        DataManager* dm = DataManager::getInstance();
        int productId = dm->getNextProductId();

        Product* product = new Product(
            productId,
            nameEdit->text(),
            descEdit->toPlainText(),
            catCombo->currentText(),
            priceSpin->value(),
            stockSpin->value(),
            currentUser->getUsername()
            );
        product->setStatus(ProductStatus::APPROVED);
        dm->addProduct(product);

        Customer* adminCustomer = dynamic_cast<Customer*>(currentUser);
        if (adminCustomer)
            adminCustomer->addRegisteredProduct(productId);

        dm->saveAllData();
        refreshAdminProducts();
        refreshProductList();
        showSuccess("Product added and approved!");
    }
}

void MainWindow::onRegisterProduct() {
    QDialog dialog(this);
    dialog.setWindowTitle("Register Product (Pending Approval)");
    dialog.setMinimumWidth(400);

    QFormLayout form(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QTextEdit* descEdit = new QTextEdit(&dialog);
    descEdit->setMaximumHeight(100);
    QComboBox* catCombo = new QComboBox(&dialog);
    catCombo->addItems(QStringList() << "Electronics" << "Clothing" << "Books" << "Home" << "Sports" << "Other");
    QDoubleSpinBox* priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setRange(0.01, 100000);
    priceSpin->setPrefix("$");
    priceSpin->setDecimals(2);
    QSpinBox* stockSpin = new QSpinBox(&dialog);
    stockSpin->setRange(1, 10000);

    form.addRow("Name:*", nameEdit);
    form.addRow("Description:", descEdit);
    form.addRow("Category:*", catCombo);
    form.addRow("Price:*", priceSpin);
    form.addRow("Stock:*", stockSpin);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit->text().isEmpty()) {
            showError("Product name is required");
            return;
        }

        DataManager* dm = DataManager::getInstance();
        int productId = dm->getNextProductId();

        Product* product = new Product(
            productId,
            nameEdit->text(),
            descEdit->toPlainText(),
            catCombo->currentText(),
            priceSpin->value(),
            stockSpin->value(),
            currentUser->getUsername()
            );
        dm->addProduct(product);

        Customer* adminCustomer = dynamic_cast<Customer*>(currentUser);
        if (adminCustomer)
            adminCustomer->addRegisteredProduct(productId);

        dm->saveAllData();
        refreshAdminProducts();
        refreshPendingProducts();
        showSuccess("Product registered and pending approval!");
    }
}

void MainWindow::onEditProduct() {
    int row = adminProductsTable->currentRow();
    if (row < 0) {
        showError("Please select a product to edit");
        return;
    }

    int productId = adminProductsTable->item(row, 0)->text().toInt();
    DataManager* dm = DataManager::getInstance();
    Product* product = dm->getProduct(productId);
    if (!product) {
        showError("Product not found");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Product");
    dialog.setMinimumWidth(400);

    QFormLayout form(&dialog);

    QLineEdit* nameEdit = new QLineEdit(product->getName(), &dialog);
    QTextEdit* descEdit = new QTextEdit(product->getDescription(), &dialog);
    descEdit->setMaximumHeight(100);
    QComboBox* catCombo = new QComboBox(&dialog);
    catCombo->addItems(QStringList() << "Electronics" << "Clothing" << "Books" << "Home" << "Sports" << "Other");
    catCombo->setCurrentText(product->getCategory());
    QDoubleSpinBox* priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setRange(0.01, 100000);
    priceSpin->setPrefix("$");
    priceSpin->setDecimals(2);
    priceSpin->setValue(product->getPrice());
    QSpinBox* stockSpin = new QSpinBox(&dialog);
    stockSpin->setRange(0, 10000);
    stockSpin->setValue(product->getStock());

    form.addRow("Name:*", nameEdit);
    form.addRow("Description:", descEdit);
    form.addRow("Category:*", catCombo);
    form.addRow("Price:*", priceSpin);
    form.addRow("Stock:*", stockSpin);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        product->setName(nameEdit->text());
        product->setDescription(descEdit->toPlainText());
        product->setCategory(catCombo->currentText());
        product->setPrice(priceSpin->value());
        product->setStock(stockSpin->value());

        dm->saveProducts();
        refreshAdminProducts();
        refreshProductList();
        showSuccess("Product updated!");
    }
}

void MainWindow::onDeleteProduct() {
    int row = adminProductsTable->currentRow();
    if (row < 0) {
        showError("Please select a product to delete");
        return;
    }

    int productId = adminProductsTable->item(row, 0)->text().toInt();
    QString productName = adminProductsTable->item(row, 1)->text();

    int reply = QMessageBox::question(this, "Confirm Delete",
                                      "Are you sure you want to delete '" + productName + "'?",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        DataManager* dm = DataManager::getInstance();
        if (dm->removeProduct(productId)) {
            refreshAdminProducts();
            refreshProductList();
            refreshPendingProducts();
            showSuccess("Product deleted!");
        }
    }
}

// ---------- My Products Tab ----------
void MainWindow::setupMyProductsTab() {
    myProductsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(myProductsTab);
    layout->setSpacing(15);

    myProductsInfoLabel = new QLabel("Products you have registered for sale:");
    layout->addWidget(myProductsInfoLabel);

    myProductsTable = new QTableWidget();
    myProductsTable->setColumnCount(6);
    myProductsTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Name" << "Category" << "Price" << "Stock" << "Status");
    myProductsTable->horizontalHeader()->setStretchLastSection(true);
    myProductsTable->setAlternatingRowColors(true);
    layout->addWidget(myProductsTable);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    registerNewProductButton = new QPushButton("Register New Product");
    registerNewProductButton->setObjectName("success");
    connect(registerNewProductButton, &QPushButton::clicked, this, &MainWindow::onRegisterNewProduct);
    buttonLayout->addWidget(registerNewProductButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);
    tabWidget->addTab(myProductsTab, "My Products");
}

void MainWindow::refreshMyProducts() {
    Customer* customer = dynamic_cast<Customer*>(currentUser);
    if (!customer) return;

    DataManager* dm = DataManager::getInstance();

    // Get all products and filter by seller username - this is the key fix
    QVector<Product*> allProducts = dm->getAllProducts();
    QVector<Product*> myProducts;

    QString myUsername = currentUser->getUsername();

    for (Product* p : allProducts) {
        if (p->getSellerUsername() == myUsername) {
            myProducts.append(p);
        }
    }

    myProductsTable->setRowCount(myProducts.size());

    for (int i = 0; i < myProducts.size(); ++i) {
        Product* p = myProducts[i];
        myProductsTable->setItem(i, 0, new QTableWidgetItem(QString::number(p->getProductId())));
        myProductsTable->setItem(i, 1, new QTableWidgetItem(p->getName()));
        myProductsTable->setItem(i, 2, new QTableWidgetItem(p->getCategory()));
        myProductsTable->setItem(i, 3, new QTableWidgetItem("$" + QString::number(p->getPrice(), 'f', 2)));
        myProductsTable->setItem(i, 4, new QTableWidgetItem(QString::number(p->getStock())));
        myProductsTable->setItem(i, 5, new QTableWidgetItem(p->getStatusString()));
    }
    myProductsTable->resizeColumnsToContents();
}

void MainWindow::onRegisterNewProduct() {
    QDialog dialog(this);
    dialog.setWindowTitle("Register New Product for Sale");
    dialog.setMinimumWidth(400);

    QFormLayout form(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QTextEdit* descEdit = new QTextEdit(&dialog);
    descEdit->setMaximumHeight(100);
    QComboBox* catCombo = new QComboBox(&dialog);
    catCombo->addItems(QStringList() << "Electronics" << "Clothing" << "Books" << "Home" << "Sports" << "Other");
    QDoubleSpinBox* priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setRange(0.01, 100000);
    priceSpin->setPrefix("$");
    priceSpin->setDecimals(2);
    QSpinBox* stockSpin = new QSpinBox(&dialog);
    stockSpin->setRange(1, 10000);

    form.addRow("Name:*", nameEdit);
    form.addRow("Description:", descEdit);
    form.addRow("Category:*", catCombo);
    form.addRow("Price:*", priceSpin);
    form.addRow("Stock:*", stockSpin);

    QLabel* infoLabel = new QLabel("Your product will be pending admin approval before appearing in the store.");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; font-style: italic;");
    form.addRow(infoLabel);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit->text().isEmpty()) {
            showError("Product name is required");
            return;
        }

        DataManager* dm = DataManager::getInstance();
        int productId = dm->getNextProductId();

        // Create product with current user as seller
        Product* product = new Product(
            productId,
            nameEdit->text(),
            descEdit->toPlainText(),
            catCombo->currentText(),
            priceSpin->value(),
            stockSpin->value(),
            currentUser->getUsername()  // Ensure seller is set correctly
            );

        if (dm->addProduct(product)) {
            // Also add to customer's registered products for backward compatibility
            Customer* customer = dynamic_cast<Customer*>(currentUser);
            if (customer) {
                customer->addRegisteredProduct(productId);
                dm->saveUsers();  // Save the updated registered products list
            }

            dm->saveAllData();
            refreshMyProducts();
            showSuccess("Product registered! It will appear in the store after admin approval.");
        } else {
            showError("Failed to register product");
            delete product;
        }
    }
}
void MainWindow::onChangePassword() {
    QString currentPass = currentPasswordEdit->text();
    QString newPass = newPasswordEdit->text();
    QString confirmPass = confirmNewPasswordEdit->text();

    // Validate current password
    DataManager* dm = DataManager::getInstance();
    QString currentHashed = User::hashPassword(currentPass);

    if (currentUser->getHashedPassword() != currentHashed) {
        showError("Current password is incorrect");
        return;
    }

    // Check new password not empty
    if (newPass.isEmpty()) {
        showError("New password cannot be empty");
        return;
    }

    // Validate new password strength (at least 8 chars, upper, lower, number)
    if (!User::validatePassword(newPass)) {
        showError("New password must be at least 8 characters with uppercase, lowercase, and number");
        return;
    }

    // Check confirmation matches
    if (newPass != confirmPass) {
        showError("New password and confirmation do not match");
        return;
    }

    // Check new password is different from current
    if (currentPass == newPass) {
        showError("New password must be different from current password");
        return;
    }

    // Update password
    QString newHashed = User::hashPassword(newPass);
    currentUser->setHashedPassword(newHashed);

    // Save to file
    dm->saveUsers();

    // Clear fields
    currentPasswordEdit->clear();
    newPasswordEdit->clear();
    confirmNewPasswordEdit->clear();

    showSuccess("Password changed successfully!");
}


// ---------- Helpers ----------
void MainWindow::showError(const QString& message) {
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::showSuccess(const QString& message) {
    QMessageBox::information(this, "Success", message);
}
