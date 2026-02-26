# Contributing to Engine Calculator

First off, thank you for considering contributing to Engine Calculator! ??

## ?? **Our Standards**

Engine Calculator is **scientific/engineering software**. This means:

- ? **ALL formulas MUST have scientific references**
- ? **Code MUST be well-documented**
- ? **Changes MUST be technically validated**
- ? **No "magic numbers" - all values must be explained**

## ?? **How Can I Contribute?**

### **Reporting Bugs**

**Before submitting a bug report:**
- Check if the issue already exists
- Collect sample data that reproduces the problem
- Note your Windows version and Visual Studio version

**Bug Report Template:**
```markdown
**Describe the bug:**
A clear description of what the bug is.

**To Reproduce:**
Steps to reproduce:
1. Go to tab '...'
2. Enter values '...'
3. Click '...'
4. See error

**Expected behavior:**
What you expected to happen.

**Actual behavior:**
What actually happened.

**Screenshots:**
If applicable, add screenshots.

**Environment:**
- OS: Windows [version]
- Visual Studio: [version]
- Engine Calculator: [version]
```

---

### **Suggesting Enhancements**

We love new ideas! But for scientific software, we need:

1. **Scientific justification**
2. **References to literature** (papers, books, standards)
3. **Use case examples**
4. **Expected behavior**

**Enhancement Template:**
```markdown
**Feature Description:**
What feature are you suggesting?

**Scientific Justification:**
Why is this technically important?

**References:**
- Paper/Book citation 1
- Paper/Book citation 2

**Use Cases:**
- Use case 1
- Use case 2

**Expected Behavior:**
What should happen?

**Alternatives Considered:**
Other approaches you thought about.
```

---

### **Contributing Code**

#### **General Rules:**

1. **Fork the repository**
2. **Create a feature branch** (`feature/your-feature-name`)
3. **Follow the coding style** (see below)
4. **Write clear commit messages**
5. **Test your changes**
6. **Submit a Pull Request**

#### **Coding Style:**

```cpp
// ? GOOD - Documented formula
double CalculateBMEP(double torque, double displacement) const {
    // BMEP (kPa) = (Torque [Nm] × 1200) / Displacement [L]
    // Reference: Heywood - "ICE Fundamentals" (1988), p.50
    // Derivation: 1200 = 2 × 2? × 1000 (for 4-stroke engines)
    
    return (torque * 1200.0) / displacement;
}

// ? BAD - No documentation, magic numbers
double calc(double t, double d) {
    return (t * 1200) / d;
}
```

#### **File Headers:**

Every file must have a header:

```cpp
// ============================================================================
// FileName.cpp - Brief Description
// Engine Calculator v1.0.0 - February 2026
// 
// Detailed description of the module
// References:
// - Heywood, J.B. "Internal Combustion Engine Fundamentals" (1988)
// - Blair, G.P. "Design and Simulation of Four-Stroke Engines" (1999)
// ============================================================================
```

#### **Comments:**

```cpp
// GOOD COMMENTS:

// Calculate swept volume per cylinder
// Formula: V = ? × (bore/2)² × stroke
double volume = M_PI * pow(bore / 2.0, 2.0) * stroke;

// Correct for volumetric efficiency
// Typical values: 80-95% (Heywood, Table 6.1)
double correctedVolume = volume * (ve / 100.0);

// BAD COMMENTS:

double v = 3.14 * r * r * s; // calculate volume
double cv = v * ve; // correction
```

---

### **Formula Contributions - CRITICAL!**

If you're adding/modifying a **formula**, you MUST provide:

#### 1. **Scientific Reference**
```cpp
// Reference: Heywood, J.B. "Internal Combustion Engine Fundamentals"
//            McGraw-Hill, 1988, ISBN: 0-07-028637-X
//            Chapter 2, Section 2.6, Page 50, Equation 2.15
```

#### 2. **Derivation (if applicable)**
```cpp
// DERIVATION:
// Starting from ideal gas law: PV = nRT
// For adiabatic compression: P?V?^? = P?V?^?
// Solving for P?:
//   P? = P? × (V?/V?)^?
//   P? = P? × CR^?
// Where ? ? 1.3 for air (Heywood, p.124)
```

#### 3. **Typical Values**
```cpp
// TYPICAL VALUES (Heywood, Table 2.2):
// - Naturally aspirated gasoline: 850-1050 kPa
// - Turbocharged gasoline: 1200-1700 kPa
// - Diesel: 1000-1600 kPa
```

#### 4. **Validation**
```cpp
// VALIDATION EXAMPLE:
// For a 2.0L engine producing 200 Nm @ 5000 RPM:
// BMEP = (200 × 1200) / 2.0 = 120,000 / 2.0 = 1200 kPa
// This matches Heywood's values for turbocharged engines ?
```

---

### **Pull Request Process**

1. **Update the README.md** if you're adding features
2. **Update docs/** with technical details
3. **Test compilation** (Debug and Release)
4. **No compiler warnings**
5. **Describe your changes** clearly

**PR Template:**
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Code refactoring
- [ ] Performance improvement

## Scientific Validation
- [ ] Formula validated against literature
- [ ] References added to code
- [ ] Typical values documented
- [ ] Test cases added

## Testing
- [ ] Tested in Debug mode
- [ ] Tested in Release mode
- [ ] No compiler warnings
- [ ] Manual testing performed

## References
- Paper/Book 1: ...
- Paper/Book 2: ...

## Checklist
- [ ] Code follows project style
- [ ] Comments are clear and technical
- [ ] Documentation updated
- [ ] No magic numbers
- [ ] All formulas have references
```

---

## ?? **Technical Standards**

### **Required References:**

When adding formulas, prefer references from:

1. **Textbooks:**
   - Heywood - "Internal Combustion Engine Fundamentals"
   - Blair - "Design and Simulation of Four-Stroke Engines"
   - Pulkrabek - "Engineering Fundamentals of ICE"
   - Stone - "Introduction to Internal Combustion Engines"

2. **SAE Papers:**
   - Must cite SAE number (e.g., SAE 2007-01-1464)
   - Available at: https://www.sae.org/publications/

3. **Standards:**
   - ASTM standards
   - ISO standards
   - EN standards

### **Forbidden:**

- ? Wikipedia as sole reference
- ? Forums/blogs without peer review
- ? "Common knowledge" without citation
- ? Unvalidated formulas

---

## ?? **Learning Resources**

New to engine thermodynamics? Start here:

1. **Heywood** - "Internal Combustion Engine Fundamentals" (1988)
   - The bible of IC engines
   - Comprehensive, mathematically rigorous

2. **Blair** - "Design and Simulation of Four-Stroke Engines" (1999)
   - Practical design-focused
   - Excellent for intake/exhaust tuning

3. **SAE Tutorials:**
   - https://www.sae.org/learn/

---

## ?? **Contact**

- **Issues:** Use GitHub Issues
- **Discussions:** Use GitHub Discussions
- **Email:** [Add your email if you want]

---

## ?? **Code of Conduct**

This project adheres to the Contributor Covenant Code of Conduct.
By participating, you are expected to uphold this code.

---

## ? **Attribution**

Contributors will be listed in:
- `CONTRIBUTORS.md`
- Release notes
- Documentation (for significant contributions)

---

**Thank you for helping make Engine Calculator better!** ??

