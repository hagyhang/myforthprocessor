/*
 * @(#)Win32Renderer.cpp	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_windows_Win32Renderer.h"
#include "java_awt_geom_PathIterator.h"

#include "Win32SurfaceData.h"
#include "awt_Component.h"
#include "awt_Pen.h"
#include "awt_Brush.h"
#include "ddrawUtils.h"

#include "jni.h"

#include <windows.h>
#include <math.h>                /* for cos(), sin(), etc */

extern "C" {

static char *GeneralPathClassName = "java/awt/geom/GeneralPath";

static jfieldID pTypesID;
static jfieldID pCoordsID;
static jfieldID pNumTypesID;
static jfieldID pWindingRuleID;

#define POLYTEMPSIZE	(512 / sizeof(POINT))

static void AngleToCoord(jint angle, jint w, jint h, jint *x, jint *y)
{
    const double pi = 3.1415926535;
    const double toRadians = 2 * pi / 360;

    *x = (long)(cos((double)angle * toRadians) * w);
    *y = -(long)(sin((double)angle * toRadians) * h);
}

static POINT *TransformPoly(jint *xpoints, jint *ypoints,
			    jint transx, jint transy,
			    POINT *pPoints, jint *pNpoints,
			    BOOL close, BOOL fixend)
{
    int npoints = *pNpoints;
    int outpoints = npoints;
    jint x, y;

    // Fix for 4298688 - draw(Line) and Polygon omit last pixel
    // We will need to add a point if we need to close it off or
    // if we need to fix the endpoint to accomodate the Windows
    // habit of never drawing the last pixel of a Polyline.  Note
    // that if the polyline is already closed then neither fix
    // is needed because the last pixel is also the first pixel
    // and so will be drawn just fine.
    // Clarification for 4298688 - regression bug 4678208 points
    // out that we still need to fix the endpoint if the closed
    // polygon never went anywhere (all vertices on same coordinate).
    jint mx = xpoints[0];
    jint my = ypoints[0];
    BOOL isclosed = (xpoints[npoints-1] == mx && ypoints[npoints-1] == my);
    if ((close && !isclosed) || fixend) {
	outpoints++;
	*pNpoints = outpoints;
    }
    if (outpoints > POLYTEMPSIZE) {
	pPoints = (POINT *) safe_Malloc(sizeof(POINT) * outpoints);
    }
    BOOL isempty = fixend;
    for (int i = 0; i < npoints; i++) {
	x = xpoints[i];
	y = ypoints[i];
	isempty = isempty && (x == mx && y == my);
	pPoints[i].x = x + transx;
	pPoints[i].y = y + transy;
    }
    if (close && !isclosed) {
	pPoints[npoints] = pPoints[0];
    } else if (fixend) {
        if (!close || isempty) {
            // Fix for 4298688 - draw(Line) and Polygon omit last pixel
            // Fix up the last segment by adding another segment after
            // it that is only 1 pixel long.  The first pixel of that
            // segment will be drawn, but the second pixel is the one
            // that Windows omits.
            pPoints[npoints] = pPoints[npoints-1];
            pPoints[npoints].x++;
        } else {
            outpoints--;
            *pNpoints = outpoints;
        }
    }

    return pPoints;
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_initIDs
    (JNIEnv *env, jclass wr)
{
    jclass pGeneralPathClass = env->FindClass(GeneralPathClassName);
    if (pGeneralPathClass == 0) {
	JNU_ThrowClassNotFoundException(env, GeneralPathClassName);
	return;
    }
    pTypesID = env->GetFieldID(pGeneralPathClass, "pointTypes", "[B");
    pCoordsID = env->GetFieldID(pGeneralPathClass, "pointCoords", "[F");
    pNumTypesID = env->GetFieldID(pGeneralPathClass, "numTypes", "I");
    pWindingRuleID = env->GetFieldID(pGeneralPathClass, "windingRule", "I");
    if (pTypesID == 0 || pCoordsID == 0 ||
	pNumTypesID == 0 || pWindingRuleID == 0)
    {
	JNU_ThrowNoSuchFieldError(env, "GeneralPath");
	return;
    }
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doDrawLine
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doDrawLine
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x1, jint y1, jint x2, jint y2)
{
    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }

    HDC hdc;
    jint patrop;
    if (x1 == x2 || y1 == y2) {
	if (x1 > x2) {
	    jint t = x1; x1 = x2; x2 = t;
	}
	if (y1 > y2) {
	    jint t = y1; y1 = y2; y2 = t;
	}
	hdc = wsdo->GetDC(env, wsdo, BRUSH, &patrop, clip, comp, color);
        if (hdc == NULL) {
            return;
        }
	::PatBlt(hdc, x1, y1, x2-x1+1, y2-y1+1, patrop);
    } else {
	hdc = wsdo->GetDC(env, wsdo, PENBRUSH, &patrop, clip, comp, color);
        if (hdc == NULL) {
            return;
        }
	::MoveToEx(hdc, x1, y1, NULL);
	::LineTo(hdc, x2, y2);
	::PatBlt(hdc, x2, y2, 1, 1, patrop);
    }
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doDrawRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doDrawRect
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h)
{
    if (w < 0 || h < 0) {
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }
    jint patrop;
    HDC hdc = wsdo->GetDC(env, wsdo, BRUSH, &patrop, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    if (w < 2 || h < 2) {
	// If one dimension is less than 2 then there is no
	// gap in the middle - draw a solid filled rectangle.
	::PatBlt(hdc, x, y, w+1, h+1, patrop);
    } else {
	// Avoid drawing the endpoints twice.
	// Also prefer including the endpoints in the
	// horizontal sections which draw pixels faster.
	::PatBlt(hdc,  x,   y,  w+1,  1,  patrop);
	::PatBlt(hdc,  x,  y+1,  1,  h-1, patrop);
	::PatBlt(hdc, x+w, y+1,  1,  h-1, patrop);
	::PatBlt(hdc,  x,  y+h, w+1,  1,  patrop);
    }
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doDrawRoundRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doDrawRoundRect
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h, jint arcW, jint arcH)
{
    if (w < 2 || h < 2 || arcW <= 0 || arcH <= 0) {
	// Fix for 4524760 - drawRoundRect0 test case fails on Windows 98
	// Thin round rects degenerate into regular rectangles
	// because there is no room for the arc sections.  Also
	// if there is no arc dimension then the roundrect must
	// be a simple rectangle.  Defer to the DrawRect function
	// which handles degenerate sizes better.
	Java_sun_awt_windows_Win32Renderer_doDrawRect(env, wr,
						      sData, clip, comp, color,
						      x, y, w, h);
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }
    HDC hdc = wsdo->GetDC(env, wsdo, PENONLY, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::RoundRect(hdc, x, y, x+w+1, y+h+1, arcW, arcH);
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doDrawOval
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doDrawOval
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h)
{
    if (w < 2 || h < 2) {
	// Thin enough ovals have no room for curvature.  Defer to
	// the DrawRect method which handles degenerate sizes better.
	Java_sun_awt_windows_Win32Renderer_doDrawRect(env, wr,
						      sData, clip, comp, color,
						      x, y, w, h);
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }
    HDC hdc = wsdo->GetDC(env, wsdo, PENONLY, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::Ellipse(hdc, x, y, x+w+1, y+h+1);
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doDrawArc
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doDrawArc
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h,
     jint angleStart, jint angleExtent)
{
    if (w < 0 || h < 0 || angleExtent == 0) {
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }

    long sx, sy, ex, ey;
    if (angleExtent >= 360 || angleExtent <= -360) {
	sx = ex = x + w;
	sy = ey = y + h/2;
    } else {
	int angleEnd;
	if (angleExtent < 0) {
	    angleEnd = angleStart;
	    angleStart += angleExtent;
	} else {
	    angleEnd = angleStart + angleExtent;
	}
	AngleToCoord(angleStart, w, h, &sx, &sy);
	sx += x + w/2;
	sy += y + h/2;
	AngleToCoord(angleEnd, w, h, &ex, &ey);
	ex += x + w/2;
	ey += y + h/2;
    }
    HDC hdc = wsdo->GetDC(env, wsdo, PEN, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::Arc(hdc, x, y, x+w+1, y+h+1, sx, sy, ex, ey);
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doDrawPoly
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;III[I[IIZ)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doDrawPoly
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint transx, jint transy,
     jintArray xpointsarray, jintArray ypointsarray,
     jint npoints, jboolean isclosed)
{
    if (env->GetArrayLength(xpointsarray) < npoints ||
	env->GetArrayLength(ypointsarray) < npoints)
    {
	JNU_ThrowArrayIndexOutOfBoundsException(env, "coordinate array");
	return;
    }
    if (npoints < 2) {
	// Fix for 4067534 - assertion failure in 1.3.1 for degenerate polys
	// Not enough points for a line.
	// Note that this would be ignored later anyway, but returning
	// here saves us from mistakes in TransformPoly and seeing bad
	// return values from the Windows Polyline function.
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }

    POINT tmpPts[POLYTEMPSIZE], *pPoints;
    jint *xpoints = (jint *) env->GetPrimitiveArrayCritical(xpointsarray, NULL);
    jint *ypoints = (jint *) env->GetPrimitiveArrayCritical(ypointsarray, NULL);
    pPoints = TransformPoly(xpoints, ypoints, transx, transy,
			    tmpPts, &npoints, isclosed, TRUE);
    env->ReleasePrimitiveArrayCritical(xpointsarray, xpoints, JNI_ABORT);
    env->ReleasePrimitiveArrayCritical(ypointsarray, ypoints, JNI_ABORT);
    if (pPoints == NULL) {
	return;
    }

    HDC hdc = wsdo->GetDC(env, wsdo, PEN, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::Polyline(hdc, pPoints, npoints);
    wsdo->ReleaseDC(env, wsdo, hdc);
    if (pPoints != tmpPts) {
	free(pPoints);
    }
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doFillRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doFillRect
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h)
{
    if (w <= 0 || h <= 0) {
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }
    jint patrop;
    HDC hdc = wsdo->GetDC(env, wsdo, BRUSH, &patrop, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::PatBlt(hdc, x, y, w, h, patrop);
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doFillRoundRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doFillRoundRect
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h, jint arcW, jint arcH)
{
    if (w < 2 || h < 2 || arcW <= 0 || arcH <= 0) {
	// Fix related to 4524760 - drawRoundRect0 fails on Windows 98
	// Thin round rects have no room for curvature.  Also, if
	// the curvature is empty then the primitive has degenerated
	// into a simple rectangle.  Defer to the FillRect method
	// which deals with degenerate sizes better.
	Java_sun_awt_windows_Win32Renderer_doFillRect(env, wr,
						      sData, clip, comp, color,
						      x, y, w, h);
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }
    HDC hdc = wsdo->GetDC(env, wsdo, BRUSHONLY, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::RoundRect(hdc, x, y, x+w+1, y+h+1, arcW, arcH);
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doFillOval
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doFillOval
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h)
{
    if (w < 3 || h < 3) {
	// Fix for 4411814 - small ovals do not draw anything
	// (related to 4205762 on Solaris platform)
	// Most platform graphics packages have poor rendering
	// for thin ellipses and the rendering is most strikingly
	// different from our theoretical arcs.  Ideally we should
	// trap all ovals less than some fairly large size and
	// try to draw aesthetically pleasing ellipses, but that
	// would require considerably more work to get the corresponding
	// drawArc variants to match pixel for pixel.
	// Thin ovals of girth 1 pixel are simple rectangles.
	// Thin ovals of girth 2 pixels are simple rectangles with
	// potentially smaller lengths.  Determine the correct length
	// by calculating .5*.5 + scaledlen*scaledlen == 1.0 which
	// means that scaledlen is the sqrt(0.75).  Scaledlen is
	// relative to the true length (w or h) and needs to be
	// adjusted by half a pixel in different ways for odd or
	// even lengths.
#define SQRT_3_4 0.86602540378443864676
	if (w > 2 && h > 1) {
	    int adjw = (int) ((SQRT_3_4 * w - ((w&1)-1)) * 0.5);
	    adjw = adjw * 2 + (w&1);
	    x += (w-adjw)/2;
	    w = adjw;
	} else if (h > 2 && w > 1) {
	    int adjh = (int) ((SQRT_3_4 * h - ((h&1)-1)) * 0.5);
	    adjh = adjh * 2 + (h&1);
	    y += (h-adjh)/2;
	    h = adjh;
	}
#undef SQRT_3_4
	if (w > 0 && h > 0) {
	    Java_sun_awt_windows_Win32Renderer_doFillRect(env, wr, sData,
							  clip, comp, color,
							  x, y, w, h);
	}
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }
    HDC hdc = wsdo->GetDC(env, wsdo, BRUSHONLY, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::Ellipse(hdc, x, y, x+w+1, y+h+1);
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doFillArc
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doFillArc
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint x, jint y, jint w, jint h,
     jint angleStart, jint angleExtent)
{
    if (w <= 0 || h <= 0 || angleExtent == 0) {
	return;
    }
    if (angleExtent >= 360 || angleExtent <= -360) {
	// Fix related to 4411814 - small ovals (and arcs) do not draw
	// If the arc is a full circle, let the Oval method handle it
	// since that method can deal with degenerate sizes better.
	Java_sun_awt_windows_Win32Renderer_doFillOval(env, wr,
						      sData, clip, comp, color,
						      x, y, w, h);
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }
    long sx, sy, ex, ey;
    int angleEnd;
    if (angleExtent < 0) {
	angleEnd = angleStart;
	angleStart += angleExtent;
    } else {
	angleEnd = angleStart + angleExtent;
    }
    AngleToCoord(angleStart, w, h, &sx, &sy);
    sx += x + w/2;
    sy += y + h/2;
    AngleToCoord(angleEnd, w, h, &ex, &ey);
    ex += x + w/2;
    ey += y + h/2;
    HDC hdc = wsdo->GetDC(env, wsdo, BRUSHONLY, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::Pie(hdc, x, y, x+w+1, y+h+1, sx, sy, ex, ey);
    wsdo->ReleaseDC(env, wsdo, hdc);
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doFillPoly
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;III[I[II)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doFillPoly
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint transx, jint transy,
     jintArray xpointsarray, jintArray ypointsarray,
     jint npoints)
{
    if (env->GetArrayLength(xpointsarray) < npoints ||
	env->GetArrayLength(ypointsarray) < npoints)
    {
	JNU_ThrowArrayIndexOutOfBoundsException(env, "coordinate array");
	return;
    }
    if (npoints < 3) {
	// Fix for 4067534 - assertion failure in 1.3.1 for degenerate polys
	// Not enough points for a triangle.
	// Note that this would be ignored later anyway, but returning
	// here saves us from mistakes in TransformPoly and seeing bad
	// return values from the Windows Polyline function.
	return;
    }

    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }

    POINT tmpPts[POLYTEMPSIZE], *pPoints;
    jint *xpoints = (jint *) env->GetPrimitiveArrayCritical(xpointsarray, NULL);
    jint *ypoints = (jint *) env->GetPrimitiveArrayCritical(ypointsarray, NULL);
    pPoints = TransformPoly(xpoints, ypoints, transx, transy,
			    tmpPts, &npoints, FALSE, FALSE);
    env->ReleasePrimitiveArrayCritical(xpointsarray, xpoints, JNI_ABORT);
    env->ReleasePrimitiveArrayCritical(ypointsarray, ypoints, JNI_ABORT);
    if (pPoints == NULL) {
	return;
    }

    HDC hdc = wsdo->GetDC(env, wsdo, BRUSHONLY, NULL, clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::SetPolyFillMode(hdc, ALTERNATE);
    ::Polygon(hdc, pPoints, npoints);
    wsdo->ReleaseDC(env, wsdo, hdc);
    if (pPoints != tmpPts) {
	free(pPoints);
    }
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doShape
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IILjava/awt/geom/GeneralPath;Z)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_doShape
    (JNIEnv *env, jobject wr,
     jobject sData,
     jobject clip, jobject comp, jint color,
     jint transX, jint transY,
     jobject gp, jboolean isfill)
{
    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, sData);
    if (wsdo == NULL) {
	return;
    }

    jarray typesarray = (jarray) env->GetObjectField(gp, pTypesID);
    jarray coordsarray = (jarray) env->GetObjectField(gp, pCoordsID);
    jint numtypes = env->GetIntField(gp, pNumTypesID);
    if (env->GetArrayLength(typesarray) < numtypes) {
	JNU_ThrowArrayIndexOutOfBoundsException(env, "types array");
	return;
    }
    jint maxcoords = env->GetArrayLength(coordsarray);
    jint rule = env->GetIntField(gp, pWindingRuleID);

    HDC hdc = wsdo->GetDC(env, wsdo, (isfill ? BRUSH : PEN), NULL,
                          clip, comp, color);
    if (hdc == NULL) {
        return;
    }
    ::SetPolyFillMode(hdc, (rule == java_awt_geom_PathIterator_WIND_NON_ZERO
			    ? WINDING : ALTERNATE));
    ::BeginPath(hdc);
    jbyte *types = (jbyte *) env->GetPrimitiveArrayCritical(typesarray,
							    NULL);
    jfloat *coords = (jfloat *) env->GetPrimitiveArrayCritical(coordsarray,
							       NULL);
    int index = 0;
    BOOL ok = TRUE;
    BOOL isempty = TRUE;
    BOOL isapoint = TRUE;
    int mx = 0, my = 0, x1 = 0, y1 = 0;
    POINT ctrlpts[3];
    for (int i = 0; ok && i < numtypes; i++) {
	switch (types[i]) {
	case java_awt_geom_PathIterator_SEG_MOVETO:
	    if (!isfill && !isempty) {
		// Fix for 4298688 - draw(Line) omits last pixel
		// Windows omits the last pixel of a path when stroking.
		// Fix up the last segment of the previous subpath by
		// adding another segment after it that is only 1 pixel
		// long.  The first pixel of that segment will be drawn,
		// but the second pixel is the one that Windows omits.
		::LineTo(hdc, x1+1, y1);
	    }
	    if (index + 2 <= maxcoords) {
		mx = x1 = transX + (int) coords[index++];
		my = y1 = transY + (int) coords[index++];
		::MoveToEx(hdc, x1, y1, NULL);
		isempty = TRUE;
		isapoint = TRUE;
	    } else {
		ok = FALSE;
	    }
	    break;
	case java_awt_geom_PathIterator_SEG_LINETO:
	    if (index + 2 <= maxcoords) {
		x1 = transX + (int) coords[index++];
		y1 = transY + (int) coords[index++];
		::LineTo(hdc, x1, y1);
		isapoint = isapoint && (x1 == mx && y1 == my);
		isempty = FALSE;
	    } else {
		ok = FALSE;
	    }
	    break;
	case java_awt_geom_PathIterator_SEG_QUADTO:
	    if (index + 4 <= maxcoords) {
		ctrlpts[0].x = transX + (int) coords[index++];
		ctrlpts[0].y = transY + (int) coords[index++];
		ctrlpts[2].x = transX + (int) coords[index++];
		ctrlpts[2].y = transY + (int) coords[index++];
		ctrlpts[1].x = (ctrlpts[0].x * 2 + ctrlpts[2].x) / 3;
		ctrlpts[1].y = (ctrlpts[0].y * 2 + ctrlpts[2].y) / 3;
		ctrlpts[0].x = (ctrlpts[0].x * 2 + x1) / 3;
		ctrlpts[0].y = (ctrlpts[0].y * 2 + y1) / 3;
		x1 = ctrlpts[2].x;
		y1 = ctrlpts[2].y;
		::PolyBezierTo(hdc, ctrlpts, 3);
		isapoint = isapoint && (x1 == mx && y1 == my);
		isempty = FALSE;
	    } else {
		ok = FALSE;
	    }
	    break;
	case java_awt_geom_PathIterator_SEG_CUBICTO:
	    if (index + 6 <= maxcoords) {
		ctrlpts[0].x = transX + (int) coords[index++];
		ctrlpts[0].y = transY + (int) coords[index++];
		ctrlpts[1].x = transX + (int) coords[index++];
		ctrlpts[1].y = transY + (int) coords[index++];
		ctrlpts[2].x = transX + (int) coords[index++];
		ctrlpts[2].y = transY + (int) coords[index++];
		x1 = ctrlpts[2].x;
		y1 = ctrlpts[2].y;
		::PolyBezierTo(hdc, ctrlpts, 3);
		isapoint = isapoint && (x1 == mx && y1 == my);
		isempty = FALSE;
	    } else {
		ok = FALSE;
	    }
	    break;
	case java_awt_geom_PathIterator_SEG_CLOSE:
	    ::CloseFigure(hdc);
	    if (x1 != mx || y1 != my) {
		x1 = mx;
		y1 = my;
		::MoveToEx(hdc, x1, y1, NULL);
		isempty = TRUE;
		isapoint = TRUE;
	    } else if (!isfill && !isempty && isapoint) {
		::LineTo(hdc, x1+1, y1);
		::MoveToEx(hdc, x1, y1, NULL);
		isempty = TRUE;
		isapoint = TRUE;
	    }
	    break;
	}
    }
    env->ReleasePrimitiveArrayCritical(typesarray, types, JNI_ABORT);
    env->ReleasePrimitiveArrayCritical(coordsarray, coords, JNI_ABORT);
    if (ok) {
	if (!isfill && !isempty) {
	    // Fix for 4298688 - draw(Line) omits last pixel
	    // Windows omits the last pixel of a path when stroking.
	    // Fix up the last segment of the previous subpath by
	    // adding another segment after it that is only 1 pixel
	    // long.  The first pixel of that segment will be drawn,
	    // but the second pixel is the one that Windows omits.
	    ::LineTo(hdc, x1+1, y1);
	}
	::EndPath(hdc);
	if (isfill) {
	    ::FillPath(hdc);
	} else {
	    ::StrokePath(hdc);
	}
    } else {
	::AbortPath(hdc);
	JNU_ThrowArrayIndexOutOfBoundsException(env, "coords array");
    }
    wsdo->ReleaseDC(env, wsdo, hdc);
}

} /* extern "C" */

extern int currNumDevices;

INLINE BOOL RectInMonitorRect(RECT *rCheck, RECT *rContainer)
{
    // Assumption: left <= right, top <= bottom
    if (rCheck->left >= rContainer->left &&
	rCheck->right <= rContainer->right &&
	rCheck->top >= rContainer->top &&
	rCheck->bottom <= rContainer->bottom)
    {
	return TRUE;
    } else {
	return FALSE;
    }
}

/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    devCopyArea
 * Signature: (Lsun/awt/windows/SurfaceData;IIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32Renderer_devCopyArea
    (JNIEnv *env, jobject wr,
     jobject wsd,
     jint srcx, jint srcy,
     jint dx, jint dy,
     jint width, jint height)
{
    Win32SDOps *wsdo = Win32SurfaceData_GetOps(env, wsd);
    DTRACE_PRINTLN("Win32SurfaceData_devCopyArea");
    DTRACE_PRINTLN6("  sx, sy, dx, dy, w, h: %d, %d, %d, %d, %d, %d",
	srcx, srcy, dx, dy, width, height);
    if (wsdo == NULL) {
	return;
    }

    if (DDCanBlt(wsdo)) {
	// We try to use ddraw for this copy because it tends to be faster
	// than GDI.  We punt if:
	//	- the source rect is clipped (GDI does a better job of
	//	validating the clipped area)
	//	- there is only one monitor 
	//	 OR
	//	- the src and dest rectangles are both wholly within the device
	//	associated with this surfaceData
	// REMIND: There may be a bug looming out here where the user
	// moves an overlapping window between the time that we check the
	// clip and the time that the Blt is executed, then we may not be
	// invalidating the src region the way we should.

	// Here, rSrc, rDst are the src/dst rectangles in screen coordinates.
	// r[Src|Dst]Absolute are the src/dst rectangles in virtual
	// screen space (where all monitors occupy the same coords).
	RECT rSrc = {srcx, srcy, srcx + width, srcy + height};
	RECT rDst, rSrcAbsolute, rDstAbsolute;
	POINT clientOrigin = {0, 0};
	MONITOR_INFO *mi = wsdo->device->GetMonitorInfo();
	::ScreenToClient(wsdo->window, &clientOrigin);
	::OffsetRect(&rSrc, 
	    (-clientOrigin.x - wsdo->insets.left), 
	    (-clientOrigin.y - wsdo->insets.top));
	rSrcAbsolute = rSrc;
	::OffsetRect(&rSrc, (-mi->rMonitor.left), (-mi->rMonitor.top));
	rDst = rSrc;
	::OffsetRect(&rDst, dx, dy);
	rDstAbsolute = rSrcAbsolute;
	::OffsetRect(&rDstAbsolute, dx, dy);

	if (DDClipCheck(wsdo, &rSrc) &&
	    (currNumDevices <= 1 ||
    	     (RectInMonitorRect(&rSrcAbsolute, &mi->rMonitor) &&
	      RectInMonitorRect(&rDstAbsolute, &mi->rMonitor))))
	{
	    AwtComponent *comp = NULL;

	    // Bug 4362500: Win2k pointer garbage on screen->screen DD blts
	    if (IS_WIN2000) {
		comp = Win32SurfaceData_GetComp(env, wsdo);
		if (comp != NULL) {
		    comp->SendMessage(WM_AWT_HIDECURSOR, NULL);
		}
	    }

	    DDBlt(env,wsdo, wsdo, &rDst, &rSrc);

	    if (comp != NULL) {
		comp->SendMessage(WM_AWT_SHOWCURSOR, NULL);
	    }
	    return;
	}
    }
    HDC hDC = wsdo->GetDC(env, wsdo, 0, NULL, NULL, NULL, 0);
    if (hDC == NULL) {
	return;
    }

    RECT r;
    ::SetRect(&r, srcx, srcy, srcx + width, srcy + height);
    HRGN rgnUpdate = ::CreateRectRgn(0, 0, 0, 0);
    VERIFY(::ScrollDC(hDC, dx, dy, &r, NULL, rgnUpdate, NULL));

    // ScrollDC invalidates the part of the source rectangle that
    // is outside of the destination rectangle on the assumption
    // that you wanted to "move" the pixels from source to dest,
    // and so now you will want to paint new pixels in the source.
    // Since our copyarea operation involves no such semantics we
    // are only interested in the part of the update region that
    // corresponds to unavailable source pixels - i.e. the part
    // that falls within the destination rectangle.

    // The update region will be in client relative coordinates
    // but the destination rect will be in window relative coordinates
    ::OffsetRect(&r, dx-wsdo->insets.left, dy-wsdo->insets.top);
    HRGN rgnDst = ::CreateRectRgnIndirect(&r);
    int result = ::CombineRgn(rgnUpdate, rgnUpdate, rgnDst, RGN_AND);

    // Invalidate the exposed area.
    if (result != NULLREGION) {
	::InvalidateRgn(wsdo->window, rgnUpdate, TRUE);
    }
    ::DeleteObject(rgnUpdate);
    ::DeleteObject(rgnDst);

    wsdo->ReleaseDC(env, wsdo, hDC);
}