/*
 * @(#)PollingServer.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import java.io.*;
import java.net.*;
import java.lang.Byte;

/**
 * Simple Java "server" using the Poller class
 * to multiplex on incoming connections.  Note
 * that handoff of events, via linked Q is not
 * actually be a performance booster here, since
 * the processing of events is cheaper than
 * the overhead in scheduling/executing them.
 * Although this demo does allow for concurrency
 * in handling connections, it uses a rather
 * primitive "gang scheduling" policy to keep
 * the code simpler.
 */

public class PollingServer
{
  public final static int MAXCONN    = 10000;
  public final static int PORTNUM    = 4444;
  public final static int BYTESPEROP = 10;

  /**
   * This synchronization object protects access to certain
   * data (bytesRead,eventsToProcess) by concurrent Consumer threads.
   */
  private final static Object eventSync = new Object();

  private static InputStream[] instr = new InputStream[MAXCONN];
  private static int[] mapping = new int[65535];
  private static LinkedQueue linkedQ = new LinkedQueue();
  private static int bytesRead = 0;
  private static int bytesToRead;
  private static int eventsToProcess=0;

  public PollingServer(int concurrency) {
    Socket[] sockArr = new Socket[MAXCONN];
    long timestart, timestop;
    short[] revents = new short[MAXCONN];
    int[] fds = new int[MAXCONN];
    int bytes;
    Poller Mux;
    int serverFd;
    int totalConn=0;
    int connects=0;

    System.out.println ("Serv: Initializing port " + PORTNUM);
    try {

      ServerSocket skMain = new ServerSocket (PORTNUM);
      /*
       * Create the Poller object Mux, allow for up to MAXCONN
       * sockets/filedescriptors to be polled.
       */
      Mux = new Poller(MAXCONN);
      serverFd = Mux.add(skMain, Poller.POLLIN);

      Socket ctrlSock = skMain.accept();

      BufferedReader ctrlReader =
	new BufferedReader(new InputStreamReader(ctrlSock.getInputStream()));
      String ctrlString = ctrlReader.readLine();
      bytesToRead = Integer.valueOf(ctrlString).intValue();
      ctrlString = ctrlReader.readLine();
      totalConn = Integer.valueOf(ctrlString).intValue();

      System.out.println("Receiving " + bytesToRead + " bytes from " +
			 totalConn + " client connections");
      
      timestart = System.currentTimeMillis();

      /*
       * Start the consumer threads to read data.
       */
      for (int consumerThread = 0;
	   consumerThread < concurrency; consumerThread++ ) {
	new Consumer(consumerThread).start();
      }
      
      /*
       * Take connections, read Data
       */
      int numEvents=0;

      while ( bytesRead < bytesToRead ) {

	int loopWaits=0;
	while (eventsToProcess > 0) {
	  synchronized (eventSync) {
	    loopWaits++;
	    if (eventsToProcess <= 0) break;
	    try { eventSync.wait(); } catch (Exception e) {e.printStackTrace();};
	  }
	}
	if (loopWaits > 1)
	  System.out.println("Done waiting...loops = " + loopWaits +
			     " events " + numEvents +
			     " bytes read : " + bytesRead );

	if (bytesRead >= bytesToRead) break; // may be done!

	/*
	 * Wait for events
	 */
	numEvents = Mux.waitMultiple(100, fds, revents);
	synchronized (eventSync) {
	  eventsToProcess = numEvents;
	}
	/*
	 * Process all the events we got from Mux.waitMultiple
	 */
	int cnt = 0;
	while ( (cnt < numEvents) && (bytesRead < bytesToRead) ) {
	  int fd = fds[cnt];
	  
	  if (revents[cnt] == Poller.POLLIN) {
	    if (fd == serverFd) {
	      /*
	       * New connection coming in on the ServerSocket
	       * Add the socket to the Mux, keep track of mapping
	       * the fdval returned by Mux.add to the connection.
	       */
	      sockArr[connects] = skMain.accept();
	      instr[connects] = sockArr[connects].getInputStream();
	      int fdval = Mux.add(sockArr[connects], Poller.POLLIN);
	      mapping[fdval] = connects;
	      synchronized(eventSync) {
		eventsToProcess--; // just processed this one!
	      }
	      connects++;
	    } else {
	      /*
	       * We've got data from this client connection.
	       * Put it on the queue for the consumer threads to process.
	       */
	      linkedQ.put(new Integer(fd));
	    }
	  } else {
	    System.out.println("Got revents[" + cnt + "] == " + revents[cnt]);
	  }
	  cnt++;
	}
      }
      timestop = System.currentTimeMillis();
      System.out.println("Time for all reads (" + totalConn +
			 " sockets) : " + (timestop-timestart));

      // Tell the client it can now go away
      byte[] buff = new byte[BYTESPEROP];
      ctrlSock.getOutputStream().write(buff,0,BYTESPEROP);
      
      // Tell the cunsumer threads they can exit.
      for (int cThread = 0; cThread < concurrency; cThread++ ) {
	linkedQ.put(new Integer(-1));
      }
    } catch (Exception exc) { exc.printStackTrace(); }
  }

  /*
   * main ... just check if a concurrency was specified
   */
  public static void main (String args[])
  {
    int concurrency;

    if (args.length == 1)
      concurrency = java.lang.Integer.valueOf(args[0]).intValue();
    else
      concurrency = Poller.getNumCPUs() + 1;
    PollingServer server = new PollingServer(concurrency);
  }

  /*
   * This class is for handling the Client data.
   * The PollingServer spawns off a number of these based upon
   * the number of CPUs (or concurrency argument).
   * Each just loops grabbing events off the queue and
   * processing them.
   */
  class Consumer extends Thread {
    private int threadNumber;
    public Consumer(int i) { threadNumber = i; }

    public void run() {
      byte[] buff = new byte[BYTESPEROP];
      int bytes = 0;

      InputStream instream;
      while (bytesRead < bytesToRead) {
	try {
	  Integer Fd = (Integer) linkedQ.take();
	  int fd = Fd.intValue();
	  if (fd == -1) break; /* got told we could exit */

	  /*
	   * We have to map the fd value returned from waitMultiple
	   * to the actual input stream associated with that fd.
	   * Take a look at how the Mux.add() was done to see how
	   * we stored that.
	   */
	  int map = mapping[fd];
	  instream = instr[map];
	  bytes = instream.read(buff,0,BYTESPEROP);
	} catch (Exception e) { System.out.println(e.toString()); }

	if (bytes > 0) {
	  /*
	   * Any real server would do some synchronized and some
	   * unsynchronized work on behalf of the client, and
	   * most likely send some data back...but this is a
	   * gross oversimplification.
	   */
	  synchronized(eventSync) {
	    bytesRead += bytes;
	    eventsToProcess--;
	    if (eventsToProcess <= 0) {
	      eventSync.notify();
	    }
	  }
	}
      }
    }
  }
}
