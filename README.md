# MiniSPICE
# MiniSPICE

A small DC circuit solver in C. Enter resistors and a voltage source, and it finds every node voltage, plus the current and power of every resistor. It uses nodal analysis and Gaussian elimination.

It's a single C file with no dependencies, and it runs in the terminal.

## Features

- Up to 10 nodes and 20 resistors
- Parallel resistors on the same node pair
- Shows the conductance matrix before solving
- Gaussian elimination with partial pivoting
- Detects floating nodes instead of giving wrong answers
- Source current and power, with a power balance check
- Save results to `results.txt`
- Rejects bad input (letters, negative values, invalid nodes)

## Build

```bash
gcc MiniSPICE.c -o minispice -lm
./minispice
```

Windows (MinGW):

```bash
gcc MiniSPICE.c -o minispice.exe
minispice.exe
```

## Usage

```
1. Set resistor values
2. Set voltage source
3. Solve circuit
4. View results
5. Save results to results.txt
6. Exit
```

- Node 0 is ground.
- The battery's negative terminal is always on ground; you choose the node for the positive terminal.
- The program asks for every node pair once. Enter `0` if there's no resistor between them.
- After a value, choose `1` to add another resistor in parallel on the same pair, or `2` to move on.

## Example

10 V source on Node 1, 100 Ω from Node 1 to Node 2, 200 Ω from Node 2 to ground:

```
Number of nodes (including ground): 3
Resistance between Node 0 and Node 1 (ohms): 0
Resistance between Node 0 and Node 2 (ohms): 200
Resistance between Node 1 and Node 2 (ohms): 100
Positive terminal node [1-2]: 1
Source voltage (volts): 10
```

```
Node 1  |     10.0000 V
Node 2  |      6.6667 V

R1   N0-N2   200.0000 ohm   -0.0333 A   0.2222 W
R2   N1-N2   100.0000 ohm    0.0333 A   0.1111 W

Source current             :     0.0333 A
Source power               :     0.3333 W
Total resistor dissipation :    0.3333 W
```

A negative current means it flows from the second node to the first.

### Larger test circuit

`hardest_circuit_input.txt` has a 10-node, 20-resistor network. To run it without typing everything in:

```bash
./minispice < hardest_circuit_input.txt
```

PowerShell:

```powershell
Get-Content hardest_circuit_input.txt | .\minispice.exe
```

## How it works

1. For each resistor between nodes a and b, its conductance g = 1/R is added to `G[a][a]` and subtracted from `G[a][b]` (KCL). Ground is left out of the matrix.
2. The battery node's row becomes `V[k] = volt`.
3. `G · V = I` is solved by Gaussian elimination with partial pivoting.
4. Resistor current is `(Va − Vb) / R` and power is `I²R`.

If a pivot is close to zero, the circuit has a node with no path to ground, and the solve stops with an error.

## Limitations

- One voltage source only, with its negative terminal at ground
- No current sources, capacitors or inductors
- DC only

## Roadmap

- [ ] Current sources and multiple voltage sources (modified nodal analysis)
- [ ] Load circuits from a file
- [ ] AC analysis
- [ ] Transient analysis with capacitors and inductors

## Contributors

Mudit Jindal · Riyansh Pandey · Aniket Rastogi · Prakhar Mishra
