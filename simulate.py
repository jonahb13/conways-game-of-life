import argparse
import numpy as np
import matplotlib.pyplot as plt

PYPLOT_DELAY = 0.0001 # Delay between showing the results of each iteration of the simulation


def main():
    # plt.ion()
    # plt.show()
    parser = argparse.ArgumentParser(description='Simulates Conways game of life given an input .npy file')
    parser.add_argument('a.npy', type=argparse.FileType('rb'), help='File to simulate')

    # Parse the command-line arguments
    args = parser.parse_args()
    
    data = np.load(getattr(args, 'a.npy'))
    print(data)


if __name__ == '__main__':
    main()