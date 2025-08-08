
# Blockchain P2P Network Simulator

This project implements a simulator to analyze **Selfish Mining** and **Eclipse Attack** strategies in a blockchain peer-to-peer (P2P) network. The simulator allows you to study the impact of these attacks on network security, consensus, and block propagation.

## Features
- Simulates a blockchain P2P network with configurable parameters
- Models honest and malicious nodes (including selfish miners and eclipse attackers)
- Supports analysis of block propagation, fork rates, and attack effectiveness
- Generates visualization results for network activity and attack impact

## How to Run
1. **Build the project:**
	```sh
	make
	```
2. **Run the simulation:**
	```sh
	make run
	```
	- Default parameters are set in the `Makefile`. Visualization output is created in `/vis`.
3. **Clean build artifacts:**
	```sh
	make clean
	```

## Simulation Parameters
Set these in the `Makefile` or pass them to the binary:
- `-n` : Number of nodes
- `-T` : Total simulation time
- `-b` : Mean block interarrival time
- `-w` : Timeout time
- `-m` : Percentage of malicious nodes

## Folder Structure
- `include/` : Header files
- `src/` : Source files
- `obj/` : Object files (created during build)
- `bin/` : Compiled binary
- `vis/` : Visualization results


## File Descriptions
### Header Files (`include/`)
- `block.h` : Defines the structure and properties of a blockchain block.
- `event.h` : Declares event types and event handling for the simulation.
- `network.h` : Declares network topology and communication logic.
- `node.h` : Declares node behavior, including honest and malicious nodes.
- `simulator.h` : Declares the main simulator class and simulation control logic.
- `transaction.h` : Declares transaction structure and handling.
- `utils.h` : Utility functions and helpers used throughout the project.

### Source Files (`src/`)
- `main.cpp` : Entry point for the simulator; parses arguments and starts the simulation.
- `network.cpp` : Implements network communication and topology logic.
- `node.cpp` : Implements node behavior, including mining and attack strategies.
- `simulator.cpp` : Implements the main simulation loop and event processing.
- `utils.cpp` : Implements utility functions used by other modules.

## Notes
- Visualization results are generated in `/vis` after running the simulation.