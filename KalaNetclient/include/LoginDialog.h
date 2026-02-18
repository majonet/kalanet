#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QComboBox>
#include "User.h"

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();

    User* getAuthenticatedUser() const { return currentUser; }
    bool isLoggedIn() const { return loggedIn; }

private slots:
    void onLoginClicked();
    void onSignupClicked();
    void onSwitchToSignup();
    void onSwitchToLogin();
    void onCancelClicked();

private:
    void setupUI();
    void setupLoginPage();
    void setupSignupPage();
    void clearFields();
    bool validateSignupFields();

    QStackedWidget* stackedWidget;

    // Login page widgets
    QWidget* loginPage;
    QLineEdit* loginUsernameEdit;
    QLineEdit* loginPasswordEdit;
    QPushButton* loginButton;
    QPushButton* switchToSignupButton;
    QLabel* loginErrorLabel;

    // Signup page widgets
    QWidget* signupPage;
    QLineEdit* signupUsernameEdit;
    QLineEdit* signupPasswordEdit;
    QLineEdit* signupConfirmPasswordEdit;
    QLineEdit* signupEmailEdit;
    QLineEdit* signupPhoneEdit;
    QLineEdit* signupAddressEdit;
    QComboBox* userTypeCombo;
    QPushButton* signupButton;
    QPushButton* switchToLoginButton;
    QPushButton* cancelButton;
    QLabel* signupErrorLabel;

    User* currentUser;
    bool loggedIn;
};

#endif // LOGINDIALOG_H