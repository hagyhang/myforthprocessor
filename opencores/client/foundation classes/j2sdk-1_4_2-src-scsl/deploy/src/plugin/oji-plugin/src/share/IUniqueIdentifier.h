/*

 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __IUNIQUEIDENTIFIER_H
#define __IUNIQUEIDENTIFIER_H

class IUniqueIdentifier: public nsISupports {
public:
	NS_IMETHOD SetUniqueId(long id) = 0;
	NS_IMETHOD GetUniqueId(long* pId) = 0;
};

#define UNIQUE_IDENTIFIER_IID \
{ /* A8F70EB5-AAEF-11d6-95A4-0050BAAC8BD3 */ \
	0xa8f70eb5, \
	0xaaef, \
	0x11d6, \
	{0x95, 0xa4, 0x0, 0x50, 0xba, 0xac, 0x8b, 0xd3} \
}


#define UNIQUE_IDENTIFIER_ID	"A8F70EB5-AAEF-11d6-95A4-0050BAAC8BD3"

#endif __IUNIQUEIDENTIFIER_H
