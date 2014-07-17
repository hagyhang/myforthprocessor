/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CJavaInstanceCB__)
#define __CJavaInstanceCB__

#include "IJavaInstanceCB.h"
#include "npapi.h"

class CJavaInstanceCB : public IJavaInstanceCB {
public:

    DECL_IEGO

    void showStatus(char * ); 
    void showDocument(char *, char *);
    void findProxy(char *, char **);
    void findCookie(char *, char **);
    void javascriptRequest(char *);
    void setCookie(char *, char *);

    CJavaInstanceCB(NPP);
    virtual ~CJavaInstanceCB();
private:
    NPP m_npp;

};

#endif