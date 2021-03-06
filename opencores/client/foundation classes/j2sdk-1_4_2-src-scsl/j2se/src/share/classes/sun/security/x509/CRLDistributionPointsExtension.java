/*
 * @(#)CRLDistributionPointsExtension.java	1.1 02/11/15
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.util.*;

import sun.security.util.DerOutputStream;
import sun.security.util.DerValue;

/**
 * Represent the CRL Distribution Points Extension (OID = 2.5.29.31).
 * <p>
 * The CRL distribution points extension identifies how CRL information
 * is obtained.  The extension SHOULD be non-critical, but the PKIX profile
 * recommends support for this extension by CAs and applications.
 * <p>
 * For PKIX, if the cRLDistributionPoints extension contains a
 * DistributionPointName of type URI, the following semantics MUST be
 * assumed: the URI is a pointer to the current CRL for the associated
 * reasons and will be issued by the associated cRLIssuer.  The
 * expected values for the URI conform to the following rules.  The
 * name MUST be a non-relative URL, and MUST follow the URL syntax and
 * encoding rules specified in [RFC 1738].  The name must include both
 * a scheme (e.g., "http" or "ftp") and a scheme-specific-part.  The
 * scheme- specific-part must include a fully qualified domain name or
 * IP address as the host.  As specified in [RFC 1738], the scheme
 * name is not case-sensitive (e.g., "http" is equivalent to "HTTP").
 * The host part is also not case-sensitive, but other components of
 * the scheme-specific-part may be case-sensitive. When comparing
 * URIs, conforming implementations MUST compare the scheme and host
 * without regard to case, but assume the remainder of the
 * scheme-specific-part is case sensitive.  Processing rules for other
 * values are not defined by this specification.  If the
 * distributionPoint omits reasons, the CRL MUST include revocations
 * for all reasons. If the distributionPoint omits cRLIssuer, the CRL
 * MUST be issued by the CA that issued the certificate.
 * <p>
 * The ASN.1 definition for this is:
 * <pre>
 * id-ce-cRLDistributionPoints OBJECT IDENTIFIER ::=  { id-ce 31 }
 *
 * cRLDistributionPoints ::= {
 *      CRLDistPointsSyntax }
 *
 * CRLDistPointsSyntax ::= SEQUENCE SIZE (1..MAX) OF DistributionPoint
 * </pre>
 * <p>
 * @author Anne Anderson
 * @author Andreas Sterbenz
 * @version 1.1, 11/15/02
 * @since 1.4.2
 * @see DistributionPoint
 * @see Extension
 * @see CertAttrSet
 */
public class CRLDistributionPointsExtension extends Extension
	implements CertAttrSet {

    /**
     * Identifier for this attribute, to be used with the
     * get, set, delete methods of Certificate, x509 type.
     */
    public static final String IDENT = 
    				"x509.info.extensions.CRLDistributionPoints";

    /**
     * Attribute name.
     */
    public static final String NAME = "CRLDistributionPoints";
    public static final String POINTS = "points";

    /**
     * The List of DistributionPoint objects.
     */
    private List distributionPoints;

    /**
     * Create a CRLDistributionPointsExtension from a List of
     * DistributionPoint; the criticality is set to false.
     *
     * @param distPoints the List of DistributionPoint
     * @throws IOException on error
     */
    public CRLDistributionPointsExtension(List distributionPoints) 
	    throws IOException {
        this.extensionId = PKIXExtensions.CRLDistributionPoints_Id;
        this.critical = false;
        this.distributionPoints = distributionPoints;
        encodeThis();
    }

    /**
     * Create the extension from the passed DER encoded value of the same.
     *
     * @param critical true if the extension is to be treated as critical.
     * @param value Array of DER encoded bytes of the actual value.
     * @exception IOException on error.
     */
    public CRLDistributionPointsExtension(Boolean critical, Object value)
	    throws IOException {
        this.extensionId = PKIXExtensions.CRLDistributionPoints_Id;
        this.critical = critical.booleanValue();

	if (!(value instanceof byte[])) {
	    throw new IOException("Illegal argument type");
	}
	
	extensionValue = (byte[])value;
        DerValue val = new DerValue(extensionValue);
	if (val.tag != DerValue.tag_Sequence) {
	    throw new IOException("Invalid encoding for " +
				  "CRLDistributionPointsExtension.");
	}
	distributionPoints = new ArrayList();
	while (val.data.available() != 0) {
	    DerValue seq = val.data.getDerValue();
	    DistributionPoint point = new DistributionPoint(seq);
	    distributionPoints.add(point);
	}
    }

    /**
     * Return the name of this attribute.
     */
    public String getName() {
        return NAME;
    }

    /**
     * Decode the extension from the InputStream.
     *
     * @param in the InputStream to unmarshal the contents from.
     * @exception IOException on decoding or validity errors.
     */
    public void decode(InputStream in) throws IOException {
        throw new IOException("Method not to be called directly.");
    }

    /**
     * Write the extension to the DerOutputStream.
     *
     * @param out the DerOutputStream to write the extension to.
     * @exception IOException on encoding errors.
     */
    public void encode(OutputStream out) throws IOException {
        DerOutputStream tmp = new DerOutputStream();
        if (this.extensionValue == null) {
	    this.extensionId = PKIXExtensions.CRLDistributionPoints_Id;
	    this.critical = false;
	    encodeThis();
	}
	super.encode(tmp);
	out.write(tmp.toByteArray());
    }

    /**
     * Set the attribute value.
     */
    public void set(String name, Object obj) throws IOException {
	if (name.equalsIgnoreCase(POINTS)) {
	    if (!(obj instanceof List)) {
	        throw new IOException("Attribute value should be of type List.");
	    }
	    distributionPoints = (List)obj;
	} else {
	    throw new IOException("Attribute name [" + name + 
                                "] not recognized by " +
				"CertAttrSet:CRLDistributionPointsExtension.");
	}
        encodeThis();
    }

    /**
     * Get the attribute value.
     */
    public Object get(String name) throws IOException {
	if (name.equalsIgnoreCase(POINTS)) {
	    return distributionPoints;
	} else {
	    throw new IOException("Attribute name [" + name + 
				"] not recognized by " +
				"CertAttrSet:CRLDistributionPointsExtension.");
	}
    }

    /**
     * Delete the attribute value.
     */
    public void delete(String name) throws IOException {
	if (name.equalsIgnoreCase(POINTS)) {
	    distributionPoints = new ArrayList();
	} else {
	    throw new IOException("Attribute name [" + name + 
			        "] not recognized by " +
				"CertAttrSet:CRLDistributionPointsExtension.");
	}
        encodeThis();
    }

    /**
     * Return an enumeration of names of attributes existing within this
     * attribute.
     */
    public Enumeration getElements() {
        AttributeNameEnumeration elements = new AttributeNameEnumeration();
        elements.addElement(POINTS);
	return elements.elements();
    }

     // Encode this extension value
    private void encodeThis() throws IOException {
        if (distributionPoints.isEmpty()) {
            this.extensionValue = null;
        } else {
	    DerOutputStream pnts = new DerOutputStream();
	    for (Iterator t = distributionPoints.iterator(); t.hasNext(); ) {
		DistributionPoint point = (DistributionPoint)t.next();
		point.encode(pnts);
	    }
	    DerOutputStream seq = new DerOutputStream();
	    seq.write(DerValue.tag_Sequence, pnts);
	    this.extensionValue = seq.toByteArray();
	}
    }

    /**
     * Return the extension as user readable string.
     */
    public String toString() {
        return super.toString() + "CRLDistributionPoints [\n  "
	       + distributionPoints + "]\n";
    }

}
