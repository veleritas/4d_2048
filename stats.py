import numpy as np

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
    vals = []
    with open(fname, "r") as fin:
        for line in fin:
            line = line.rstrip("\n")
            vals.append(int(line))

    return vals

def stats(max_vals):
    tiles = np.zeros(CELLS, dtype=np.int32)
    for mval in max_vals:
        tiles[mval] += 1

    print("Performance stats for {} games:".format(len(max_vals)))

    idxs = np.flip(np.nonzero(tiles)[0], 0)
    for val, count in zip(idxs, tiles[idxs]):
        percent = count / len(max_vals) * 100
        print("Got 2^{0} ({1}) {2} times ({3:.2f}%)".format(val, 2**val, count, percent))

    return tiles

def main():
    vals = read_vals("res.txt")

    max_vals = []
    tile_counts = np.zeros(CELLS, dtype=np.int32)

    for state in vals:
        tiles = count_tiles(state)

        mtile = np.max(np.nonzero(tiles))
        max_vals.append(mtile)

    stats(max_vals)

if __name__ == "__main__":
    main()
