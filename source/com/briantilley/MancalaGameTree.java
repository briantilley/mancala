package com.briantilley;

import java.util.Deque;
import java.util.LinkedList;
import java.util.NoSuchElementException;

public class MancalaGameTree {

    // node of the decision tree
    private class MancalaTurnNode {
        public MancalaGameState state;
        public MancalaTurnNode[] next;
        public MancalaTurnNode(MancalaGameState s, int locationsPerPlayer) {
            state = s;
            next = new MancalaTurnNode[locationsPerPlayer];
        }

        // store active player plus all values of the board in one byte each
        // if next is null, this state an end to the game and 0xff follows the usual information
        public byte[] toBytes() {
            byte[] byteRepresentation;

            // handle game-end nodes
            if(next != null)
                byteRepresentation = new byte[state.locations.length + 1];
            else {
                byteRepresentation = new byte[state.locations.length + 2];
                byteRepresentation[byteRepresentation.length - 1] = (byte)0xff;
            }

            // fill standard info
            byteRepresentation[0] = (byte)(state.activePlayer ? 1 : 0);
            for(int i = 0; i < state.locations.length; ++i)
                byteRepresentation[i + 1] = (byte)state.locations[i];

            return byteRepresentation;
        }
    }

    // this is for file-based creation of the Mancala tree
    private class MancalaStackFrame {
        public MancalaGameState state;
        public int currentMove, currentPlayer, playerStore, opponentStore;
        public MancalaStackFrame(MancalaGameState s) {
            state = s;
            currentMove = 0;
            currentPlayer = state.activePlayer ? 1 : 0;
            playerStore = currentPlayer * (locationsPerPlayer + 1) + locationsPerPlayer;
            opponentStore = ((!state.activePlayer) ? 1 : 0) * (locationsPerPlayer + 1) + locationsPerPlayer;
        }
    }

    // number of playable spots on each player's side
    // number of tokens needed to guarantee a win (over hald of the total)
    private int locationsPerPlayer, winningThreshold;

    // starting point (fresh game)
    private MancalaTurnNode root;

    // sum of elements in ints from a to b excluding b
    private int intSum(int[] ints, int a, int b) throws IllegalArgumentException {
        if(a < 0 || b < 0)
            throw new IllegalArgumentException("invalid range sum index given");

        int accumulator = 0;
        int finalIdx = Math.min(b, ints.length);
        for(int i = a; i < finalIdx; ++i)
            accumulator += ints[i];

        return accumulator;
    }

    // sum of first n elements of ints
    private int intSum(int[] ints, int n) { return intSum(ints, 0, n); }

    // start the tree (will only create a root at first)
    public MancalaGameTree(int locationsPerPlayer, int startingTokensPerLocation) {
        this.locationsPerPlayer = locationsPerPlayer;
        this.winningThreshold = locationsPerPlayer * startingTokensPerLocation;
        this.root = new MancalaTurnNode(new MancalaGameState(locationsPerPlayer, startingTokensPerLocation), locationsPerPlayer);
    }

    // public trigger to create all possible states
    public void generate() {
        printDecisionTree();
    }

    // give the next state by performing the move specified
    // return null if move is invalid
    private MancalaGameState doTurn(MancalaGameState previousState, int chosenMove, int currentPlayer, int playerStore, int opponentStore) {
        // "Which location am I moving from?"
        int currentLocation = currentPlayer * (locationsPerPlayer + 1) + chosenMove;

        // "Are there tokens in this spot to be moved?
        if(previousState.locations[currentLocation] <= 0)
            return null;

        // clone a copy of the board to modify
        MancalaGameState currentState = previousState.clone();

        // "I'm going to pick up my tokens from this location"
        int tokensInHand = currentState.locations[currentLocation];
        currentState.locations[currentLocation] = 0;

        while(tokensInHand > 0) {
            // move one spot forward (stay on the board)
            currentLocation++; currentLocation %= currentState.locations.length;
            // "If I'm not at my opponent's store, I'll deposit one token from my hand."
            if(currentLocation != opponentStore) {
                currentState.locations[currentLocation]++;
                tokensInHand--;
            }
        }

        // "Unless I stopped at my own store, it's my opponent's turn."
        if(currentLocation != playerStore) {
            currentState.activePlayer = !currentState.activePlayer;
            // "If I stopped at a previously empty location on my side, I store my opponent's tokens across from my stopping point"
            if((currentState.locations[currentLocation] == 1) && (currentLocation < playerStore) && (currentLocation >= ((opponentStore + 1) % currentState.locations.length))) {
                int oppositeLocation = currentState.locations.length - currentLocation - 2;
                currentState.locations[playerStore] += 1 + currentState.locations[oppositeLocation];
                currentState.locations[oppositeLocation] = currentState.locations[currentLocation] = 0;
            }
        }

        // return the generated game state
        return currentState;
    }

    // the game ends when either player has stored more than half the pieces or either side empties
    private boolean isEndState(MancalaGameState s, int locationsPerPlayer, int winningThreshold) {
        return s.locations[locationsPerPlayer] > winningThreshold ||
                s.locations[2 * locationsPerPlayer + 1] > winningThreshold ||
                intSum(s.locations, locationsPerPlayer) <= 0 ||
                intSum(s.locations, locationsPerPlayer + 1, 2 * locationsPerPlayer + 1) <= 0;
    }

    // print the would-be game tree in pre-order traversal fashion, don't store in memory
    private void printDecisionTree() {
        // hold the entire path of states leading up to current state
        Deque<MancalaStackFrame> previousStates = new LinkedList<>();
        previousStates.addFirst(new MancalaStackFrame(root.state));

        // print initial state
        System.out.println(root.state);

        // metrics
        long maxMoveCount = Long.MIN_VALUE, minMoveCount = Long.MAX_VALUE, avgMoveCountAccumulator = 0, avgMoveCountDivisor = 0;

        while(true) {
            // grab a frame to start doing turns from
            // end when there are no more
            MancalaStackFrame previousFrame;
            try { previousFrame = previousStates.removeFirst(); }
            catch(NoSuchElementException e) { break; }

            // if this state is the end of a game path, forget it and move up
            if(isEndState(previousFrame.state, locationsPerPlayer, winningThreshold)) {
                // metrics
                maxMoveCount = Math.max(maxMoveCount, previousStates.size());
                minMoveCount = Math.min(minMoveCount, previousStates.size());
                avgMoveCountAccumulator += previousStates.size();
                avgMoveCountDivisor++;

                continue;
            }

            // find the next valid state that follows
            MancalaGameState currentState = null;
            while(currentState == null && previousFrame.currentMove < locationsPerPlayer) {
                currentState = doTurn(previousFrame.state, previousFrame.currentMove, previousFrame.currentPlayer, previousFrame.playerStore, previousFrame.opponentStore);
                previousFrame.currentMove++;
            }

            // if we hit a valid move
            if(currentState != null) {
                // output the state
//                System.out.println(currentState);

                // put the previous frame back on the stack
                previousStates.addFirst(previousFrame);

                // put the just-found state onto the stack
                previousStates.addFirst(new MancalaStackFrame(currentState));
            }
        }

        System.err.println("longest game: " + maxMoveCount + " moves\nshortest game: " + minMoveCount + " moves");
        System.err.println("average game: " + (int)(100.0 * (double)avgMoveCountAccumulator / (double)avgMoveCountDivisor + 0.5) / 100.0 + " moves");
        System.err.println("total games: " + avgMoveCountDivisor);
    }
}
