/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbTgsRep.java	1.6 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 *  IBM Confidential
 *  OCO Source Materials
 *  Licensed Materials - Property of IBM
 * 
 *  (C) Copyright IBM Corp. 1999 All Rights Reserved.
 * 
 *  The source code for this program is not published or otherwise divested of
 *  its trade secrets, irrespective of what has been deposited with the U.S.
 *  Copyright Office.
 * 
 *  Copyright 1997 The Open Group Research Institute.  All rights reserved.
 * ===========================================================================
 * 
 */

package sun.security.krb5;

import sun.security.krb5.internal.*;
import sun.security.util.*;
import java.io.IOException;

/**
 * This class encapsulates a TGS-REP that is sent from the KDC to the 
 * Kerberos client.
 */
public class KrbTgsRep extends KrbKdcRep {
    private TGSRep rep;
    private Credentials creds;
    private Ticket secondTicket;

    KrbTgsRep(byte[] ibuf, Credentials asCreds, KrbTgsReq tgsReq) 
	throws KrbException, IOException {
	DerValue ref = new DerValue(ibuf); 
	TGSReq req = tgsReq.getMessage();
	TGSRep rep = null;
	try {
	    rep = new TGSRep(ref);
    	} catch (Asn1Exception e) {
	    e.printStackTrace();
	    rep = null;
	    KRBError err = new KRBError(ref);
	    String eText = null; // pick up text sent by the server (if any)
	    if (err.eText != null && err.eText.length() > 0) { 
    		if (err.eText.charAt(err.eText.length() - 1) == 0)
    		    eText = err.eText.substring(0, err.eText.length() - 1);
    		else
    		    eText = err.eText;
	    }
	    KrbException ke; 
	    if (eText == null) {
		// no text sent from server
		ke = new KrbException(err.errorCode); 
	    } else { 
		// override default text with server text
		ke = new KrbException(err.errorCode, eText); 
	    }
	    ke.initCause(e);
	    throw ke;
	}
        byte[] enc_tgs_rep_bytes = rep.encPart.decrypt(asCreds.key);
        byte[] enc_tgs_rep_part = rep.encPart.reset(enc_tgs_rep_bytes, true);
	ref = new DerValue(enc_tgs_rep_part);
	EncTGSRepPart enc_part = new EncTGSRepPart(ref);
	rep.ticket.sname.setRealm(rep.ticket.realm);
	rep.encKDCRepPart = enc_part;
	
	check(req, rep);
	
	creds = new Credentials(rep.ticket,
				req.reqBody.cname,
				rep.ticket.sname,
				enc_part.key,
				enc_part.flags,
				enc_part.authtime,
				enc_part.starttime,
				enc_part.endtime,
				enc_part.renewTill,
				enc_part.caddr
				);
	this.rep = rep;
	this.creds = creds;
	this.secondTicket = tgsReq.getSecondTicket();
    }

    /**
     * Return the credentials that were contained in this KRB-TGS-REP.
     */
    public Credentials getCreds() {
	return creds;
    }

    sun.security.krb5.internal.ccache.Credentials setCredentials() {	
	return new sun.security.krb5.internal.ccache.Credentials(rep, secondTicket);
    }
}
