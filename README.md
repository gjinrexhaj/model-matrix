goal: build a customizable 3D cellular automata engine,
which uses string manipulation to programmatically
generate rulesets based on user-specified rules

Example rule
```
4/4/5/M
```

State 1 cell survives if it has 4 neighbors.
A Cell is born in an empty location if it has 4 neighbors.
5 Specifies that the cells can have a minimum state of 5.
M means use Moores neighbor counting system.