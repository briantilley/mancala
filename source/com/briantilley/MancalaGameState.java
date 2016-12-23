package com.briantilley;

import java.util.Arrays;

public class MancalaGameState implements Cloneable{

    // number of tokens in each spot on the board
    public int[] locations;

    // which player's turn is it?
    public boolean activePlayer;

    // no-arg constructor
    public MancalaGameState() {}

    // set up a new game
    // locationsPerPlayer should include only playable locations, not stores (6 in standard game)
    // locations are organized with first player's playable spots, their store, then repeat for second player
    public MancalaGameState(int locationsPerPlayer, int startingTokensPerLocation) {
        // initialize members ("create a board")
        locations = new int[2 * (locationsPerPlayer + 1)];
        Arrays.fill(locations, startingTokensPerLocation);

        // empty each player's store
        locations[locationsPerPlayer] = 0;
        locations[2 * locationsPerPlayer + 1] = 0;
    }

    // deep copy
    @Override
    public MancalaGameState clone() {
        MancalaGameState copy = new MancalaGameState();
        copy.locations = Arrays.copyOf(this.locations, this.locations.length);
        copy.activePlayer = this.activePlayer;
        return copy;
    }

    // print in a somewhat viewable format
    @Override
    public String toString() {
        int[] farSide = Arrays.copyOfRange(locations, locations.length / 2, locations.length);
        for(int i = 0; i < farSide.length / 2; ++i) {
            int temp = farSide[i];
            farSide[i] = farSide[farSide.length - 1 - i];
            farSide[farSide.length - 1 - i] = temp;
        }
        return Arrays.toString(farSide) + "\n   " + Arrays.toString(Arrays.copyOf(locations, locations.length / 2));
    }
}
