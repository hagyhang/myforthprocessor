/*
 * @(#)VMObjectFactory.java	1.4 03/01/23 11:46:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.lang.reflect.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

/** <P> This class implements a factory mechanism for the objects
    created to wrap Addresses. It requires that the class passed in
    implement a constructor taking with the following signature: </P>

    <P>
    <CODE> public &lt;Type&gt;(sun.jvm.hotspot.Address)
    </CODE>
    </P>

    <P> It is used to write shorter code when wrapping Addresses since
    null checks are no longer necessary. In addition, it is a central
    location where a canonicalizing map could be implemented if one
    were desired (though the current system is designed to not require
    one.) </P>
*/

public class VMObjectFactory {
  public static Object newObject(Class clazz, Address addr)
    throws ConstructionException {
    try {
      if (addr == null) {
        return null;
      }

      Constructor c = clazz.getConstructor(new Class[] {
        Address.class
      });
      return c.newInstance(new Object[] { addr });
    }
    catch (Exception e) {
      throw new ConstructionException(e);
    }
  }
}
