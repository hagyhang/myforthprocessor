/*
 * @(#)ObjectReference.java	1.40 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

import java.util.List;
import java.util.Map;

/**
 * An object that currently exists in the target VM. An ObjectReference
 * mirrors only the object itself and is not specific to any 
 * {@link Field} or {@link LocalVariable} to which it is currently
 * assigned. An ObjectReference can
 * have 0 or more references from field(s) and/or variable(s). 
 * <p>
 * Any method on <code>ObjectReference</code> which directly or
 * indirectly takes <code>ObjectReference</code> as an parameter may throw 
 * {@link com.sun.jdi.VMDisconnectedException} if the target VM is 
 * disconnected and the {@link com.sun.jdi.event.VMDisconnectEvent} has been or is
 * available to be read from the {@link com.sun.jdi.event.EventQueue}.
 * <p>
 * Any method on <code>ObjectReference</code> which directly or
 * indirectly takes <code>ObjectReference</code> as an parameter may throw
 * {@link com.sun.jdi.VMOutOfMemoryException} if the target VM has run out of memory.
 * <p>
 * Any method on <code>ObjectReference</code> or which directly or indirectly takes
 * <code>ObjectReference</code> as parameter may throw 
 * {@link com.sun.jdi.ObjectCollectedException} if the mirrored object has been 
 * garbage collected.
 *
 * @author Robert Field
 * @author Gordon Hirsch
 * @author James McIlree
 * @since  1.3
 */
public interface ObjectReference extends Value
{
    /**
     * Gets the {@link ReferenceType} that mirrors the type
     * of this object. The type may be a subclass or implementor of the 
     * declared type of any field or variable which currently holds it.
     * For example, right after the following statement.
     * <p>
     * <code>Object obj = new String("Hello, world!");</code>
     * <p>
     * The ReferenceType of obj will mirror java.lang.String and not 
     * java.lang.Object.
     * <p>
     * The type of an object never changes, so this method will
     * always return the same ReferenceType over the lifetime of the
     * mirrored object.
     * <p>
     * The returned ReferenceType will be a {@link ClassType} or 
     * {@link ArrayType} and never an {@link InterfaceType}.
     * 
     * @return the {@link ReferenceType} for this object.
     */
    ReferenceType referenceType();

    /**
     * Gets the value of a given instance or static field in this object.
     * The Field must be valid for this ObjectReference;
     * that is, it must be from
     * the mirrored object's class or a superclass of that class.
     *
     * @param sig the field containing the requested value
     * @return the {@link Value} of the instance field.
     * @throws java.lang.IllegalArgumentException if the field is not valid for
     * this object's class.
     */
    Value getValue(Field sig);

    /**
     * Gets the value of multiple instance and/or static fields in this object.
     * The Fields must be valid for this ObjectReference;
     * that is, they must be from
     * the mirrored object's class or a superclass of that class.
     *
     * @param fields a list of {@link Field} objects containing the
     * requested values.
     * @return a Map of the requested {@link Field} objects with 
     * their {@link Value}.
     * @throws java.lang.IllegalArgumentException if any field is not valid for
     * this object's class.
     */
    Map getValues(List fields);

    /**
     * Sets the value of a given instance or static field in this object.
     * The {@link Field} must be valid for this ObjectReference; that is, 
     * it must be from the mirrored object's class or a superclass of that class.
     * If static, the field must not be final.
     * <p>
     * Object values must be assignment compatible with the field type
     * (This implies that the field type must be loaded through the
     * enclosing class's class loader). Primitive values must be 
     * either assignment compatible with the field type or must be
     * convertible to the field type without loss of information. 
     * See the <a href="http://java.sun.com/docs/books/jls/">
     * Java<sup><font size=-2>TM</font></sup> Language Specification</a>.
     * section 
     * <a href="http://java.sun.com/docs/books/jls/second_edition/html/conversions.doc.html#184206">5.2</a>
     * for more information on assignment
     * compatibility.
     * 
     * @param field the field containing the requested value    
     * @param value the new value to assign
     * @throws java.lang.IllegalArgumentException if the field is not valid for
     * this object's class.
     * @throws InvalidTypeException if the value's type does not match
     * the field's type.
     * @throws ClassNotLoadedException if 'value' is not null, and the field 
     * type has not yet been loaded through the appropriate class loader. 
     */
    void setValue(Field field, Value value) 
        throws InvalidTypeException, ClassNotLoadedException;

    /** Perform method invocation with only the invoking thread resumed */
    static final int INVOKE_SINGLE_THREADED = 0x1;
    /** Perform non-virtual method invocation */
    static final int INVOKE_NONVIRTUAL      = 0x2; 

    /**
     * Invokes the specified {@link Method} on this object in the 
     * target VM. The
     * specified method can be defined in this object's class,
     * in a superclass of this object's class, or in an interface
     * implemented by this object. The method may be a static method
     * or an instance method, but not a static initializer or constructor.
     * Use {@link ClassType#newInstance} to create a new object and 
     * run its constructor.
     * <p>
     * The method invocation will occur in the specified thread.
     * Method invocation can occur only if the specified thread
     * has been suspended by an event which occurred in that thread. 
     * Method invocation is not supported
     * when the target VM has been suspended through 
     * {@link VirtualMachine#suspend} or when the specified thread
     * is suspended through {@link ThreadReference#suspend}.
     * <p>
     * The specified method is invoked with the arguments in the specified 
     * argument list.  The method invocation is synchronous; this method
     * does not return until the invoked method returns in the target VM.
     * If the invoked method throws an exception, this method         
     * will throw an {@link InvocationException} which contains 
     * a mirror to the exception object thrown.                        
     * <p>
     * Object arguments must be assignment compatible with the argument type
     * (This implies that the argument type must be loaded through the
     * enclosing class's class loader). Primitive arguments must be 
     * either assignment compatible with the argument type or must be
     * convertible to the argument type without loss of information. 
     * See the <a href="http://java.sun.com/docs/books/jls/">
     * Java Language Specification</a>.
     * section 
     * <a href="http://java.sun.com/docs/books/jls/second_edition/html/conversions.doc.html#184206">5.2</a>
     * for more information on assignment compatibility.
     * <p>
     * By default, the method is invoked using dynamic lookup as 
     * documented in the
     * <a href="http://java.sun.com/docs/books/jls/">
     * Java Language Specification</a>
     * second edition, section 
     * <a href="http://java.sun.com/docs/books/jls/second_edition/html/expressions.doc.html#45606">15.12.4.4</a>; 
     * in particular, overriding based on the runtime type of the object 
     * mirrored by this {@link ObjectReference} will occur. This 
     * behavior can be changed by specifying the 
     * {@link #INVOKE_NONVIRTUAL} bit flag in the <code>options</code>
     * argument. If this flag is set, the specified method is invoked
     * whether or not it is overridden for this object's runtime type. 
     * The method, in this case, must not belong to an interface and 
     * must not be abstract. This option is useful for performing method 
     * invocations like those done with the <code>super</code> keyword in 
     * the Java programming language.
     * <p>
     * By default, all threads in the target VM are resumed while
     * the method is being invoked if they were previously
     * suspended by an event or by {@link VirtualMachine#suspend} or
     * {@link ThreadReference#suspend}. This is done to prevent the deadlocks
     * that will occur if any of the threads own monitors
     * that will be needed by the invoked method. It is possible that
     * breakpoints or other events might occur during the invocation.
     * Note, however, that this implicit resume acts exactly like
     * {@link ThreadReference#resume}, so if the thread's suspend
     * count is greater than 1, it will remain in a suspended state
     * during the invocation. By default, when the invocation completes,
     * all threads in the target VM are suspended, regardless their state 
     * before the invocation.  
     * <p>
     * The resumption of other threads during the invocation can be prevented 
     * by specifying the {@link #INVOKE_SINGLE_THREADED}
     * bit flag in the <code>options</code> argument; however, 
     * there is no protection against or recovery from the deadlocks 
     * described above, so this option should be used with great caution.
     * Only the specified thread will be resumed (as described for all
     * threads above). Upon completion of a single threaded invoke, the invoking thread
     * will be suspended once again. Note that any threads started during
     * the single threaded invocation will not be suspended when the 
     * invocation completes. 
     * <p>
     * If the target VM is disconnected during the invoke (for example, through
     * {@link VirtualMachine#dispose}) the method invocation continues.
     *
     * @param thread the thread in which to invoke.
     * @param method the {@link Method} to invoke.
     * @param arguments the list of {@link Value} arguments bound to the 
     * invoked method. Values from the list are assigned to arguments
     * in the order they appear in the method signature. 
     * @param options the integer bit flag options.
     * @return a {@link Value} mirror of the invoked method's return value.
     * @throws java.lang.IllegalArgumentException if the method is not
     * a member of this object's class, if the size of the argument list 
     * does not match the number of declared arguemnts for the method,
     * if the method is a constructor or static intializer, or
     * if {@link #INVOKE_NONVIRTUAL} is specified and the method is 
     * either abstract or an interface member.
     * @throws {@link InvalidTypeException} if any argument in the 
     * argument list is not assignable to the corresponding method argument
     * type.
     * @throws ClassNotLoadedException if any argument type has not yet been loaded
     * through the appropriate class loader. 
     * @throws IncompatibleThreadStateException if the specified thread has not
     * been suspended by an event.
     * @throws InvocationException if the method invocation resulted in
     * an exception in the target VM.
     * @throws InvalidTypeException If the arguments do not meet this requirement --
     *         Object arguments must be assignment compatible with the argument 
     *         type.  This implies that the argument type must be
     *         loaded through the enclosing class's class loader.  
     *         Primitive arguments must be either assignment compatible with the
     *         argument type or must be convertible to the argument type without loss  
     *         of information. See JLS section 5.2 for more information on assignment
     *         compatibility.
     */
    Value invokeMethod(ThreadReference thread, Method method, 
                       List arguments, int options) 
                                   throws InvalidTypeException,
                                          ClassNotLoadedException,
                                          IncompatibleThreadStateException,
                                          InvocationException;

    /**
     * Prevents garbage collection for this object. By default all 
     * {@link ObjectReference} values returned by JDI may be collected
     * at any time the target VM is running. A call to this method 
     * guarantees that the object will not be collected. 
     * {@link #enableCollection} can be used to allow collection once
     * again.
     * <p>
     * Calls to this method are counted. Every call to this method
     * requires a corresponding call to {@link #enableCollection} before
     * garbage collection is re-enabled.
     * <p>
     * Note that while the target VM is suspended, no garbage collection 
     * will occur because all threads are suspended. The typical 
     * examination of variables, fields, and arrays during the suspension
     * is safe without explicitly disabling garbage collection. 
     * <p>
     * This method should be used sparingly, as it alters the 
     * pattern of garbage collection in the target VM and,
     * consequently, may result in application behavior under the 
     * debugger that differs from its non-debugged behavior.
     */
    void disableCollection();

    /**
     * Permits garbage collection for this object. By default all 
     * {@link ObjectReference} values returned by JDI may be collected
     * at any time the target VM is running. A call to this method 
     * is necessary only if garbage collection was previously disabled
     * with {@link #disableCollection}.
     */
    void enableCollection();

    /**
     * Determines if this object has been garbage collected in the target
     * VM.
     *
     * @return <code>true</code> if this {@link ObjectReference} has been collected;
     * <code>false</code> otherwise.
     */
    boolean isCollected();

    /**
     * Returns a unique identifier for this ObjectReference.
     * It is guaranteed to be unique among all 
     * ObjectReferences from the same VM that have not yet been disposed. 
     * The guarantee applies as long
     * as this ObjectReference has not yet been disposed.
     *
     * @return a long unique ID
     */
    long uniqueID();

    /**
     * Returns a List containing a {@link ThreadReference} for 
     * each thread currently waiting for this object's monitor.
     * See {@link ThreadReference#currentContendedMonitor} for 
     * information about when a thread is considered to be waiting
     * for a monitor.
     * <p>
     * Not all target VMs support this operation. See
     * VirtualMachine#canGetMonitorInfo to determine if the 
     * operation is supported.
     *
     * @return a List of {@link ThreadReference} objects. The list
     * has zero length if no threads are waiting for the monitor.
     * @throws java.lang.UnsupportedOperationException if the
     * target VM does not support this operation.
     * @throws IncompatibleThreadStateException if any 
     * waiting thread is not suspended
     * in the target VM
     */
    List waitingThreads() throws IncompatibleThreadStateException;

    /**
     * Returns an {@link ThreadReference} for the thread, if any, 
     * which currently owns this object's monitor.
     * See {@link ThreadReference#ownedMonitors} for a definition
     * of ownership.
     * <p>
     * Not all target VMs support this operation. See
     * VirtualMachine#canGetMonitorInfo to determine if the 
     * operation is supported.
     *
     * @return the {@link ThreadReference} which currently owns the
     * monitor, or null if it is unowned.
     * 
     * @throws java.lang.UnsupportedOperationException if the
     * target VM does not support this operation.
     * @throws IncompatibleThreadStateException if the owning thread is 
     * not suspended in the target VM
     */
    ThreadReference owningThread() throws IncompatibleThreadStateException;
    
    /**
     * Returns the number times this object's monitor has been
     * entered by the current owning thread.
     * See {@link ThreadReference#ownedMonitors} for a definition
     * of ownership.
     * <p>
     * Not all target VMs support this operation. See
     * VirtualMachine#canGetMonitorInfo to determine if the 
     * operation is supported.
     *
     * @see #owningThread
     * @return the integer count of the number of entries.
     * 
     * @throws java.lang.UnsupportedOperationException if the
     * target VM does not support this operation.
     * @throws IncompatibleThreadStateException if the owning thread is 
     * not suspended in the target VM
     */
    int entryCount() throws IncompatibleThreadStateException;
    
    /**
     * Compares the specified Object with this ObjectReference for equality.
     * 
     * @return  true if the Object is an ObjectReference, if the 
     * ObjectReferences belong to the same VM, and if applying the
     * "==" operator on the mirrored objects in that VM evaluates to true.
     */
    boolean equals(Object obj);

    /**
     * Returns the hash code value for this ObjectReference.
     * 
     * @return the integer hash code
     */
    int hashCode();
}

