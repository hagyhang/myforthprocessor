/*
 * @(#)Util.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Licensed Materials - Property of IBM
 * RMI-IIOP v1.0
 * Copyright IBM Corp. 1998 1999  All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

package sun.rmi.rmic;

import java.io.File;
import sun.tools.java.Identifier;

/**
 * Util provides static utility methods used by other rmic classes.
 * @author Bryan Atsatt
 */

public class Util implements sun.rmi.rmic.Constants {

    /**
     * Return the directory that should be used for output for a given
     * class.
     * @param theClass The fully qualified name of the class.
     * @param rootDir The directory to use as the root of the
     * package heirarchy.  May be null, in which case the current
     * working directory is used as the root.
     */
    public static File getOutputDirectoryFor(Identifier theClass,
	                                     File rootDir,
	                                     BatchEnvironment env) {
        
        File outputDir = null;
        String className = theClass.getFlatName().toString().replace('.', SIGC_INNERCLASS);    		
	String qualifiedClassName = className;
 	String packagePath = null;
 	String packageName = theClass.getQualifier().toString();
    		
	if (packageName.length() > 0) {
    	    qualifiedClassName = packageName + "." + className;
 	    packagePath = packageName.replace('.', File.separatorChar);
 	}

        // Do we have a root directory?
        
	if (rootDir != null) {
		    
            // Yes, do we have a package name?
                
            if (packagePath != null) {
            	    
    		// Yes, so use it as the root. Open the directory...
        		    
    		outputDir = new File(rootDir, packagePath);
        		    
    		// Make sure the directory exists...
        		    
                ensureDirectory(outputDir,env);
                    
            } else {
            	    
        	// Default package, so use root as output dir...
            	    
        	outputDir = rootDir;
            }		    
	} else {
		    
            // No root directory. Get the current working directory...
                    
            String workingDirPath = System.getProperty("user.dir");
            File workingDir = new File(workingDirPath);
                    
            // Do we have a package name?
                    
            if (packagePath == null) {
                        
                // No, so use working directory...
               
                outputDir = workingDir;
                        
            } else {
                        
                // Yes, so use working directory as the root...
                            
      		outputDir = new File(workingDir, packagePath);
                		    
    		// Make sure the directory exists...
                		    
    		ensureDirectory(outputDir,env);
            }
	}

	// Finally, return the directory...
	    
	return outputDir;
    }
 
    private static void ensureDirectory (File dir, BatchEnvironment env) {
    	if (!dir.exists()) {
            dir.mkdirs();
            if (!dir.exists()) {
                env.error(0,"rmic.cannot.create.dir",dir.getAbsolutePath());
                throw new InternalError();
            }
    	}
    }
}
