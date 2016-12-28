package com.briantilley;

import java.util.Deque;
import java.util.LinkedList;
import java.util.NoSuchElementException;

public final class MancalaGame {

    // generate moves in the game
    public static void generateSerialDecisionTree(MancalaGameState initial) { generateSerialDecisionTree(initial, true, true); }
    public static void generateSerialDecisionTree(MancalaGameState initial, boolean printStates, boolean collectMetrics) {
        // game metrics
        long gameCount = 0, moveCount = 0;

        // store all the states leading up to the one being worked on except ones without any more moves
        Deque<MancalaGameState> pendingStates = new LinkedList<>();

        // print the starting state
        if(printStates)
            initial.print();

        // hold onto the state for which moves are being generated
        MancalaGameState referenceState = initial;

        // pre-order style generation of a serialized tree
        while(true) {
            // we're guaranteed to have a valid move to do
            // ensure next check is from a different position
            MancalaGameState newState = referenceState.clone();
            newState.doTurn();

            // print the state
            if(printStates)
                newState.print();

            if(collectMetrics)
                moveCount++;

            // if the starting frame has been exhausted
            referenceState.walkToNextMove();
            if(referenceState.isExhausted())
            {
                // let go of the reference state
                referenceState = null;
            }

            // if the new state has moves to be done
            if(!newState.isGameOver())
            {
                // if there were still more moves
                if(null != referenceState)
                {
                    // push the frame to the stack
                    pendingStates.addFirst(referenceState);
                }

                // hold the new state
                referenceState = newState;

                // start the new state at a valid move
                referenceState.walkToNextMove();
            }
            else
            {
                // a game just ended
                if(collectMetrics)
                    gameCount++;
            }

            // if we exhausted a frame and didn't find a state to continue down
            if(null == referenceState)
            {
                // get a state to work on
                // we're all done when the queue can't return an element
                try { referenceState = pendingStates.removeFirst(); }
                catch(NoSuchElementException e) { break; }
            }
        }

	    if(collectMetrics)
            System.out.print(String.format(" total games: %d\ntotal states: %d\n", gameCount, moveCount + 1));
    }
}
