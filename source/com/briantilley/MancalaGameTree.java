package com.briantilley;

import java.util.Arrays;
import java.util.stream.IntStream;

public class MancalaGameTree {

    private int nodeCount = 0;

    // node of the decision tree
    private class MancalaTurnNode {
        public MancalaGameState state;
        public MancalaTurnNode[] next;
        public MancalaTurnNode(MancalaGameState s, int locationsPerPlayer) {
            state = s;
            next = new MancalaTurnNode[locationsPerPlayer];
            nodeCount++;
        }
    }

    // number of playable spots on each player's side
    // number of tokens needed to guarantee a win (over hald of the total)
    private int locationsPerPlayer, winningThreshold;

    // starting point (fresh game)
    private MancalaTurnNode root;

    // start the tree (will only create a root at first)
    public MancalaGameTree(int locationsPerPlayer, int startingTokensPerLocation) {
        this.locationsPerPlayer = locationsPerPlayer;
        this.winningThreshold = locationsPerPlayer * startingTokensPerLocation;
        this.root = new MancalaTurnNode(new MancalaGameState(locationsPerPlayer, startingTokensPerLocation), locationsPerPlayer);
    }

    // public trigger for recursive call
    public void generate() {
        // start at the fresh game
        nodeCount = 1;
        createDecisionTree(root);
        System.out.println(nodeCount + " nodes created");
    }

    // generate entire decision tree
    private void createDecisionTree(MancalaTurnNode root) {
//        System.out.println(root.state);
        // base case (end of the game)
        // either side is empty or either player has more than half the tokens in their store
        if(IntStream.of(Arrays.copyOf(root.state.locations, root.state.locations.length / 2 - 1)).sum() <= 0 ||
                IntStream.of(Arrays.copyOfRange(root.state.locations, root.state.locations.length / 2, root.state.locations.length - 1)).sum() <= 0 ||
                root.state.locations[root.state.locations.length / 2 - 1] > winningThreshold ||
                root.state.locations[root.state.locations.length - 1] > winningThreshold) {
            root.next = null; // this signifies a finished game
//            System.out.println("***game over***\n");
            return;
        }

//        System.out.println("");

        // "Which player am I?"
        int currentPlayer = root.state.activePlayer ? 1 : 0;

        // "Who is my opponent?"
        int currentOpponent = (!root.state.activePlayer) ? 1 : 0;

        // "Where is my store?"
        int playerStore = currentPlayer * (locationsPerPlayer + 1) + locationsPerPlayer;

        // "Where is my opponent's store?"
        int opponentStore = currentOpponent * (locationsPerPlayer + 1) + locationsPerPlayer;

        // iterate through each possible move (like a player considering his options)
        for(int chosenMove = 0; chosenMove < locationsPerPlayer; chosenMove++) {
            // "What does the board look like?"
            MancalaGameState currentState = root.state.clone();

            // "Which location am I moving from?"
            int currentLocation = currentPlayer * (locationsPerPlayer + 1) + chosenMove;

            // "I'm going to pick up my tokens from this location"
            int tokensInHand = currentState.locations[currentLocation];
            currentState.locations[currentLocation] = 0;

            // "If there aren't any tokens here, I can't move."
            if(tokensInHand <= 0) continue;

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

            // store the generated game state
            root.next[chosenMove] = new MancalaTurnNode(currentState, locationsPerPlayer);

            // generate the new state's decision subtree
            createDecisionTree(root.next[chosenMove]);
        }
    }
}
