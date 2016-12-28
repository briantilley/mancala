package com.briantilley;

import java.util.Arrays;

public final class MancalaGameState implements Cloneable{

    // number of tokens in each spot on the board
    private int[] board;
    private int store0index, store1index;
    private int currentMoveIndex = 0;

    // useful game parameters
    private int locationsPerPlayer, winningThreshold;

    // which player's turn is it?
    private boolean player = false;

    // convenience method
    // checks whether the range of values in the specified array is empty
    private boolean rangeIsEmpty(int[] values, int begin, int end) {
        for(int i = begin; i < end; ++i)
            if(values[i] != 0) return false;
        return true;
    }

    // set up a new game
    // locationsPerPlayer should include only playable locations, not stores (6 in standard game)
    // locations are organized with first player's playable spots, their store, then repeat for second player
    public MancalaGameState(int locationsPerPlayer, int startingTokensPerLocation) throws IllegalArgumentException {
        // ensure unsigned bytes are sufficient for eventual storage
        if(2 * locationsPerPlayer * startingTokensPerLocation > 255)
            throw new IllegalArgumentException("total number of tokens must be 255 or less");

        // create and fill a board
        board = new int[2 * (locationsPerPlayer + 1)];
        Arrays.fill(board, startingTokensPerLocation);

        // empty each player's store
        board[locationsPerPlayer] = 0;
        board[2 * locationsPerPlayer + 1] = 0;

        // calculate game parameters
        store0index = locationsPerPlayer;
        store1index = board.length - 1;
        this.locationsPerPlayer = locationsPerPlayer;
        winningThreshold = startingTokensPerLocation * locationsPerPlayer;
    }

    // deep copy
    @Override
    public MancalaGameState clone() {
        // get a shallow copy
        MancalaGameState copy = null;
        try { copy = (MancalaGameState)super.clone(); }
        catch (CloneNotSupportedException e) { e.printStackTrace(); }

        // duplicate the board in a totally new array
        copy.board = new int[board.length];
        System.arraycopy(this.board, 0, copy.board, 0, this.board.length);

        // increment the move index
        this.currentMoveIndex++;

        return copy;
    }

    // modify this instance by doing a turn in the game
    void doTurn() {
        // determine absolute initial position on board
        int currentLocation = player ? (locationsPerPlayer + 1 + currentMoveIndex) : currentMoveIndex;

        // this is essentially a new state after turn is done
        currentMoveIndex = 0;

        // check for invalid move
        if(currentLocation > board.length || 0 == board[currentLocation])
            return;

        // pick up tokens from currentLocation
        int tokensInHand = board[currentLocation];
        board[currentLocation] = 0;

        // record parameters
        int playerStore, opponentStore;
        playerStore = player ? store1index : store0index;
        opponentStore = !player ? store1index : store0index;

        // step forward and drop tokens in place
        while(0 < tokensInHand)
        {
            // one step forward, stay on the board
            currentLocation = (currentLocation + 1) % board.length;

            // if not over opponent's store
            if(currentLocation != opponentStore)
            {
                // drop one token into location
                board[currentLocation]++;
                tokensInHand--;
            }
        }

        // player gets to go again if they stopped on their own store
        if(currentLocation != playerStore)
        {
            // if the player stopped on a previously empty location on their side,
            // they capture opponent's tokens opposite that location
            if(board[currentLocation] == 1 &&
                    currentLocation < playerStore &&
                    (!player || currentLocation > opponentStore))
            {
                int oppositeLocation = board.length - 2 - currentLocation;
                board[playerStore] += 1 + board[oppositeLocation];
                board[oppositeLocation] = board[currentLocation] = 0;
            }

            // give opponent a turn
            player = !player;
        }
    }

    // get location from which the next move could occur
    void walkToNextMove() {
        // check the active player's side, repeatedly incrementing nextStartPos
        int offset = player ? (locationsPerPlayer + 1) : 0;
        while(currentMoveIndex < locationsPerPlayer) {
            if(0 != board[offset + currentMoveIndex])
                break;
            currentMoveIndex++;
        }
    }

    // returns true if there are no more moves to be done
    public boolean isExhausted() { return currentMoveIndex >= locationsPerPlayer; }

    // return true if this state is the end of a game
    public boolean isGameOver()
    {
        // either store has more than half the tokens or either side is empty
        return board[store0index] > winningThreshold ||
                board[store1index] > winningThreshold ||
                rangeIsEmpty(board, 0, store0index) ||
                rangeIsEmpty(board, store0index + 1, store1index);
    }

    // output a pretty format board for the console
    @Override
    public String toString() {
        int i; // counter

        String output = " ______ ______ ";
        for(i = 0; i < locationsPerPlayer; ++i)
            output += "______ ";

        output += "\n|      |      |";
        for(i = 0; i < locationsPerPlayer; ++i)
            output += "      |";

        output += "\n|      |";
        for(i = 0; i < locationsPerPlayer; ++i)
            output += String.format("  %2d  |", board[board.length - 2 - i]);
        output += "      |";
        if(player)
            output += " 1";

        output += String.format("\n|  %2d  |", board[board.length - 1]);
        for(i = 0; i < locationsPerPlayer; ++i)
            output += "______|";
        output += String.format("  %2d  |", board[locationsPerPlayer]);

        output += "\n|      |      |";
        for(i = 0; i < locationsPerPlayer; ++i)
            output += "      |";

        output += "\n|      |";
        for(i = 0; i < locationsPerPlayer; ++i)
            output += String.format("  %2d  |", board[i]);
        output += "      |";
        if(!player)
            output += " 0";

        output += "\n|______|______|";
        for(i = 0; i < locationsPerPlayer; ++i)
            output += "______|";

        return output + "\n\n";
    }

    // convenience
    void print() { System.out.print(this); }
}
