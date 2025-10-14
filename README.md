
# Syzygy - Language for Algebraic Specification and Computation

## Overview
Syzygy is a domain-specific language designed for formalizing and working with abstract algebraic structures. It treats programs as mathematical specifications where execution involves either computing with these structures or verifying their properties.

## Core Syntax Explained

### Basic Structure Definition
```
// Define a finite field with 7 elements
ring F7 = integers mod 7

// Create a 3-dimensional vector space over this field  
module VectorSpace = free_module(F7, 3)

// Declare basis vectors
generators {
    v1 = (1, 0, 0)
    v2 = (0, 1, 0)
    v3 = (0, 0, 1)
}

// Define linear dependencies (syzygies)
relations {
    2*v1 + 3*v2 == v3      // v3 is linearly dependent on v1, v2
    v1 - v2 == 0           // v1 equals v2 in this space
}
```

### Syntax Explanation:
- **`ring`**: Defines the base ring (integers, rationals, finite fields, polynomial rings)
- **`module`**: Creates a module over the specified ring. `free_module(ring, n)` creates Rⁿ
- **`generators`**: Declares the basis elements with their coordinate representations
- **`relations`**: Specifies linear dependencies between generators using equations
- **Equations use `==`** for mathematical equality, not assignment

## Complete Working Example

```
// Example: Working with polynomial rings and quotient spaces
ring PolyRing = Q[x,y]  // Polynomials in x,y over rationals

// Define an ideal and create quotient ring
ideal I = generated_by(x^2 + y^2 - 1, x*y - 1)
ring Circle = PolyRing / I  // Quotient by ideal I

// Create modules over this ring
module M = free_module(Circle, 2)
module N = free_module(Circle, 1)

generators {
    // Basis for M
    e1 = (1, 0) in M
    e2 = (0, 1) in M
    
    // Basis for N  
    f1 = (1) in N
}

// Define a module homomorphism
homomorphism projection: M -> N {
    projection(e1) = x * f1    // x from the quotient ring
    projection(e2) = y * f1    // y from the quotient ring
}

// Verify it's a valid homomorphism
verify homomorphism(projection)

// Compute kernel and image
kernel_vectors = ker(projection)
image_vectors = im(projection) 

// Create quotient module
quotient_module = M / kernel_vectors

// Check if the sequence is exact
exact_sequence = check_exactness(M, N, projection)
```

## Syntax Elements Breakdown

### 1. Ring Definitions
```
ring Z = integers                    // ℤ
ring Q = rationals                   // ℚ  
ring Fp = integers mod p            // 𝔽ₚ (p prime)
ring Poly = R[x,y,z]                // Polynomial ring
ring Quotient = BaseRing / ideal    // Quotient ring
```

### 2. Module Operations
```
module A = free_module(R, n)        // Rⁿ
module B = submodule(A, generators) // Submodule
module C = A / B                    // Quotient module
module D = A ⊕ B                    // Direct sum
module E = A ⊗ B                    // Tensor product
```

### 3. Homomorphisms
```
homomorphism name: Domain -> Codomain {
    name(generator₁) = expression₁
    name(generator₂) = expression₂
    // Must be linear: name(r*x + s*y) = r*name(x) + s*name(y)
}
```

### 4. Computations
```
k = ker(hom)                        // Kernel
i = im(hom)                         // Image  
c = coker(hom)                      // Cokernel
h = homology(complex)               // Homology groups
```

## Execution Model

When you run a Syzygy program:

1. **Parsing Phase**: Syntax is checked and abstract syntax tree built
2. **Type Checking**: Mathematical consistency verified (e.g., homomorphism properties)
3. **Symbolic Computation**: Expressions are reduced using algebraic rules
4. **Result Generation**: Output can be:
   - Concrete values (vectors, matrices)
   - Abstract structures (quotient spaces)
   - Proof certificates (theorem verifications)

## Key Design Principles

- **Mathematical Naturalness**: Code reads like mathematical text
- **Verification Built-in**: Type system ensures mathematical correctness
- **Symbolic Over Numeric**: Focuses on exact computation rather than approximation
- **Structure Preservation**: Operations maintain mathematical properties

This makes Syzygy particularly useful for educational purposes, research prototyping, and formal verification of algebraic arguments.
