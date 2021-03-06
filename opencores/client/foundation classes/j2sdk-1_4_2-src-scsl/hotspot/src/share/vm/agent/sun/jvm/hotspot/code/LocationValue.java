/*
 * @(#)LocationValue.java	1.3 03/01/23 11:24:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.io.*;

/** A Location value describes a value in a given location; i.e. the
    corresponding logical entity (e.g., a method temporary) lives in
    this location. */

public class LocationValue extends ScopeValue {
  private Location location;

  public LocationValue(Location location) {
    this.location = location;
  }

  public boolean isLocation() {
    return true;
  }

  public Location getLocation() {
    return location;
  }

  /** Serialization of debugging information */
  LocationValue(DebugInfoReadStream stream) {
    location = new Location(stream);
  }

  // FIXME: not yet implementable
  // void write_on(DebugInfoWriteStream* stream);

  // Printing

  public void print() {
    printOn(System.out);
  }

  public void printOn(PrintStream tty) {
    getLocation().printOn(tty);
  }
};
