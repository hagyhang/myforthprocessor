/*
 * @(#)PKCS10Attributes.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.pkcs;

import java.io.IOException;
import java.io.OutputStream;
import java.security.cert.CertificateException;
import java.util.Collection;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Hashtable;

import sun.security.util.*;

/**
 * This class defines the PKCS10 attributes for the request.
 * The ASN.1 syntax for this is:
 * <pre>
 * Attributes ::= SET OF Attribute
 * </pre>
 *
 * @author Amit Kapoor
 * @author Hemma Prafullchandra
 * @version 1.18
 * @see PKCS10
 * @see PKCS10Attribute
 */
public class PKCS10Attributes implements DerEncoder {

    private Hashtable map = new Hashtable(3);

    /**
     * Default constructor for the PKCS10 attribute.
     */
    public PKCS10Attributes() { }

    /**
     * Create the object from the array of PKCS10Attribute objects.
     *
     * @param attrs the array of PKCS10Attribute objects.
     */
    public PKCS10Attributes(PKCS10Attribute[] attrs) {
        for (int i = 0; i < attrs.length; i++) {
            map.put(attrs[i].getAttributeId().toString(), attrs[i]);
	}
    }

    /**
     * Create the object, decoding the values from the passed DER stream.
     * The DER stream contains the SET OF Attribute.
     *
     * @param in the DerInputStream to read the attributes from.
     * @exception IOException on decoding errors.
     */
    public PKCS10Attributes(DerInputStream in) throws IOException {
        DerValue[] attrs = in.getSet(3, true);

	if (attrs == null)
            throw new IOException("Illegal encoding of attributes");
        for (int i = 0; i < attrs.length; i++) {
            PKCS10Attribute attr = new PKCS10Attribute(attrs[i]);
            map.put(attr.getAttributeId().toString(), attr);
	}
    }

    /**
     * Encode the attributes in DER form to the stream.
     *
     * @param out the OutputStream to marshal the contents to.
     * @exception IOException on encoding errors.
     */
    public void encode(OutputStream out) throws IOException {
        derEncode(out);
    }

    /**
     * Encode the attributes in DER form to the stream.
     * Implements the <code>DerEncoder</code> interface.
     *
     * @param out the OutputStream to marshal the contents to.
     * @exception IOException on encoding errors.
     */
    public void derEncode(OutputStream out) throws IOException {
	// first copy the elements into an array
        Collection allAttrs = map.values();
        PKCS10Attribute[] attribs =
           (PKCS10Attribute[])allAttrs.toArray(new PKCS10Attribute[map.size()]);

	DerOutputStream attrOut = new DerOutputStream();
	attrOut.putOrderedSetOf(DerValue.createTag(DerValue.TAG_CONTEXT,
                                                   true, (byte)0), 
			        attribs);
        out.write(attrOut.toByteArray());
    }

    /**
     * Set the attribute value.
     */
    public void setAttribute(String name, Object obj) {
        map.put(name, obj);
    }

    /**
     * Get the attribute value.
     */
    public Object getAttribute(String name) {
        return map.get(name);
    }

    /**
     * Delete the attribute value.
     */
    public void deleteAttribute(String name) {
        map.remove(name);
    }

    /**
     * Return an enumeration of names of attributes existing within this
     * attribute.
     */
    public Enumeration getElements() {
        return (map.elements());
    }

    /**
     * Return a Collection of attributes existing within this
     * PKCS10Attributes object.
     */
    public Collection getAttributes() {
        return (Collections.unmodifiableCollection(map.values()));
    }

    /**
     * Compares this PKCS10Attributes for equality with the specified
     * object. If the <code>other</code> object is an
     * <code>instanceof</code> <code>PKCS10Attributes</code>, then
     * all the entries are compared with the entries from this.
     *   
     * @param other the object to test for equality with this PKCS10Attributes.
     * @return true iff all the entries match that of the Other,
     * false otherwise.
     */  
    public boolean equals(Object other) {
        if (this == other)
            return true;
        if (!(other instanceof PKCS10Attributes))
            return false;
 
        Collection othersAttribs = ((PKCS10Attributes)other).getAttributes();
        Object[] objs = othersAttribs.toArray();
        int len = objs.length;
        if (len != map.size())
            return false;
        PKCS10Attribute thisAttr, otherAttr;
        String key = null;
        for (int i=0; i < len; i++) {
            if (objs[i] instanceof PKCS10Attribute) {
                otherAttr = (PKCS10Attribute)objs[i];
                key = otherAttr.getAttributeId().toString();

               if (key == null)
                   return false;
               thisAttr = (PKCS10Attribute)map.get(key);
               if (thisAttr == null)
                   return false;
               if (! thisAttr.equals(otherAttr))
                   return false;
            }
        }
        return true;
    }
 
    /**
     * Returns a hashcode value for this PKCS10Attributes.
     *   
     * @return the hashcode value.
     */  
    public int hashCode() {
        return map.hashCode();
    }

    /**
     * Returns a string representation of this <tt>PKCS10Attributes</tt> object
     * in the form of a set of entries, enclosed in braces and separated
     * by the ASCII characters "<tt>,&nbsp;</tt>" (comma and space).
     * <p>Overrides the <tt>toString</tt> method of <tt>Object</tt>.
     *   
     * @return  a string representation of this PKCS10Attributes.
     */
    public String toString() {
        String s = map.size() + "\n" + map.toString();
        return s;
    }
}