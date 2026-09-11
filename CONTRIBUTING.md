# Contributing to Tarplex Scinfinity Engine Omega

Thank you for your interest in contributing! This document provides guidelines for participating in the project.

## Code of Conduct

Be respectful, inclusive, and professional in all interactions.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/tarplex-scinfinity-engine-omega.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Commit with descriptive messages: `git commit -m "feat: add new feature"`
6. Push to your fork: `git push origin feature/your-feature-name`
7. Open a Pull Request

## Development Setup

See BUILD.md for detailed setup instructions.

## Coding Standards

### C++ Style

- Use modern C++17 features
- Follow Google C++ Style Guide
- Use meaningful variable names
- Add comments for complex logic
- Keep functions focused and small

### Example

```cpp
// Good
void ProcessHTMLNode(const HTMLNode& node) {
    // Validate input
    if (!node.IsValid()) {
        LOG(ERROR) << "Invalid node";
        return;
    }
    
    // Process the node
    // ...
}

// Avoid
void phn(const HTMLNode& n) {
    if (n.Valid()) {
        // process
    }
}
```

## Commit Messages

Use conventional commit format:

```
type(scope): subject

body (optional)

footer (optional)
```

Types:
- `feat` - New feature
- `fix` - Bug fix
- `docs` - Documentation
- `style` - Code style changes
- `refactor` - Code refactoring
- `perf` - Performance improvements
- `test` - Test additions/changes

Example:
```
feat(renderer): implement CSS flexbox layout

Adds support for CSS flexbox layout algorithm.
Implements flex-direction, justify-content, and align-items properties.

Fixes #123
```

## Testing

- Write unit tests for new features
- Ensure all tests pass: `ctest`
- Add integration tests for complex features
- Test on multiple platforms (Windows, Linux, WSL)

## Documentation

- Update README.md if adding new features
- Document public APIs with Doxygen comments
- Update ARCHITECTURE.md for major changes
- Include usage examples where appropriate

## Pull Request Process

1. Update documentation and tests
2. Ensure code passes all tests
3. Provide clear PR description
4. Link related issues
5. Be responsive to review feedback

## Areas for Contribution

### High Priority
- Core rendering engine optimization
- JavaScript interpreter improvements
- Security enhancements
- Bug fixes

### Medium Priority
- UI improvements
- Documentation
- Build system optimization
- Platform-specific enhancements

### Community
- Bug reports
- Feature suggestions
- Testing on different platforms
- Translation (i18n)

## Reporting Issues

When reporting bugs, include:
- OS and version
- Steps to reproduce
- Expected vs actual behavior
- Screenshots/logs if applicable

## Questions?

Open a GitHub Discussion or create an issue.

---

Thank you for contributing to making the web better! 🚀
