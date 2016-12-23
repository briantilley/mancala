package com.briantilley;

public final class TestDriver {
	public static void main(String[] args) {
		MancalaGameTree game = new MancalaGameTree(Integer.parseInt(args[0]), Integer.parseInt(args[1]));
		game.generate();
	}
}
