# Initial-x64-series
optimisation for assembly

spec: 2,2 GHz Intel Core i7

## Multiplication (Integer without taking care of carry)

```
====MULTIPLICATION BY 2=========
[MUL 1] Use 'imul' instruction
[MUL 1] Time : 2.511815
[MUL 2] Use 'mov, add' instruction
[MUL 2] Time : 2.491328
[MUL 3] Use 'shl' instruction
[MUL 3] Time : 2.455770
[MUL 4] Use 'lea' instruction
[MUL 4] Time : 2.471328
====MULTIPLICATION BY 5=========
[MUL 5] Use 'imul' instruction
[MUL 5] Time : 2.590941
[MUL 6] Use 'lea' instruction
[MUL 6] Time : 2.526823
```
