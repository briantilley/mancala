package com.briantilley;

public final class TestDriver {
	public static void main(String[] args) {
        if(args.length != 2) {
            System.err.println("usage: <executable> <locations per player (excluding stores)> <initial # of tokens per location>\n");
            System.exit(1);
        }

		MancalaGame.generateSerialDecisionTree(new MancalaGameState(Integer.parseInt(args[0]), Integer.parseInt(args[1])), false, true);
	}
}
