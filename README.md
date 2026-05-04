# 🔐 CS515 Ciphers Project

## 📌 Overview

This repository contains implementations of classical cryptographic ciphers developed as part of the **CS515 course project**. The goal is to demonstrate how basic encryption and decryption techniques work using simple algorithms such as substitution and transposition ciphers.

These implementations are intended for **educational purposes**, helping students understand the fundamentals of cryptography before moving to modern encryption systems.

---

## 🧠 Features

* Implementation of classical ciphers
* Encryption and decryption support
* Command-line based interaction
* Readable C++ code structure
* Useful for learning cryptography concepts

---

## 🔑 Implemented Ciphers

The project includes (or is expected to include) the following ciphers:

### 1. Caesar Cipher

A substitution cipher where each character is shifted by a fixed number of positions in the alphabet.

### 2. Rail Fence Cipher

A transposition cipher that writes text in a zig-zag pattern across multiple “rails” and reads it row by row.

### 3. Row Transposition Cipher

Text is written into a grid based on a key and then read row-wise according to a key order.

---

## ⚙️ How It Works

1. User selects a cipher type.
2. Input plaintext is provided.
3. A key is entered (if required).
4. The program outputs:

   * Encrypted text (ciphertext)
   * Decrypted text (to verify correctness)

---

## 🛠️ Technologies Used

* C++
* Standard libraries only (no external dependencies)
* CLI-based execution

---

## 🚀 Getting Started

### Clone the repository

```bash
git clone https://github.com/maamostafa/cs515_ciphers.git
cd cs515_ciphers
```

### Compile

```bash
./buid.sh
```

## 📚 Educational Objectives

This project helps to understand:

* Basics of symmetric encryption
* Difference between substitution and transposition
* Importance of keys in cryptography
* How plaintext is transformed into ciphertext

---

## 📖 Example

### Input:

```
Plaintext: HELLO
Key: 3
Cipher: Caesar
```

### Output:

```
Ciphertext: KHOOR
```

---

## ⚠️ Disclaimer

This project is for **educational purposes only** and should not be used for securing real-world sensitive data.

---

## 👨‍💻 Authors

Developed as part of CS515 coursework by:
* Ahmed Gameil
* Mohammed Mostafa
