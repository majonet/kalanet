#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include "User.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(User* user, QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void logoutRequested();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Profile tab
    void onUpdateProfile();
    void onAddFunds();
    void onLogout();
    void onChangePassword();

    // Products tab
    void onSearchProducts();
    void onCategoryChanged(int index);
    void onAddToCart();
    void onViewProductDetails();
    void refreshProductList();

    // Cart tab
    void onRemoveFromCart();
    void onClearCart();
    void onCheckout();
    void refreshCart();

    // Wallet tab
    void onDepositFunds();
    void onWithdrawFunds();
    void refreshWallet();
    void refreshTransactionHistory();

    // Admin tab
    void onAddProduct();
    void onRegisterProduct();      // For both admin and customer
    void onEditProduct();
    void onDeleteProduct();
    void onApproveProduct();
    void onRejectProduct();
    void refreshAdminProducts();
    void refreshPendingProducts();

    // My Products tab (for customers)
    void onRegisterNewProduct();
    void refreshMyProducts();
    void onUploadProductImage();
    void onClearProductImage();
private:
    void setupUI();
    void setupProfileTab();
    void setupProductsTab();
    void setupCartTab();
    void setupWalletTab();
    void setupAdminTab();
    void setupMyProductsTab();

    void updateProfileInfo();
    void showError(const QString& message);
    void showSuccess(const QString& message);

    User* currentUser;
    bool isAdmin;

    // Main widget
    QTabWidget* tabWidget;

    // Profile tab
    QWidget* profileTab;
    QLabel* usernameLabel;
    QLabel* userTypeLabel;
    QLabel* walletLabel;
    QLineEdit* profileEmailEdit;
    QLineEdit* profilePhoneEdit;
    QLineEdit* profileAddressEdit;
    QPushButton* updateProfileButton;
    // Password change
    QLineEdit* currentPasswordEdit;
    QLineEdit* newPasswordEdit;
    QLineEdit* confirmNewPasswordEdit;
    QPushButton* changePasswordButton;

    QPushButton* addFundsButton;
    QPushButton* logoutButton;

    // Products tab
    QWidget* productsTab;
    QLineEdit* searchEdit;
    QComboBox* categoryCombo;
    QPushButton* searchButton;
    QTableWidget* productsTable;
    QPushButton* addToCartButton;
    QPushButton* viewDetailsButton;
    QLabel* productDetailsLabel;
    QLabel* productImageLabel;      // For showing product image in dialogs
    QPushButton* uploadImageButton; // For uploading image
    QPushButton* clearImageButton;  // For clearing image
    QString currentImageBase64;

    // Cart tab
    QWidget* cartTab;
    QTableWidget* cartTable;
    QLabel* cartTotalLabel;
    QPushButton* removeFromCartButton;
    QPushButton* clearCartButton;
    QPushButton* checkoutButton;

    // Wallet tab
    QWidget* walletTab;
    QLabel* walletBalanceLabel;
    QDoubleSpinBox* depositSpinBox;
    QPushButton* depositButton;
    QTableWidget* transactionTable;

    // Admin tab
    QWidget* adminTab;
    QTabWidget* adminTabWidget;

    // Admin - All Products
    QWidget* adminProductsTab;
    QTableWidget* adminProductsTable;
    QPushButton* addProductButton;
    QPushButton* registerProductButton;
    QPushButton* editProductButton;
    QPushButton* deleteProductButton;

    // Admin - Pending Approvals
    QWidget* pendingTab;
    QTableWidget* pendingTable;
    QPushButton* approveButton;
    QPushButton* rejectButton;

    // My Products tab (for customers)
    QWidget* myProductsTab;
    QTableWidget* myProductsTable;
    QPushButton* registerNewProductButton;
    QLabel* myProductsInfoLabel;
};

#endif
