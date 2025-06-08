# Analysis of Algorithms I (MC458) - Project 0

- [Requirements](./Enunc.pdf)
- [Instructions](./Instructions.pdf)
- [Report](./relatorio/main.pdf)

---

## Mobile Equilibrium Checker

A tiny C/C++ program that reads the textual description of a mobile (the hanging-toy kind), decides if every lever in the structure is balanced, and prints a single character:

* `S` - the mobile *is* in equilibrium
* `N` - the mobile *is not* in equilibrium

The algorithm is a straight divide-and-conquer on the tree that the input encodes (mobile → internal node, weight → leaf) and runs in θ(n) time, where *n* is the number of weights. Details, proofs and complexity analysis live in **Report.pdf**.

---

## Building

```sh
make
```

### Running Tests

```sh
cd testes
bash run_tests.sh
```
