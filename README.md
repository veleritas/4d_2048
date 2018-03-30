# 4D 2048 Solver

Algorithmic solver for [4D 2048](https://huonw.github.io/2048-4D/) based
on [nneonneo's standard 2048 solver](https://github.com/nneonneo/2048-ai).

## Files

- `util.h`: Various helper functions and constant definitions.
- `move.h`: Define how the board changes after each move.
- `main.h`: The search algorithm.

### Helper files

- `multi.sh`: Run multiple instances of the algorithm on multiple cores.
- `quit_screens.sh`: Shut down `screen` instances used to run the algorithm.
- `stats.py`: Analyze the results of multiple runs to determine if performance
    changed.
