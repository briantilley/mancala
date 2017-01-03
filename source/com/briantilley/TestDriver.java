package com.briantilley;

import java.io.*;
import java.util.Deque;
import java.util.LinkedList;

public final class TestDriver {
	public static void main(String[] args) throws IOException {
        if(!(args.length == 2 || args.length == 3)) {
            System.err.println("usage: <executable> <locations per player (excluding stores)> <initial # of tokens per location> (\"r\"/\"w\")\n");
            System.exit(1);
        }

        int locationsPerPlayer = Integer.parseInt(args[0]), tokensPerLocation = Integer.parseInt(args[1]);
        if(args.length == 2 || args[2].equals("r")) read(locationsPerPlayer, tokensPerLocation);
        else write(locationsPerPlayer, tokensPerLocation);

    }

    public static void read(int locationsPerPlayer, int tokensPerLocation) {
        // find a file fitting the given parameters
        String inputFilename = locationsPerPlayer + "l" + tokensPerLocation + "t." + MancalaGame.TREE_FILENAME_EXTENSION;
        File inputFile = new File(inputFilename);
        for(int i = 1; !inputFile.exists(); ++i)
            inputFile = new File(locationsPerPlayer + "l" + tokensPerLocation + "t(" + i + ")." + MancalaGame.TREE_FILENAME_EXTENSION);

        // run the file
        MancalaGame.printTreeFromFile(inputFile);
    }

    public static void write (int locationsPerPlayer, int tokensPerLocation) {
        // open a file for final output
        String outputFilename = locationsPerPlayer + "l" + tokensPerLocation + "t." + MancalaGame.TREE_FILENAME_EXTENSION;
        File outputFile = new File(outputFilename);
        for(int i = 1; outputFile.exists(); ++i)
            outputFile = new File(locationsPerPlayer + "l" + tokensPerLocation + "t(" + i + ")." + MancalaGame.TREE_FILENAME_EXTENSION);

        // get number of threads on the system
        int threadCount = Runtime.getRuntime().availableProcessors();

        // create a work queue
        Deque<MancalaGameState> initialStates = new LinkedList<>();

        // create an output queue
        // how to ensure output is ordered?

        // fill the work queue with at least 2 tasks per thread

        // create a thread pool

        // assign work

        // wait

        // assemble partial results

//        // generate output
//        try(OutputStream outputStream = new BufferedOutputStream(new FileOutputStream(outputFile), 1 << 18 )) {
//            MancalaGameState first = new MancalaGameState(locationsPerPlayer, tokensPerLocation);
//            MancalaGame.generateSerialDecisionTree(outputStream, first, false, true);
//        }
//        catch (IOException e) { e.printStackTrace(); System.exit(1); }
    }
}
