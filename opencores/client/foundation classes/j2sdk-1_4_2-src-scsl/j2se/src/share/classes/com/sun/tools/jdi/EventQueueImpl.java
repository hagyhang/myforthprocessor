/*
 * @(#)EventQueueImpl.java	1.42 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;
import com.sun.jdi.event.EventQueue;
import com.sun.jdi.event.EventSet;

import java.util.*;

public class EventQueueImpl extends MirrorImpl implements EventQueue {

    /*
     * Note this is not a synchronized list. Iteration/update should be
     * protected through the 'this' monitor.
     */
    LinkedList eventSets = new LinkedList();

    TargetVM target;
    boolean closed = false;

    EventQueueImpl(VirtualMachine vm, TargetVM target) {
        super(vm);                   
        this.target = target;
        target.addEventQueue(this);
    }

    /*
     * Override superclass back to default equality
     */
    public boolean equals(Object obj) {
        return this == obj;
    }

    public int hashCode() {
        return System.identityHashCode(this);
    }

    synchronized void enqueue(EventSet eventSet) {
        eventSets.add(eventSet);
        notifyAll();    
    }

    synchronized int size() {
        return eventSets.size();
    }

    synchronized void close() {
        if (!closed) {
            closed = true; // OK for this the be first since synchronized

            // place VMDisconnectEvent into queue
            enqueue(new EventSetImpl(vm, 
                                     (byte)JDWP.EventKind.VM_DISCONNECTED));
        }
    }

    public EventSet remove() throws InterruptedException {
        return remove(0);
    }

    /**
     * Filter out events not for user's eyes.  
     * Then filter out empty sets.
     */
    public EventSet remove(long timeout) throws InterruptedException {
        if (timeout < 0) {
            throw new IllegalArgumentException("Timeout cannot be negative");
        }

        EventSet eventSet;
        do {
            EventSetImpl fullEventSet = removeUnfiltered(timeout);
            if (fullEventSet == null) {
                eventSet = null;  // timeout
                break;      
            }
            eventSet = fullEventSet.userFilter();
        } while (eventSet == null || eventSet.isEmpty());

        if ((eventSet != null) && (eventSet.suspendPolicy() == JDWP.SuspendPolicy.ALL)) {
            vm.notifySuspend();
        }

        return eventSet;
    }

    EventSet removeInternal() throws InterruptedException {
        EventSet eventSet;
        do {
            // Waiting forever, so removeUnfiltered() is never null
            eventSet = removeUnfiltered(0).internalFilter();
        } while (eventSet == null || eventSet.isEmpty());

        /*
         * Currently, no internal events are requested with a suspend
         * policy other than none, so we don't check for notifySuspend()
         * here. If this changes in the future, there is much 
         * infrastructure that needs to be updated. 
         */

        return eventSet;
    }

    private TimerThread startTimerThread(long timeout) {
        TimerThread thread = new TimerThread(timeout);
        thread.setDaemon(true);
        thread.start();
        return thread;
    }

    private boolean shouldWait(TimerThread timerThread) {
        return !closed && eventSets.isEmpty() &&
               ((timerThread == null) ? true : !timerThread.timedOut());
    }

    private EventSetImpl removeUnfiltered(long timeout) 
                                               throws InterruptedException {
        EventSetImpl eventSet = null;

        /* 
         * Make sure the VM has completed initialization before 
         * trying to build events.
         */
        vm.waitInitCompletion();

        synchronized(this) {
            if (!eventSets.isEmpty()) {
                /*
                 * If there's already something there, no need
                 * for anything elaborate.
                 */
                eventSet = (EventSetImpl)eventSets.removeFirst();
            } else {
                /*
                 * If a timeout was specified, create a thread to 
                 * notify this one when a timeout 
                 * occurs. We can't use the timed version of wait()
                 * because it is possible for multiple enqueue() calls
                 * before we see something in the eventSet queue
                 * (this is possible when multiple threads call 
                 * remove() concurrently -- not a great idea, but
                 * it should be supported). Even if enqueue() did a 
                 * notify() instead of notifyAll() we are not able to 
                 * use a timed wait because there's no way to distinguish
                 * a timeout from a notify.  That limitation implies a 
                 * possible race condition between a timed out thread
                 * and a notified thread.
                 */
                TimerThread timerThread = null;
                try {
                    if (timeout > 0) { 
                        timerThread = startTimerThread(timeout);
                    }
        
                    while (shouldWait(timerThread)) {
                        this.wait();
                    }
                } finally {
                    if ((timerThread != null) && !timerThread.timedOut()) {
                        timerThread.interrupt();
                    }
                }
    
                if (eventSets.isEmpty()) {
                    if (closed) {
                        throw new VMDisconnectedException();
                    }
                } else {
                    eventSet = (EventSetImpl)eventSets.removeFirst();
                }
            }
        }

        // The build is synchronized on the event set, don't hold
        // the queue lock.
        if (eventSet != null) {
            target.notifyDequeueEventSet();
            eventSet.build();
        }
        return eventSet;
    }

    private class TimerThread extends Thread {
        private boolean timedOut = false;
        private long timeout;

        TimerThread(long timeout) {
            super(vm.threadGroupForJDI(), "JDI Event Queue Timer");
            this.timeout = timeout;
        }

        boolean timedOut() {
            return timedOut;
        }

        public void run() {
            try {
                Thread.sleep(timeout);
                EventQueueImpl queue = EventQueueImpl.this;
                synchronized(queue) {
                    timedOut = true;
                    queue.notifyAll();
                }
            } catch (InterruptedException e) {
                // Exit without notifying
            }
        }
    }
}
