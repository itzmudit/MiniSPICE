# MiniSPICE

A small DC circuit solver in C. Enter resistors and a voltage source, and it finds every node voltage, plus the current, direction and power of every resistor. It uses nodal analysis and Gaussian elimination.

It's a single C file with no dependencies, and it runs in the terminal.

## Features

- Up to 10 nodes and 20 resistors
- Parallel resistors on the same node pair
- Shows the nodal matrix `[G | I]` before solving
- Gaussian elimination with partial pivoting
- Detects floating nodes instead of giving wrong answers
- Shows the real direction of every current (`N2 -> N0`)
- Values in readable units (`33.3333 mA`, `1.5000 kohm`)
- Power balance check: power supplied vs power used
- Save the report to `results.txt`
- Strict input: rejects `abc`, `10x`, negative values and out-of-range nodes

## Build

```bash
gcc minispice.c -o minispice -lm
./minispice
```

Windows (MinGW):

```bash
gcc minispice.c -o minispice.exe
minispice.exe
```

## Usage

```
  +--------------------------- MENU -----------------------------+
  |  1  Set resistors                                            |
  |  2  Set voltage source                                       |
  |  3  Solve                                                    |
  |  4  View results                                             |
  |  5  Save results to results.txt                              |
  |  6  Exit                                                     |
  +--------------------------------------------------------------+
  Nodes: 3   Resistors: 2   Source: 10.0000 V on node 1   Status: solved
```

- Node 0 is ground.
- The source's negative terminal is always on ground; you pick the node for the positive terminal.
- The program asks for every node pair once. Enter `0` if nothing is connected.
- After a value, choose `1` to add another resistor in parallel on the same pair, or `2` to move on.
- Changing the resistors or the source clears the old results, so you never see stale numbers.

## Example

10 V source on node 1, 100 Ω from node 1 to node 2, 200 Ω from node 2 to ground:

```
  N0 - N1  : 0
  N0 - N2  : 200
           1 = add in parallel, 2 = next pair : 2
  N1 - N2  : 100
           1 = add in parallel, 2 = done      : 2

  Positive terminal node [1-2] : 1
  Voltage (V)                  : 10
```

```
  NODE VOLTAGES
  ------------------------------------------------------------------
  N0     0.0000 V      (ground)
  N1    10.0000 V
  N2     6.6667 V

  RESISTORS
  ------------------------------------------------------------------
  ID    Flow          Resistance     Current        Power
  ------------------------------------------------------------------
  R1    N2 -> N0     200.0000 ohm    33.3333 mA    222.2222 mW
  R2    N1 -> N2     100.0000 ohm    33.3333 mA    111.1111 mW

  SOURCE
  ------------------------------------------------------------------
  Voltage           10.0000 V      (node 1 to ground)
  Current           33.3333 mA
  Power supplied   333.3333 mW
  Power used       333.3333 mW     (all resistors)
```

## How it works

1. Each resistor between nodes a and b adds its conductance g = 1/R to `G[a][a]` and subtracts it from `G[a][b]` (KCL). Ground is left out of the matrix.
2. The source node's row becomes `V[k] = volts`.
3. `G · V = I` is solved by Gaussian elimination with partial pivoting, then back substitution.
4. Resistor current is `(Va − Vb) / R` and power is `I²R`. The source current is found by KCL at the source node.

If a pivot is close to zero (below `1e-12`), some node has no path to ground, and the solve stops with an error.

## Limitations

- One voltage source only, with its negative terminal on ground
- No current sources, capacitors or inductors
- DC only

## Roadmap

- [ ] Current sources and multiple voltage sources (modified nodal analysis)
- [ ] Load circuits from a file
- [ ] AC analysis
- [ ] Transient analysis with capacitors and inductors

## Contributors

Mudit Jindal · Riyansh Pandey · Aniket Rastogi · Prakhar Mishra
