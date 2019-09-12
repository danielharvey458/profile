#!/usr/bin/python3

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
import sys
import argparse

def plot_dimensions (num_plots):
    # Compute number of rows and columns
    # such that there are:
    #
    #  - No more than 9 plots on the page
    #  - Number of rows differs from number of cols
    #    by no more than 1
    #  - There are the fewest unused plots
    nrows = 1
    ncols = 1

    while nrows * ncols < num_plots:

        if ncols > nrows:
            nrows += 1
        elif nrows >= ncols:
            ncols += 1

    return (nrows, ncols)

def parse_args ():

    parser = argparse.ArgumentParser ()
    parser.add_argument ("-o", help="Output file")
    parser.add_argument ("input", help="Input file, use '-' for stdin")

    return parser.parse_args ()

if __name__ == "__main__":

    args = parse_args ()

    if args.input == '-':
        fh = sys.stdin
    else:
        fh = open (args.input)

    data = pd.read_csv (fh, index_col=[0,1]).T
    types = sorted (set (data.columns.get_level_values (0)))
    events = sorted (set (map (lambda x : x.split ("/")[0],
                               data[types[0]].index)))

    rows, cols = plot_dimensions (len (events))

    fig, axes = plt.subplots (rows, cols, figsize=(10, 10))

    def sca (idx):
        if len (events) > 1:
            plt.sca (axes.ravel ()[idx])

    for idx, event in enumerate (events):
        plot_idx = idx % (rows * cols)
        sca (plot_idx)
        for tp in types:
            subframe = data[tp]
            events = subframe.index
            sizes = subframe.columns
            plt.errorbar (data[tp].T.index,
                          data[tp].T[event + '/mu'],
                          data[tp].T[event + '/sigma'],
                          label=tp)
        plt.title (event)
        plt.grid (True)
        if plot_idx == 0:
            plt.legend (loc='best', framealpha=0.0, fontsize=10)

    plt.show ()
