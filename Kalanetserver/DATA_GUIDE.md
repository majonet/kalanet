# KalaNet Data Persistence Guide

## Where is my data saved?

The application saves data to a `data` folder. The location depends on how you run the program:

### When Running from Qt Creator:
**Location:** `build/Desktop_Qt_6_5_3_MinGW_64_bit-Debug/data/`
(or similar, depending on your kit name)

The data is saved in the **build directory**, not the source directory.

### When Running Executable Directly:
**Location:** Same folder as the executable, in a `data` subfolder

Example:
```
KalaNet/
├── KalaNet.exe
└── data/
    ├── users.dat
    └── products.dat
```

## Files Created

1. **users.dat** - Contains all user accounts:
   - Usernames and hashed passwords
   - Profile information (email, phone, address)
   - Wallet balances
   - Shopping carts
   - Purchase history
   - Registered product IDs

2. **products.dat** - Contains all products:
   - Product details (name, description, price, stock)
   - Status (Pending/Approved/Sold)
   - Seller information
   - Registration dates

## Important Notes

### Data is Portable
You can copy the `data` folder to another location:
1. Copy `data/users.dat` and `data/products.dat`
2. Place them in the new location's `data` folder
3. All users, products, and history will be preserved

### Clean Build Deletes Data
If you **delete the build folder**, you **lose all data**!

To preserve data:
1. Copy the `data` folder before cleaning
2. Paste it back after rebuilding

### Finding Your Data

The application prints the data location in the console:
```
Data directory: F:/final_ap_projeject/KalaNet/build/Desktop_Qt_6_5_3_MinGW_64_bit-Debug/data
```

Check the Qt Creator "Application Output" panel to see this message.

## Backup Your Data

To backup:
1. Go to your build directory
2. Copy the entire `data` folder
3. Store it somewhere safe

To restore:
1. Paste the `data` folder back into the build directory
2. Run the application

## Troubleshooting

### "No previous users found"
**Cause:** Data is being saved to a different location than where it's being loaded from.

**Solution:** 
1. Check the console output for "Data directory:" path
2. Verify that `users.dat` exists in that folder
3. If not, the data might be in a different build folder

### Data not saving
**Cause:** Application doesn't have write permissions.

**Solution:**
1. Run Qt Creator as Administrator (Windows)
2. Or change the build directory to a location with write permissions

### Lost all data after rebuild
**Cause:** Build directory was cleaned/deleted.

**Prevention:**
Always backup the `data` folder before:
- Cleaning the project
- Deleting the build folder
- Switching between Debug/Release builds

## Default Admin Account

If no users exist, the application creates:
- **Username:** `admin`
- **Password:** `Admin123`

This only happens on first run (when no `users.dat` exists).
