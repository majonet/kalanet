# KalaNet - Qt C++ Shopping Application

A complete client-server shopping application built with Qt framework and C++ for IUT Advanced Programming course.

## Features

### User Management
- **Login/Signup** with password hashing (SHA-256)
- **Password validation**: Minimum 8 characters, uppercase, lowercase, and numbers
- **Two user types**: Admin and Customer
- **Persistent data storage** using binary serialization

### Product Management
- **Product registration** by any user (Admin or Customer)
- **Admin approval system** for products
- **Product categories**: Electronics, Clothing, Books, Home, Sports, Other
- **Search and filter** products
- **Stock management**

### Shopping Cart
- **Add to cart** with quantity selection
- **Remove items** from cart
- **Clear cart** functionality
- **Checkout** with wallet payment

### Wallet System
- **Add funds** to wallet
- **Automatic payment** processing on checkout
- **Transaction history** with buyer/seller information
- **Seller receives payment** when product is sold

### Admin Features
- **Approve/Reject** pending products
- **Direct product addition** (approved immediately)
- **Edit/Delete** any product
- **View all products** and their status

### Customer Features
- **Register products** for sale (pending approval)
- **View my products** and their status
- **Purchase products** from store
- **View transaction history**

## Technical Implementation

### Object-Oriented Programming
- **Inheritance**: Admin and Customer inherit from User base class
- **Polymorphism**: Virtual methods for user type-specific behavior
- **Encapsulation**: Private data with public getters/setters
- **Singleton Pattern**: DataManager for centralized data access

### STL Containers
- `QVector` for lists
- `QMap` for key-value storage (users, cart)
- `QQueue` for pending operations

### Multi-threading
- `QMutex` for thread-safe data access
- `QMutexLocker` for automatic lock management

### File I/O
- Binary serialization with `QDataStream`
- Automatic data persistence
- Data directory: `./data/`

## Project Structure

```
KalaNet/
├── include/
│   ├── Product.h         # Product class with status management
│   ├── User.h            # User base class + Admin/Customer
│   ├── DataManager.h     # Singleton data manager
│   ├── LoginDialog.h     # Login/Signup dialog
│   └── MainWindow.h      # Main application window
├── src/
│   ├── main.cpp          # Application entry point
│   ├── Product.cpp       # Product implementation
│   ├── User.cpp          # User classes implementation
│   ├── DataManager.cpp   # Data persistence
│   ├── LoginDialog.cpp   # Login UI
│   └── MainWindow.cpp    # Main UI
├── data/                 # Data storage directory
├── KalaNet.pro          # Qt project file
├── CMakeLists.txt       # CMake configuration
├── build.bat            # Windows build script
└── build.sh             # Linux/macOS build script
```

## Building the Project

### Option 1: Using Qt Creator (Recommended)
1. Open `KalaNet.pro` in Qt Creator
2. Press **Ctrl+B** to build
3. Press **Ctrl+R** to run

### Option 2: Command Line - Windows
```cmd
cd KalaNet
build.bat
```

### Option 3: Command Line - Linux/macOS
```bash
cd KalaNet
chmod +x build.sh
./build.sh
```

### Option 4: Using CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Default Login

- **Username**: `admin`
- **Password**: `Admin123`

## Data Persistence

All data is automatically saved to the `data/` directory:
- `users.dat` - User accounts, passwords, wallets, cart, transaction history
- `products.dat` - Product catalog with IDs, status, and details

Data is loaded automatically when the application starts and saved on logout or exit.

## Usage Guide

### For Customers:
1. **Login** or **Create Account**
2. **Browse Products** - View approved products in the Products tab
3. **Add to Cart** - Select products and quantities
4. **Checkout** - Pay using wallet balance
5. **Register Products** - Add your own products for sale (requires admin approval)
6. **View My Products** - Check status of your registered products
7. **Add Funds** - Deposit money to your wallet

### For Admins:
1. **Login** with admin credentials
2. **Manage Products** - Add, edit, delete products directly
3. **Approve Products** - Review and approve pending products from users
4. **View All Data** - Access complete product catalog

## Payment Flow

1. Customer adds products to cart
2. Customer clicks **Checkout**
3. System validates wallet balance
4. Amount is deducted from buyer's wallet
5. Amount is added to seller's wallet
6. Product stock is reduced
7. Transaction is recorded for both parties

## Security Features

- Passwords are hashed using **SHA-256**
- Password validation enforces strong passwords
- Thread-safe data access with mutexes
- Data integrity through binary serialization

## Requirements

- Qt 5.15+ or Qt 6.x
- C++17 compatible compiler
- CMake 3.16+ (optional)

## Authors

- Advanced Programming Course Project
- IUT (Isfahan University of Technology)

## License

This project is for educational purposes.
