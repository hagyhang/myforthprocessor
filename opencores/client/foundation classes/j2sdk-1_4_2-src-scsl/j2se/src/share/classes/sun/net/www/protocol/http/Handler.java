/*
 * @(#)Handler.java	1.54 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *	HTTP stream opener
 */

package sun.net.www.protocol.http;

import java.io.IOException;
import java.net.URL;

/** open an http input stream given a URL */
public class Handler extends java.net.URLStreamHandler {
    protected String proxy;
    protected int proxyPort;

    protected int getDefaultPort() {
        return 80;
    }

    public Handler () {
	proxy = null;
	proxyPort = -1;
    }

    public Handler (String proxy, int port) {
	this.proxy = proxy;
	this.proxyPort = port;
    }

    protected java.net.URLConnection openConnection(URL u)
    throws IOException {
	return new HttpURLConnection(u, this);
    }
}
