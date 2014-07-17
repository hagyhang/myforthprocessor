/*
 * @(#)AppletMessageHandler.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.text.MessageFormat;

/**
 * An hanlder of localized messages.
 *
 * @version     1.8, 03 Mar 1997
 * @author      Koji Uno
 */
public class AppletMessageHandler {
    private static ResourceBundle rb;
    private String baseKey = null;

    static {
        try {
            rb = ResourceBundle.getBundle
		("sun.applet.resources.MsgAppletViewer");
        } catch (MissingResourceException e) {
            System.out.println(e.getMessage());
            System.exit(1);
        }
    };

    public AppletMessageHandler(String baseKey) {
        this.baseKey = baseKey;
    }

    public String getMessage(String key) {
        return (String)rb.getString(getQualifiedKey(key));
    }

    public String getMessage(String key, Object arg){
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        Object msgobj[] = new Object[1];
	if (arg == null) {
	    arg = "null"; // mimic java.io.PrintStream.print(String)
	}
	msgobj[0] = arg;
	return msgfmt.format(msgobj);
    }

    public String getMessage(String key, Object arg1, Object arg2) {
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        Object msgobj[] = new Object[2];
	if (arg1 == null) {
	    arg1 = "null";
	}
	if (arg2 == null) {
	    arg2 = "null";
	}
	msgobj[0] = arg1;
	msgobj[1] = arg2;
	return msgfmt.format(msgobj);
    }

    public String getMessage(String key, Object arg1, Object arg2, Object arg3) {
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        Object msgobj[] = new Object[3];
	if (arg1 == null) {
	    arg1 = "null";
	}
	if (arg2 == null) {
	    arg2 = "null";
	}
	if (arg3 == null) {
	    arg3 = "null";
	}
	msgobj[0] = arg1;
	msgobj[1] = arg2;
	msgobj[2] = arg3;
	return msgfmt.format(msgobj);
    }

    public String getMessage(String key, Object arg[]) {
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        return msgfmt.format(arg);
    }

    String getQualifiedKey(String subKey) {
        return baseKey + "." + subKey;
    }
}