/*
 * @(#)LocaleElements_lt_LT.java	1.10 03/01/23
 */

/*
 * Portions Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright Taligent, Inc. 1996, 1997 - All Rights Reserved
 * (C) Copyright IBM Corp. 1996 - 1998 - All Rights Reserved
 *
 * The original version of this source code and documentation
 * is copyrighted and owned by Taligent, Inc., a wholly-owned
 * subsidiary of IBM. These materials are provided under terms
 * of a License Agreement between Taligent and Sun. This technology
 * is protected by multiple US and International patents.
 *
 * This notice and attribution to Taligent may not be removed.
 * Taligent is a registered trademark of Taligent, Inc.
 *
 */

package sun.text.resources;

import java.util.ListResourceBundle;

public class LocaleElements_lt_LT extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "NumberPatterns",
                new String[] {
                    "##,##0.##;-##,##0.##", // decimal pattern
                    "#,##0.## \u00A4;-#,##0.## \u00A4", // currency pattern
                    "#,##%" // percent pattern
                }
            },
            { "CurrencySymbols",
                new String[][] {
                   {"LTL", "Lt"}
                }
            },
            { "DateTimeElements",
                new String[] {
                    "2", // first day of week
                    "4" // min days in first week
                }
            }
        };
    }
}
