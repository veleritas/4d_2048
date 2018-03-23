import numpy as np
import pandas as pd
import argparse

CELLS = 16

def draw_board(state):
    assert isinstance(state, int)

    res = str(hex(state))[2:] # map to hex, remove leading 0x
    res = res[::-1] # flip for actual grid order
    fin = "\n".join(res[i:i+4] for i in range(0, 16, 4)) # add newlines

    print(fin)

def count_tiles(state):
    # Count the number of 0 to f tiles

    vals = np.zeros(CELLS, dtype=np.int32)
    for i in range(CELLS):
        vals[state & 0xF] += 1
        state = state >> 4;

    return vals

def read_vals(fname):
    return pd.read_csv(fname, sep=" ", names=["endstate", "moves"]);

def max_size(state):
    # Given an endgame state, determine the largest achieved tile

    tiles = count_tiles(state)
    return np.max(np.nonzero(tiles))

def main():
    parser = argparse.ArgumentParser(
        description="Determine endgame result statistics."
    )

    parser.add_argument("fname", type=str, help="Result file name to process")
    args = parser.parse_args()

#-------------------------------------------------------------------------------

    data = (read_vals(args.fname)
        .assign(max_tile = lambda df: df["endstate"].map(max_size))
    )

    ngames = len(data)
    print("Performance stats for {} games:".format(ngames))

    for i in range(11, 16):
        num = len(data.query("max_tile >= @i"))
        percent = num / ngames * 100

        print("Got to 2^{0} ({1}) {2} times ({3:.2f}%)".format(i, 2**i, num, percent))

    print()

    print("Largest endgame tiles:")
    res = (data["max_tile"]
        .value_counts()
        .sort_index()
        .to_frame()
        .assign(percent = lambda df: df["max_tile"] / ngames * 100)
    )
    print(res)

if __name__ == "__main__":
    main()
