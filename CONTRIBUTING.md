# Contributing to the MiP Power Up library

Thank you for taking the time to contribute to this library! 

This is a small project maintained by a hobbyist, meant to make controlling MiP easy, accessible, and fun for everyone. Whether you are refining the API, expanding upon MiP's capabilities, fixing a bug, or improving documentation to clarify confusing issues, your contributions are highly welcome and deeply appreciated!

---

## 🛠️ Contribution Guidelines

To keep the codebase clean, consistent, and easy to maintain, please follow these guidelines when preparing a pull request:

### 1. Code Formatting
* **Use the Included `.clang-format`:** Please format all C++ header (`.h`), source (`.cpp`), and Arduino sketch (`.ino`) files using the `.clang-format` file included in the root of the repository before submitting a pull request.
* **Consistent Style:** Ensure clean indentation, explicit type casting where appropriate, and avoid raw string literals in dynamic RAM when logging debug messages (use the `F()` macro where possible).

### 2. Personification & Tone
* **Treat MiP as a Person:** Refer to MiP directly by name, "MiP."
* **Avoid Impersonal Phrasing:** Don't refer to MiP as *"the MiP"*, *"the robot"*, or *"the MiP robot"*. Keep descriptions friendly and personified across all documentation, Doxygen comments, and log output.

### 3. Documentation & Doxygen
* **Document Public Elements:** If you add or modify public classes, structs, enums, or functions, please include clear Doxygen comments (`@brief`, `@details`, `@param`, `@return`).
* **Keep Descriptions Clear:** Help users understand *why* a function is used and *what* to expect, ensuring the generated Wiki and API reference remain accurate and helpful.

### 4. Testing
* **Verify Examples:** If your changes affect core methods, make sure the library example sketches compile cleanly and function as expected.

---

## 🚀 How to Submit a Pull Request

1. **Fork the Repository:** Create a fork of the repository and create a new branch for your feature or bug fix.
2. **Commit Your Changes:** Keep commits focused and include clear, descriptive commit messages.
3. **Format Your Code:** Apply `.clang-format` across modified files.
4. **Open a Pull Request:** Submit your PR against the `master` branch with a short summary of the changes you've made.

Thank you again for helping improve this project for the community!
