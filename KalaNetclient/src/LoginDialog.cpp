#include "LoginDialog.h"
#include "DataManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>
#include <QFrame>
#include <QRegularExpression>   // for phone validation

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), currentUser(nullptr), loggedIn(false) {
    setupUI();
    setWindowTitle("KalaNet - Login");
    setMinimumSize(400, 500);
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLabel {
            color: #333;
            font-size: 14px;
        }
        QLineEdit {
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #e74c3c;
        }
        QPushButton {
            padding: 12px 24px;
            background-color: #e74c3c;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 14px;
            font-weight: bold;
            cursor: pointer;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
        QPushButton:pressed {
            background-color: #a93226;
        }
        QComboBox {
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
            background-color: white;
        }
    )");
}

LoginDialog::~LoginDialog() {
}

void LoginDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Title
    QLabel* titleLabel = new QLabel("KalaNet");
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e74c3c;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QLabel* subtitleLabel = new QLabel("The Best Shopping Experience in IUT");
    subtitleLabel->setStyleSheet("font-size: 14px; color: #666; margin-bottom: 20px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    // Stacked widget for login/signup
    stackedWidget = new QStackedWidget();
    setupLoginPage();
    setupSignupPage();
    mainLayout->addWidget(stackedWidget);

    mainLayout->addStretch();
}

void LoginDialog::setupLoginPage() {
    loginPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(loginPage);
    layout->setSpacing(15);

    QLabel* headerLabel = new QLabel("Welcome Back");
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // Username
    QLabel* userLabel = new QLabel("Username:");
    layout->addWidget(userLabel);
    loginUsernameEdit = new QLineEdit();
    loginUsernameEdit->setPlaceholderText("Enter your username");
    layout->addWidget(loginUsernameEdit);

    // Password
    QLabel* passLabel = new QLabel("Password:");
    layout->addWidget(passLabel);
    loginPasswordEdit = new QLineEdit();
    loginPasswordEdit->setPlaceholderText("Enter your password");
    loginPasswordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(loginPasswordEdit);

    // Error label
    loginErrorLabel = new QLabel();
    loginErrorLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
    loginErrorLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(loginErrorLabel);

    // Login button
    loginButton = new QPushButton("Login");
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    layout->addWidget(loginButton);

    // Switch to signup
    QHBoxLayout* switchLayout = new QHBoxLayout();
    switchLayout->addStretch();
    QLabel* noAccountLabel = new QLabel("Don't have an account?");
    noAccountLabel->setStyleSheet("color: #666;");
    switchLayout->addWidget(noAccountLabel);
    switchToSignupButton = new QPushButton("Sign Up");
    switchToSignupButton->setStyleSheet("background-color: transparent; color: #e74c3c; text-decoration: underline; padding: 5px;");
    connect(switchToSignupButton, &QPushButton::clicked, this, &LoginDialog::onSwitchToSignup);
    switchLayout->addWidget(switchToSignupButton);
    switchLayout->addStretch();
    layout->addLayout(switchLayout);

    layout->addStretch();
    stackedWidget->addWidget(loginPage);
}

void LoginDialog::setupSignupPage() {
    signupPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(signupPage);
    layout->setSpacing(10);

    QLabel* headerLabel = new QLabel("Create Account");
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // Form fields
    QGridLayout* formLayout = new QGridLayout();
    formLayout->setSpacing(10);

    // Username
    formLayout->addWidget(new QLabel("Username:*"), 0, 0);
    signupUsernameEdit = new QLineEdit();
    signupUsernameEdit->setPlaceholderText("Choose a username");
    formLayout->addWidget(signupUsernameEdit, 0, 1);

    // Password
    formLayout->addWidget(new QLabel("Password:*"), 1, 0);
    signupPasswordEdit = new QLineEdit();
    signupPasswordEdit->setPlaceholderText("Min 8 chars, upper, lower, number");
    signupPasswordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addWidget(signupPasswordEdit, 1, 1);

    // Confirm Password
    formLayout->addWidget(new QLabel("Confirm:*"), 2, 0);
    signupConfirmPasswordEdit = new QLineEdit();
    signupConfirmPasswordEdit->setPlaceholderText("Confirm password");
    signupConfirmPasswordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addWidget(signupConfirmPasswordEdit, 2, 1);

    // Email
    formLayout->addWidget(new QLabel("Email:*"), 3, 0);
    signupEmailEdit = new QLineEdit();
    signupEmailEdit->setPlaceholderText("your@email.com");
    formLayout->addWidget(signupEmailEdit, 3, 1);

    // Phone
    formLayout->addWidget(new QLabel("Phone:*"), 4, 0);
    signupPhoneEdit = new QLineEdit();
    signupPhoneEdit->setPlaceholderText("Your phone number (11 digits)");
    formLayout->addWidget(signupPhoneEdit, 4, 1);

    // Address
    formLayout->addWidget(new QLabel("Address:*"), 5, 0);
    signupAddressEdit = new QLineEdit();
    signupAddressEdit->setPlaceholderText("Your address");
    formLayout->addWidget(signupAddressEdit, 5, 1);

    // User Type
    formLayout->addWidget(new QLabel("Account Type:*"), 6, 0);
    userTypeCombo = new QComboBox();
    userTypeCombo->addItem("Customer", static_cast<int>(UserType::CUSTOMER));
    userTypeCombo->addItem("Admin", static_cast<int>(UserType::ADMIN));
    formLayout->addWidget(userTypeCombo, 6, 1);

    layout->addLayout(formLayout);

    // Error label
    signupErrorLabel = new QLabel();
    signupErrorLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
    signupErrorLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(signupErrorLabel);

    // Signup button
    signupButton = new QPushButton("Create Account");
    connect(signupButton, &QPushButton::clicked, this, &LoginDialog::onSignupClicked);
    layout->addWidget(signupButton);

    // Buttons layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    switchToLoginButton = new QPushButton("Back to Login");
    switchToLoginButton->setStyleSheet("background-color: #95a5a6;");
    connect(switchToLoginButton, &QPushButton::clicked, this, &LoginDialog::onSwitchToLogin);
    buttonLayout->addWidget(switchToLoginButton);

    cancelButton = new QPushButton("Cancel");
    cancelButton->setStyleSheet("background-color: #7f8c8d;");
    connect(cancelButton, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);
    layout->addStretch();

    stackedWidget->addWidget(signupPage);
}

void LoginDialog::onLoginClicked() {
    QString username = loginUsernameEdit->text().trimmed();
    QString password = loginPasswordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        loginErrorLabel->setText("Please enter both username and password");
        return;
    }

    DataManager* dm = DataManager::getInstance();

    if (!dm->userExists(username)) {
        loginErrorLabel->setText("Username not found");
        return;
    }

    if (!dm->validateLogin(username, password)) {
        loginErrorLabel->setText("Invalid password");
        return;
    }

    currentUser = dm->getUser(username);
    loggedIn = true;
    accept();
}

void LoginDialog::onSignupClicked() {
    signupErrorLabel->clear();

    if (!validateSignupFields()) {
        return;
    }

    DataManager* dm = DataManager::getInstance();

    QString username = signupUsernameEdit->text().trimmed();
    QString password = signupPasswordEdit->text();
    QString email = signupEmailEdit->text().trimmed();
    QString phone = signupPhoneEdit->text().trimmed();
    QString address = signupAddressEdit->text().trimmed();
    UserType type = static_cast<UserType>(userTypeCombo->currentData().toInt());

    if (dm->userExists(username)) {
        signupErrorLabel->setText("Username already exists");
        return;
    }

    QString hashedPass = User::hashPassword(password);

    User* newUser = nullptr;
    if (type == UserType::ADMIN) {
        newUser = new Admin(username, hashedPass, email, phone, address);
    } else {
        newUser = new Customer(username, hashedPass, email, phone, address);
    }

    if (dm->addUser(newUser)) {
        QMessageBox::information(this, "Success", "Account created successfully! Please login.");
        onSwitchToLogin();
        clearFields();
    } else {
        signupErrorLabel->setText("Failed to create account");
        delete newUser;
    }
}

void LoginDialog::onSwitchToSignup() {
    stackedWidget->setCurrentWidget(signupPage);
    loginErrorLabel->clear();
}

void LoginDialog::onSwitchToLogin() {
    stackedWidget->setCurrentWidget(loginPage);
    signupErrorLabel->clear();
}

void LoginDialog::onCancelClicked() {
    reject();
}

void LoginDialog::clearFields() {
    loginUsernameEdit->clear();
    loginPasswordEdit->clear();
    signupUsernameEdit->clear();
    signupPasswordEdit->clear();
    signupConfirmPasswordEdit->clear();
    signupEmailEdit->clear();
    signupPhoneEdit->clear();
    signupAddressEdit->clear();
}

// ==================== Updated validation with phone check ====================
bool LoginDialog::validateSignupFields() {
    QString username = signupUsernameEdit->text().trimmed();
    QString password = signupPasswordEdit->text();
    QString confirmPass = signupConfirmPasswordEdit->text();
    QString email = signupEmailEdit->text().trimmed();
    QString phone = signupPhoneEdit->text().trimmed();
    QString address = signupAddressEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty() || confirmPass.isEmpty() ||
        email.isEmpty() || phone.isEmpty() || address.isEmpty()) {
        signupErrorLabel->setText("All fields are required");
        return false;
    }

    if (password != confirmPass) {
        signupErrorLabel->setText("Passwords do not match");
        return false;
    }

    if (!User::validatePassword(password)) {
        signupErrorLabel->setText("Password must be at least 8 characters with uppercase, lowercase, and number");
        return false;
    }

    // Simple email validation
        if (!email.contains("@") || !email.contains(".")) {
        signupErrorLabel->setText("Please enter a valid email address");
        return false;
    }


    // Phone validation: exactly 11 digits
    QRegularExpression phoneRegex("^\\d{11}$");
    if (!phoneRegex.match(phone).hasMatch()) {
        signupErrorLabel->setText("Phone number must be exactly 11 digits");
        return false;
    }

    return true;
}
