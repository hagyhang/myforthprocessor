/*
 * @(#)PolicyChecker.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.util.Set;
import java.util.HashSet;
import java.util.Collection;
import java.util.Collections;
import java.util.Vector;
import java.util.Iterator;
import java.io.IOException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.cert.PKIXCertPathChecker;
import java.security.cert.CertPathValidatorException;
import java.security.cert.PolicyNode;
import sun.security.util.Debug;
import sun.security.x509.CertificatePoliciesExtension;
import sun.security.x509.PolicyConstraintsExtension;
import sun.security.x509.PolicyMappingsExtension;
import sun.security.x509.CertificatePolicyMap;
import sun.security.x509.PKIXExtensions;
import sun.security.x509.PolicyInformation;
import sun.security.x509.X509CertImpl;
import sun.security.x509.InhibitAnyPolicyExtension;

/**
 * PolicyChecker is a <code>PKIXCertPathChecker</code> that checks policy
 * information on a PKIX certificate, namely certificate policies, policy
 * mappings, policy constraints and policy qualifiers.
 *
 * @version 	1.8 01/23/03
 * @since	1.4
 * @author	Yassir Elley
 */
class PolicyChecker extends PKIXCertPathChecker {
 
    private final Set initPolicies;
    private final int certPathLen;
    private final boolean expPolicyRequired;
    private final boolean polMappingInhibited;
    private final boolean anyPolicyInhibited;
    private final boolean rejectPolicyQualifiers;
    private PolicyNodeImpl rootNode;
    private int explicitPolicy;
    private int policyMapping;
    private int inhibitAnyPolicy;
    private int certIndex;
    private Set supportedExts;

    private static final Debug debug = Debug.getInstance("certpath");
    static final String ANY_POLICY = "2.5.29.32.0";

    /**
     * Constructs a Policy Checker.
     * 
     * @param initialPolicies Set of initial policies
     * @param certPathLen length of the certification path to be checked
     * @param expPolicyRequired true if explicit policy is required
     * @param polMappingInhibited true if policy mapping is inhibited
     * @param anyPolicyInhibited true if the ANY_POLICY OID should be inhibited
     * @param rejectPolicyQualifiers true if pol qualifiers are to be rejected
     * @param rootNode the initial root node of the valid policy tree
     */
    PolicyChecker(Set initialPolicies, int certPathLen, 
	boolean expPolicyRequired, boolean polMappingInhibited, 
	boolean anyPolicyInhibited, boolean rejectPolicyQualifiers,
	PolicyNodeImpl rootNode) throws CertPathValidatorException
    {
	if (initialPolicies.isEmpty()) {
	    // if no initialPolicies are specified by user, set 
	    // initPolicies to be anyPolicy by default
	    this.initPolicies = new HashSet(1);
	    this.initPolicies.add(ANY_POLICY);
	} else {
	    this.initPolicies = new HashSet(initialPolicies);
	}
	this.certPathLen = certPathLen;
	this.expPolicyRequired = expPolicyRequired;
	this.polMappingInhibited = polMappingInhibited;
        this.anyPolicyInhibited = anyPolicyInhibited;
	this.rejectPolicyQualifiers = rejectPolicyQualifiers;
	this.rootNode = rootNode;
	init(false);
    }
    
    /**
     * Initializes the internal state of the checker from parameters
     * specified in the constructor
     *
     * @param forward a boolean indicating whether this checker should
     * be initialized capable of building in the forward direction
     * @exception CertPathValidatorException Exception thrown if user
     * wants to enable forward checking and forward checking is not supported.
     */
    public void init(boolean forward) throws CertPathValidatorException {
	if (forward) {
	    throw new CertPathValidatorException
	    				("forward checking not supported");
	}

	certIndex = 1;
	explicitPolicy = (expPolicyRequired ? 0 : certPathLen + 1);
        policyMapping = (polMappingInhibited ? 0 : certPathLen + 1);
	inhibitAnyPolicy = (anyPolicyInhibited ? 0 : certPathLen + 1);
    }

    /**
     * Checks if forward checking is supported. Forward checking refers
     * to the ability of the PKIXCertPathChecker to perform its checks
     * when presented with certificates in the forward direction (from
     * target to anchor).
     *
     * @return true if forward checking is supported, false otherwise
     */
    public boolean isForwardCheckingSupported() {
	return false;
    }

    /**
     * Gets an immutable Set of the OID strings for the extensions that
     * the PKIXCertPathChecker supports (i.e. recognizes, is able to 
     * process), or null if no extensions are
     * supported. All OID strings that a PKIXCertPathChecker might 
     * possibly be able to process should be included.
     *
     * @return the Set of extensions supported by this PKIXCertPathChecker,
     * or null if no extensions are supported
     */
    public Set getSupportedExtensions() {
	if (supportedExts == null) {
	    supportedExts = new HashSet();
	    supportedExts.add(PKIXExtensions.CertificatePolicies_Id.toString());
	    supportedExts.add(PKIXExtensions.PolicyMappings_Id.toString());
	    supportedExts.add(PKIXExtensions.PolicyConstraints_Id.toString());
            supportedExts.add(PKIXExtensions.InhibitAnyPolicy_Id.toString());
	    supportedExts = Collections.unmodifiableSet(supportedExts);
	}
	return supportedExts;
    }

    /**
     * Performs the policy processing checks on the certificate using its 
     * internal state. 
     *
     * @param cert the Certificate to be processed
     * @param unresCritExts the unresolved critical extensions
     * @exception CertPathValidatorException Exception thrown if
     * the certificate does not verify.
     */
    public void check(Certificate cert, Collection unresCritExts) 
	throws CertPathValidatorException
    {
	// now do the policy checks
	checkPolicy((X509Certificate) cert);

	if (unresCritExts != null && !unresCritExts.isEmpty()) {
	    unresCritExts.remove(PKIXExtensions.CertificatePolicies_Id.toString());
	    unresCritExts.remove(PKIXExtensions.PolicyMappings_Id.toString());
	    unresCritExts.remove(PKIXExtensions.PolicyConstraints_Id.toString());
            unresCritExts.remove(PKIXExtensions.InhibitAnyPolicy_Id.toString());
	}
    }

    /**
     * Internal method to run through all the checks.
     *
     * @param currCert the certificate to be processed
     * @exception CertPathValidatorException Exception thrown if
     * the certificate does not verify
     */
    private void checkPolicy(X509Certificate currCert) 
	throws CertPathValidatorException
    {
	String msg = "certificate policies";
	if (debug != null) {
	    debug.println("PolicyChecker.checkPolicy() ---checking " + msg 
		+ "...");
	    debug.println("PolicyChecker.checkPolicy() certIndex = " 
		+ certIndex);
	    debug.println("PolicyChecker.checkPolicy() BEFORE PROCESSING: "
		+ "explicitPolicy = " + explicitPolicy);
	    debug.println("PolicyChecker.checkPolicy() BEFORE PROCESSING: "
		+ "policyMapping = " + policyMapping);
	    debug.println("PolicyChecker.checkPolicy() BEFORE PROCESSING: "
		+ "inhibitAnyPolicy = " + inhibitAnyPolicy);
	    debug.println("PolicyChecker.checkPolicy() BEFORE PROCESSING: "
		+ "policyTree = " + rootNode);
	}

	X509CertImpl currCertImpl = null;
	try {
	    currCertImpl = X509CertImpl.toImpl(currCert);
	} catch (CertificateException ce) {
	    throw new CertPathValidatorException(ce);
	}

        boolean finalCert = (certIndex == certPathLen);

	rootNode = processPolicies(certIndex, initPolicies, explicitPolicy, 
	    policyMapping, inhibitAnyPolicy, rejectPolicyQualifiers, rootNode, 
	    currCertImpl, finalCert);

        if (!finalCert) {
            explicitPolicy = mergeExplicitPolicy(explicitPolicy, currCertImpl,
                                                 finalCert);
            policyMapping = mergePolicyMapping(policyMapping, currCertImpl);
            inhibitAnyPolicy = mergeInhibitAnyPolicy(inhibitAnyPolicy, 
						     currCertImpl);
        }
	
	certIndex++;

	if (debug != null) {
	    debug.println("PolicyChecker.checkPolicy() AFTER PROCESSING: "
		+ "explicitPolicy = " + explicitPolicy);
	    debug.println("PolicyChecker.checkPolicy() AFTER PROCESSING: "
		+ "policyMapping = " + policyMapping);
	    debug.println("PolicyChecker.checkPolicy() AFTER PROCESSING: "
		+ "inhibitAnyPolicy = " + inhibitAnyPolicy);
	    debug.println("PolicyChecker.checkPolicy() AFTER PROCESSING: "
		+ "policyTree = " + rootNode);
	    debug.println("PolicyChecker.checkPolicy() " + msg + " verified");
	}
    }

    /**
     * Merges the specified explicitPolicy value with the 
     * requireExplicitPolicy field of the <code>PolicyConstraints</code>
     * extension obtained from the certificate. An explicitPolicy
     * value of -1 implies no constraint.
     *
     * @param explicitPolicy an integer which indicates if a non-null
     * valid policy tree is required
     * @param currCert the Certificate to be processed
     * @param finalCert a boolean indicating whether currCert is
     * the final cert in the cert path
     * @return returns the new explicitPolicy value
     * @exception CertPathValidatorException Exception thrown if an error 
     * occurs
     */
    static int mergeExplicitPolicy(int explicitPolicy, X509CertImpl currCert,
        boolean finalCert) throws CertPathValidatorException
    {
        if ((explicitPolicy > 0) && !X509CertImpl.isSelfIssued(currCert)) {
            explicitPolicy--;
        }

        try {
            PolicyConstraintsExtension polConstExt 
		= currCert.getPolicyConstraintsExtension();
	    if (polConstExt == null)
		return explicitPolicy;
            int require = ((Integer) 
		polConstExt.get(PolicyConstraintsExtension.REQUIRE)).intValue();
	    if (debug != null) {
	        debug.println("PolicyChecker.mergeExplicitPolicy() "
                   + "require Index from cert = " + require);
	    }
            if (!finalCert) {
                if (require != -1) {
                    if ((explicitPolicy == -1) || (require < explicitPolicy)) {
                        explicitPolicy = require;
                    } 
                } 
            } else {
		if (require == 0)
                    explicitPolicy = require;
            }
        } catch (Exception e) {
	    if (debug != null) {
	        debug.println("PolicyChecker.mergeExplicitPolicy "
                              + "unexpected exception");
	        e.printStackTrace();
	    }
	    throw new CertPathValidatorException(e);
        }	    
        
        return explicitPolicy; 
    }

    /**
     * Merges the specified policyMapping value with the 
     * inhibitPolicyMapping field of the <code>PolicyConstraints</code>
     * extension obtained from the certificate. A policyMapping
     * value of -1 implies no constraint.
     *
     * @param policyMapping an integer which indicates if policy mapping
     * is inhibited
     * @param currCert the Certificate to be processed
     * @return returns the new policyMapping value
     * @exception CertPathValidatorException Exception thrown if an error 
     * occurs
     */    
    static int mergePolicyMapping(int policyMapping, X509CertImpl currCert)
        throws CertPathValidatorException 
    {
        if ((policyMapping > 0) && !X509CertImpl.isSelfIssued(currCert)) {
            policyMapping--;
        }
        
        try {
            PolicyConstraintsExtension polConstExt 
		= currCert.getPolicyConstraintsExtension();
	    if (polConstExt == null)
		return policyMapping;

            int inhibit = ((Integer) 
		polConstExt.get(PolicyConstraintsExtension.INHIBIT)).intValue();
            if (debug != null)
                debug.println("PolicyChecker.mergePolicyMapping() "
                    + "inhibit Index from cert = " + inhibit);
                
            if (inhibit != -1) {
                if ((policyMapping == -1) || (inhibit < policyMapping)) {
                    policyMapping = inhibit;
                } 
            } 
        } catch (Exception e) {
	    if (debug != null) {
	        debug.println("PolicyChecker.mergePolicyMapping "
                              + "unexpected exception");
	        e.printStackTrace();
	    }
	    throw new CertPathValidatorException(e);
        }	    
        
        return policyMapping; 
    }

    /**
     * Merges the specified inhibitAnyPolicy value with the 
     * SkipCerts value of the InhibitAnyPolicy
     * extension obtained from the certificate.
     *
     * @param inhibitAnyPolicy an integer which indicates whether
     * "any-policy" is considered a match
     * @param currCert the Certificate to be processed
     * @return returns the new inhibitAnyPolicy value
     * @exception CertPathValidatorException Exception thrown if an error 
     * occurs
     */    
    static int mergeInhibitAnyPolicy(int inhibitAnyPolicy, 
        X509CertImpl currCert) throws CertPathValidatorException
    {
        if ((inhibitAnyPolicy > 0) && !X509CertImpl.isSelfIssued(currCert)) {
            inhibitAnyPolicy--;
        }
        
        try {
            InhibitAnyPolicyExtension inhAnyPolExt = (InhibitAnyPolicyExtension)
	    	currCert.getExtension(PKIXExtensions.InhibitAnyPolicy_Id);
	    if (inhAnyPolExt == null)
		return inhibitAnyPolicy;

            int skipCerts = ((Integer) 
		inhAnyPolExt.get(InhibitAnyPolicyExtension.SKIP_CERTS)).intValue();
            if (debug != null)
                debug.println("PolicyChecker.mergeInhibitAnyPolicy() "
                    + "skipCerts Index from cert = " + skipCerts);
                
            if (skipCerts != -1) {
                if (skipCerts < inhibitAnyPolicy) {
                    inhibitAnyPolicy = skipCerts;
                } 
            } 
        } catch (Exception e) {
	    if (debug != null) {
	        debug.println("PolicyChecker.mergeInhibitAnyPolicy "
                              + "unexpected exception");
	        e.printStackTrace();
	    }
	    throw new CertPathValidatorException(e);
        }	    
        
        return inhibitAnyPolicy; 
    }

    /**
     * Processes certificate policies in the certificate.
     * 
     * @param certIndex the index of the certificate 
     * @param initPolicies the initial policies required by the user
     * @param explicitPolicy an integer which indicates if a non-null
     * valid policy tree is required
     * @param policyMapping an integer which indicates if policy
     * mapping is inhibited
     * @param inhibitAnyPolicy an integer which indicates whether
     * "any-policy" is considered a match
     * @param rejectPolicyQualifiers a boolean indicating whether the
     * user wants to reject policies that have qualifiers
     * @param origRootNode the root node of the valid policy tree
     * @param currCert the Certificate to be processed
     * @param finalCert a boolean indicating whether currCert is the final
     * cert in the cert path
     * @return the root node of the valid policy tree after modification
     * @exception CertPathValidatorException Exception thrown if an
     * error occurs while processing policies.
     */
    static PolicyNodeImpl processPolicies(int certIndex, Set initPolicies,
        int explicitPolicy, int policyMapping, int inhibitAnyPolicy, 
        boolean rejectPolicyQualifiers, PolicyNodeImpl origRootNode,
        X509CertImpl currCert, boolean finalCert) 
	throws CertPathValidatorException
    {
	boolean policiesCritical = false;
	Vector policyInfo;
        PolicyNodeImpl rootNode = null;
        Set anyQuals = new HashSet();

        if (origRootNode == null)
            rootNode = null;
        else
            rootNode = origRootNode.copyTree();

	// retrieve policyOIDs from currCert
        CertificatePoliciesExtension currCertPolicies 
	    = currCert.getCertificatePoliciesExtension();

        // PKIX: Section 6.1.3: Step (d)
        if ((currCertPolicies != null) && (rootNode != null)) {
	    policiesCritical = currCertPolicies.isCritical();
	    if (debug != null)
	        debug.println("PolicyChecker.processPolicies() "
		    + "policiesCritical = " + policiesCritical);

	    try {
	        policyInfo = (Vector) 
		    currCertPolicies.get(CertificatePoliciesExtension.POLICIES);
	    } catch (IOException ioe) {
	        throw new CertPathValidatorException("Exception while "
		    + "retrieving policyOIDs", ioe);
	    }

	    if (debug != null)
	        debug.println("PolicyChecker.processPolicies() "
		    + "rejectPolicyQualifiers = " + rejectPolicyQualifiers);

            boolean foundAnyPolicy = false;

            Iterator policyIter = policyInfo.iterator();
            // process each policy in cert
            while (policyIter.hasNext()) {
	        PolicyInformation curPolInfo 
		    = (PolicyInformation) policyIter.next();
                String curPolicy = 
		    curPolInfo.getPolicyIdentifier().getIdentifier().toString();

                if (curPolicy.equals(ANY_POLICY)) {
                    foundAnyPolicy = true;
                    anyQuals = curPolInfo.getPolicyQualifiers();
                } else {
                    // PKIX: Section 6.1.3: Step (d)(1)
                    if (debug != null)
                        debug.println("PolicyChecker.processPolicies() "
                                      + "processing policy: " + curPolicy);
                    
                    // retrieve policy qualifiers from cert 
                    Set pQuals = curPolInfo.getPolicyQualifiers();
        
		    // reject cert if we find critical policy qualifiers and 
        	    // the policyQualifiersRejected flag is set in the params
        	    if (!pQuals.isEmpty() && rejectPolicyQualifiers && 
			policiesCritical) {
			    throw new CertPathValidatorException("critical " +
			        "policy qualifiers present in certificate");
		    }

                    // PKIX: Section 6.1.3: Step (d)(1)(i)
                    boolean foundMatch = processParents(certIndex, 
			policiesCritical, rejectPolicyQualifiers, rootNode, 
			curPolicy, pQuals, false);
                    
                    if (!foundMatch) {
                        // PKIX: Section 6.1.3: Step (d)(1)(ii)
                        processParents(certIndex, policiesCritical, 
			    rejectPolicyQualifiers, rootNode, curPolicy, 
			    pQuals, true);
                    }
                }
	    } // end of policyIter

            // PKIX: Section 6.1.3: Step (d)(2)
            if (foundAnyPolicy) {
		if ((inhibitAnyPolicy > 0) || 
			(!finalCert && X509CertImpl.isSelfIssued(currCert))) {
		    if (debug != null) {
			debug.println("PolicyChecker.processPolicies() "
			    + "processing policy: " + ANY_POLICY);
		    }
		    processParents(certIndex, policiesCritical,
			rejectPolicyQualifiers, rootNode, ANY_POLICY, anyQuals,
			true);
		}
            }
 
            // PKIX: Section 6.1.3: Step (d)(3)            
            rootNode.prune(certIndex);
            Iterator it = rootNode.getChildren();
            if (!it.hasNext()) {
                rootNode = null;
            }
        } else if (currCertPolicies == null) {
            if (debug != null)
                debug.println("PolicyChecker.processPolicies() "
                              + "no policies present in cert");
            // PKIX: Section 6.1.3: Step (e)            
            rootNode = null;
        }

        // We delay PKIX: Section 6.1.3: Step (f) to the end
        // because the code that follows may delete some nodes
        // resulting in a null tree
        if (rootNode != null) {
            if (!finalCert) {
                // PKIX: Section 6.1.4: Steps (a)-(b)            
                rootNode = processPolicyMappings(currCert, certIndex, 
                    policyMapping, rootNode, policiesCritical, anyQuals);
            }
        }
         
        // At this point, we optimize the PKIX algorithm by
        // removing those nodes which would later have
        // been removed by PKIX: Section 6.1.5: Step (g)(iii)
        
        if ((rootNode != null) && (!initPolicies.contains(ANY_POLICY))
            && (currCertPolicies != null)) {
            rootNode = removeInvalidNodes(rootNode, certIndex, 
                                          initPolicies, currCertPolicies);
					  
	    // PKIX: Section 6.1.5: Step (g)(iii)
	    if ((rootNode != null) && finalCert) {
		// rewrite anyPolicy leaf nodes (see method comments)
		rootNode = rewriteLeafNodes(certIndex, initPolicies, rootNode);
	    }
        }

        
        if (finalCert) {
            // PKIX: Section 6.1.5: Steps (a) and (b)
            explicitPolicy = mergeExplicitPolicy(explicitPolicy, currCert,
                                             finalCert);
        }
            
        // PKIX: Section 6.1.3: Step (f)
        // verify that either explicit policy is greater than 0 or
        // the valid_policy_tree is not equal to NULL

        if ((explicitPolicy == 0) && (rootNode == null)) {
            throw new CertPathValidatorException
                ("non-null policy tree required and policy tree is null");
        }

        return rootNode;
    }
    
    /**
     * Rewrite leaf nodes at the end of validation as described in RFC 3280
     * section 6.1.5: Step (g)(iii). Leaf nodes with anyPolicy are replaced
     * by nodes explicitly representing initial policies not already 
     * represented by leaf nodes.
     *
     * This method should only be called when processing the final cert
     * and if the policy tree is not null and initial policies is not
     * anyPolicy.
     *
     * @param certIndex the depth of the tree
     * @param initPolicies Set of user specified initial policies
     * @param rootNode the root of the policy tree
     */
    private static PolicyNodeImpl rewriteLeafNodes(int certIndex, 
	    Set initPolicies, PolicyNodeImpl rootNode) {
	Set anyNodes = rootNode.getPolicyNodesValid(certIndex, ANY_POLICY);
	if (anyNodes.isEmpty()) {
	    return rootNode;
	}
	PolicyNodeImpl anyNode = (PolicyNodeImpl)anyNodes.iterator().next();
	PolicyNodeImpl parentNode = (PolicyNodeImpl)anyNode.getParent();
	parentNode.deleteChild(anyNode);
	// see if there are any initialPolicies not represented by leaf nodes
	Set initial = new HashSet(initPolicies);
	for (Iterator t = rootNode.getPolicyNodes(certIndex).iterator();
		t.hasNext(); ) {
	    PolicyNodeImpl node = (PolicyNodeImpl)t.next();
	    initial.remove(node.getValidPolicy());
	}
	if (initial.isEmpty()) {
	    // we deleted the anyPolicy node and have nothing to re-add,
	    // so we need to prune the tree
            rootNode.prune(certIndex);
	    if (rootNode.getChildren().hasNext() == false) {
		rootNode = null;
	    }
	} else {
	    boolean anyCritical = anyNode.isCritical();
	    Set anyQualifiers = anyNode.getPolicyQualifiers();
	    for (Iterator t = initial.iterator(); t.hasNext(); ) {
		String policy = (String)t.next();
		Set expectedPolicies = Collections.singleton(policy);
		PolicyNodeImpl node = new PolicyNodeImpl(parentNode, policy, 
		    anyQualifiers, anyCritical, expectedPolicies, false);
	    }
	}
	return rootNode;
    }

    /**
     * Finds the policy nodes of depth (certIndex-1) where curPolicy
     * is in the expected policy set and creates a new child node
     * appropriately. If matchAny is true, then a value of ANY_POLICY
     * in the expected policy set will match any curPolicy. If matchAny
     * is false, then the expected policy set must exactly contain the
     * curPolicy to be considered a match. This method returns a boolean
     * value indicating whether a match was found.
     * 
     * @param certIndex the index of the certificate whose policy is
     * being processed
     * @param policiesCritical a boolean indicating whether the certificate
     * policies extension is critical
     * @param rejectPolicyQualifiers a boolean indicating whether the
     * user wants to reject policies that have qualifiers
     * @param rootNode the root node of the valid policy tree
     * @param curPolicy a String representing the policy being processed
     * @param pQuals the policy qualifiers of the policy being processed or an
     * empty Set if there are no qualifiers
     * @param matchAny a boolean indicating whether a value of ANY_POLICY
     * in the expected policy set will be considered a match
     * @return a boolean indicating whether a match was found
     * @exception CertPathValidatorException Exception thrown if error occurs.
     */
    private static boolean processParents(int certIndex, 
	boolean policiesCritical, boolean rejectPolicyQualifiers, 
	PolicyNodeImpl rootNode, String curPolicy, Set pQuals, 
	boolean matchAny) throws CertPathValidatorException
    {
        boolean foundMatch = false;

        if (debug != null)
            debug.println("PolicyChecker.processParents(): matchAny = " 
		+ matchAny);
        
        // find matching parents
        Set parentNodes = rootNode.getPolicyNodesExpected(certIndex - 1, 
                                                          curPolicy, matchAny);
        Iterator parentIter = parentNodes.iterator();

        // for each matching parent, extend policy tree
        while (parentIter.hasNext()) {
            PolicyNodeImpl curParent = (PolicyNodeImpl) parentIter.next();
            if (debug != null)
                debug.println("PolicyChecker.processParents() "
                              + "found parent:\n" + curParent.asString());
            
            foundMatch = true;
            String curParPolicy = curParent.getValidPolicy();
            
            PolicyNodeImpl curNode = null;
            Set curExpPols = null;
            
            if (curPolicy.equals(ANY_POLICY)) {
                // do step 2
                Set parExpPols = curParent.getExpectedPolicies();
                Iterator parExpPolIter = parExpPols.iterator();
            parentExplicitPolicies:
                while (parExpPolIter.hasNext()) {
                    String curParExpPol = (String) parExpPolIter.next();

                    Iterator childIter = curParent.getChildren();
                    while (childIter.hasNext()) {
                        PolicyNodeImpl childNode = 
			    (PolicyNodeImpl) childIter.next();
                        String childPolicy = childNode.getValidPolicy();
                        if (curParExpPol.equals(childPolicy)) {
                            if (debug != null)
                                debug.println(childPolicy + " in parent's "
				    + "expected policy set already appears in "
				    + "child node");
                            continue parentExplicitPolicies;
                        }
                    }
                        
                    Set expPols = new HashSet();                   
                    expPols.add(curParExpPol);
                    
                    curNode = new PolicyNodeImpl
                        (curParent, curParExpPol, pQuals, 
                         policiesCritical, expPols, false);
                }
            } else {
                curExpPols = new HashSet();
                curExpPols.add(curPolicy);
                
                curNode = new PolicyNodeImpl
                    (curParent, curPolicy, pQuals, 
                     policiesCritical, curExpPols, false);
            }
        }

        return foundMatch;
    }

    /**
     * Processes policy mappings in the certificate.
     *
     * @param currCert the Certificate to be processed
     * @param certIndex the index of the current certificate
     * @param policyMapping an integer which indicates if policy
     * mapping is inhibited
     * @param rootNode the root node of the valid policy tree
     * @param policiesCritical a boolean indicating if the certificate policies
     * extension is critical
     * @param anyQuals the qualifiers associated with ANY-POLICY, or an empty
     * Set if there are no qualifiers associated with ANY-POLICY
     * @return the root node of the valid policy tree after modification
     * @exception CertPathValidatorException exception thrown if an error
     * occurs while processing policy mappings
     */
    private static PolicyNodeImpl processPolicyMappings(X509CertImpl currCert, 
        int certIndex, int policyMapping, PolicyNodeImpl rootNode, 
	boolean policiesCritical, Set anyQuals) 
	throws CertPathValidatorException
    {
        PolicyMappingsExtension polMappingsExt 
            = currCert.getPolicyMappingsExtension();
            
        if (polMappingsExt == null)
	    return rootNode;

        if (debug != null)
            debug.println("PolicyChecker.processPolicyMappings() " 
                + "inside policyMapping check");
            
        Vector maps = null;
        try {
            maps = (Vector) polMappingsExt.get(PolicyMappingsExtension.MAP);
        } catch (IOException e) {
            if (debug != null) {
                debug.println("PolicyChecker.processPolicyMappings() "
                    + "mapping exception");
                e.printStackTrace();
            }
            throw new CertPathValidatorException("Exception while checking "
                                                 + "mapping", e);
        }

        boolean childDeleted = false;
        for (int j = 0; j < maps.size(); j++) {
            CertificatePolicyMap polMap = (CertificatePolicyMap)maps.get(j);
            String issuerDomain 
                = polMap.getIssuerIdentifier().getIdentifier().toString();
            String subjectDomain 
                = polMap.getSubjectIdentifier().getIdentifier().toString();
            if (debug != null) {
                debug.println("PolicyChecker.processPolicyMappings() "
                              + "issuerDomain = " + issuerDomain);
                debug.println("PolicyChecker.processPolicyMappings() "
                              + "subjectDomain = " + subjectDomain);
            }
                
            if (issuerDomain.equals(ANY_POLICY)) {
                throw new CertPathValidatorException
                    ("encountered an issuerDomainPolicy of ANY_POLICY");
            }
                
            if (subjectDomain.equals(ANY_POLICY)) {
                throw new CertPathValidatorException
                    ("encountered a subjectDomainPolicy of ANY_POLICY");
            }

            Set validNodes =
                rootNode.getPolicyNodesValid(certIndex, issuerDomain);
            if (!validNodes.isEmpty()) {
                Iterator validIter = validNodes.iterator();
                while (validIter.hasNext()) {
                    PolicyNodeImpl curNode = (PolicyNodeImpl) validIter.next();
                        
                    if ((policyMapping > 0) || (policyMapping == -1)) {
                        curNode.addExpectedPolicy(subjectDomain);
                    } else if (policyMapping == 0) {
                        PolicyNodeImpl parentNode = 
                            (PolicyNodeImpl) curNode.getParent();
                        if (debug != null)
                            debug.println("PolicyChecker.processPolicyMappings"
				+ "() before deleting: policy tree = " 
				+ rootNode);
                        parentNode.deleteChild(curNode);
                        childDeleted = true;
                        if (debug != null)
                            debug.println("PolicyChecker.processPolicyMappings"
				+ "() after deleting: policy tree = " 
				+ rootNode);
                    }
                }
            } else { // no node of depth i has a valid policy
                if ((policyMapping > 0) || (policyMapping == -1)) {
                    Set validAnyNodes = 
			rootNode.getPolicyNodesValid(certIndex, ANY_POLICY);
                    Iterator validAnyIter = validAnyNodes.iterator();
                    while (validAnyIter.hasNext()) {
                        PolicyNodeImpl curAnyNode = 
			    (PolicyNodeImpl) validAnyIter.next();
                        PolicyNodeImpl curAnyNodeParent = 
			    (PolicyNodeImpl) curAnyNode.getParent();

                        Set expPols = new HashSet();
                        expPols.add(subjectDomain);

                        PolicyNodeImpl curNode = new PolicyNodeImpl
                            (curAnyNodeParent, issuerDomain, anyQuals,
                             policiesCritical, expPols, true);
                    }
                }
            }
        }

        if (childDeleted) {
            rootNode.prune(certIndex);
            Iterator it = rootNode.getChildren();
            if (!it.hasNext()) {
                if (debug != null)
                    debug.println("setting rootNode to null");
                rootNode = null;
            }
        }

        return rootNode;
    }

    /**
     * Removes those nodes which do not intersect with the initial policies
     * specified by the user.
     *
     * @param rootNode the root node of the valid policy tree
     * @param certIndex the index of the certificate being processed
     * @param initPolicies the Set of policies required by the user
     * @param currCertPolicies the CertificatePoliciesExtension of the
     * certificate being processed
     * @returns the root node of the valid policy tree after modification
     * @exception CertPathValidatorException Exception thrown if error occurs.
     */
    private static PolicyNodeImpl removeInvalidNodes(PolicyNodeImpl rootNode,
        int certIndex, Set initPolicies, 
	CertificatePoliciesExtension currCertPolicies) 
	throws CertPathValidatorException
    {
        Vector policyInfo = null;
        try {
            policyInfo = (Vector) 
                currCertPolicies.get(CertificatePoliciesExtension.POLICIES);
        } catch (IOException ioe) {
            throw new CertPathValidatorException("Exception while "
                + "retrieving policyOIDs", ioe);
        }
        
        boolean childDeleted = false;
        Iterator policyIter = policyInfo.iterator();
        while (policyIter.hasNext()) {
            PolicyInformation curPolInfo 
                = (PolicyInformation) policyIter.next();
            String curPolicy = 
		curPolInfo.getPolicyIdentifier().getIdentifier().toString();
            
            if (debug != null)
                debug.println("PolicyChecker.processPolicies() "
                              + "processing policy second time: " + curPolicy);
            
            Set validNodes = rootNode.getPolicyNodesValid(certIndex, curPolicy);
            Iterator validIter = validNodes.iterator();
            while (validIter.hasNext()) {
                PolicyNodeImpl curNode = (PolicyNodeImpl)validIter.next();
                PolicyNodeImpl parentNode = (PolicyNodeImpl)curNode.getParent();
                if (parentNode.getValidPolicy().equals(ANY_POLICY)) {
                    if ((!initPolicies.contains(curPolicy)) && 
                        (!curPolicy.equals(ANY_POLICY))) {
                        if (debug != null)
                            debug.println("PolicyChecker.processPolicies() "
				+ "before deleting: policy tree = " + rootNode);
                        parentNode.deleteChild(curNode);
                        childDeleted = true;
                        if (debug != null)
                            debug.println("PolicyChecker.processPolicies() "
				+ "after deleting: policy tree = " + rootNode);
                    }
                }
            }
        }
        
        if (childDeleted) {
            rootNode.prune(certIndex);
            Iterator it = rootNode.getChildren();
            if (!it.hasNext()) {
                rootNode = null;
            }
        }

        return rootNode;
    }

    /**
     * Gets the root node of the valid policy tree, or null if the
     * valid policy tree is null. Marks each node of the returned tree 
     * immutable and thread-safe.
     *
     * @returns the root node of the valid policy tree, or null if
     * the valid policy tree is null
     */    
    PolicyNode getPolicyTree() {
        if (rootNode == null)
            return null;
        else {
            PolicyNodeImpl policyTree = rootNode.copyTree();
	    policyTree.setImmutable();
	    return policyTree;
	}
    }
}
