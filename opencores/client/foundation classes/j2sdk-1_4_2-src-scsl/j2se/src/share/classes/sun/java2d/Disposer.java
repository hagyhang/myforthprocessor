/*
 * @(#)Disposer.java	1.4 03/03/19
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.PhantomReference;
import java.lang.ref.WeakReference;
import java.util.Hashtable;

/**
 * This class is used for registering and disposing the native
 * data associated with java objects.
 *
 * The object can register itself by calling one of the addRecord
 * methods and providing either the pointer to the native disposal
 * method or a descendant of the DisposerRecord class with overridden
 * dispose() method.
 * 
 * When the object becomes unreachable, the dispose() method
 * of the associated DisposerRecord object will be called.
 *
 * @see DisposerRecord
 */
public class Disposer implements Runnable {
    private static final ReferenceQueue queue = new ReferenceQueue();
    private static final Hashtable records = new Hashtable();

    private static Disposer disposerInstance;
    public static final int WEAK = 0;
    public static final int PHANTOM = 1;
    public static int refType = PHANTOM;

    static {
	initIDs();
	String type = (String) java.security.AccessController.doPrivileged(
		new sun.security.action.GetPropertyAction("sun.java2d.reftype"));
	if (type != null) {
	    if (type.equals("weak")) {
		refType = WEAK;
		System.err.println("Using WEAK refs");
	    } else {
		refType = PHANTOM;
		System.err.println("Using PHANTOM refs");
	    }
	}
	disposerInstance = new Disposer();
	Thread t = new Thread(disposerInstance, "Java2D Disposer");
	t.setDaemon(true);
	t.setPriority(Thread.MAX_PRIORITY);
	t.start();

    }

    /**
     * Registers the object and the native data for later disposal.
     * @param obj Object to be registered
     * @param disposeMethod pointer to the native disposal method
     * @param pData pointer to the data to be passed to the
     *              native disposal method
     */
    public static void addRecord(DisposerTarget target, 
				 long disposeMethod, long pData) {
	disposerInstance.add(target, 
			     new DefaultDisposerRecord(disposeMethod, pData));
    }

    /**
     * Registers the object and the native data for later disposal.
     * @param obj Object to be registered
     * @param rec The assosicated DisposerRecord class
     * @see DisposerRecord
     */
    public static void addRecord(DisposerTarget target, DisposerRecord rec) {
	disposerInstance.add(target, rec);
    }

    synchronized void add(DisposerTarget target, DisposerRecord rec) {
	java.lang.ref.Reference ref;
	if (refType == PHANTOM) {
	    ref = 
		new PhantomReference(target.getDisposerReferent(), queue);
	} else {
	    ref = 
		new WeakReference(target.getDisposerReferent(), queue);
	}
        records.put(ref, rec);

	/* store the disposer record  for later use */
	target.setDisposerRecord(rec);
    }

    public void run() {
        while (true) {
            try {
                Object obj = queue.remove();
                DisposerRecord rec = (DisposerRecord)records.remove(obj);
		rec.dispose();
            } catch (Exception e) {
                System.out.println("Exception while removing reference: " + e);
		e.printStackTrace();
            }
        }
    }

    private static native void initIDs();
}
