/*
 * @(#)GraphicComponent.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright IBM Corp. 1998, All Rights Reserved
 *
 */

package sun.awt.font;

import java.awt.font.LineMetrics;
import java.awt.font.GraphicAttribute;
import java.awt.font.GlyphJustificationInfo;

import java.awt.geom.Rectangle2D;

import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Shape;

import java.text.Bidi;

import java.util.Map;

public final class GraphicComponent implements TextLineComponent,
                                               Decoration.Label {

    public static final float GRAPHIC_LEADING = 2;

    private GraphicAttribute graphic;
    private int graphicCount;
    private int[] charsLtoV;  // possibly null
    private byte[] levels; // possibly null

    // evaluated in computeVisualBounds
    private Rectangle2D visualBounds = null;

    // used everywhere so we'll cache it
    private float graphicAdvance;
    
    private LineMetrics lineMetrics;
    private Decoration decorator;
    
    private final class GraphicLineMetrics extends LineMetrics {

        public int getNumChars() {
            return graphicCount;
        }

        public float getAscent() {
            return graphic.getAscent();
        }

        public float getDescent() {
            return graphic.getDescent();
        }

        public float getLeading() {
            return GRAPHIC_LEADING;
        }

        public float getHeight() {
            return getAscent() + getDescent() + getLeading();
        }

        public int getBaselineIndex() {
            return graphic.getAlignment();
        }

        public float[] getBaselineOffsets() {
            return new float[] { 0, -7, -14 };  // or something...
        }

        public float getStrikethroughOffset() {
            return -(getAscent() / 2);
        }

        public float getStrikethroughThickness() {
            return getAscent() / 12;
        }

        public float getUnderlineOffset() {
            return getDescent() / 3;
        }

        public float getUnderlineThickness() {
            return getAscent() / 12;
        }
    }

    /**
     * Create a new GraphicComponent.  start and limit are indices
     * into charLtoV and levels.  charsLtoV and levels may be adopted.
     */
    public GraphicComponent(GraphicAttribute graphic,
                            Decoration decorator,
                            int[] charsLtoV,
                            byte[] levels,
                            int start,
                            int limit) {

        if (limit <= start) {
            throw new IllegalArgumentException("0 or negative length in GraphicComponent");
        }
        this.graphic = graphic;
        this.graphicAdvance = graphic.getAdvance();
        lineMetrics = new GraphicLineMetrics();
        this.decorator = decorator;
        
        initLocalOrdering(charsLtoV, levels, start, limit);
    }

    private GraphicComponent(GraphicComponent parent, int start, int limit, int dir) {
        
        this.graphic = parent.graphic;
        this.graphicAdvance = parent.graphicAdvance;
        this.decorator = parent.decorator;
        this.lineMetrics = new GraphicLineMetrics();
        
        int[] charsLtoV = null;
        byte[] levels = null;

        if (dir == UNCHANGED) {
            charsLtoV = parent.charsLtoV;
            levels = parent.levels;
        }
        else if (dir == LEFT_TO_RIGHT || dir == RIGHT_TO_LEFT) {
            limit -= start;
            start = 0;
            if (dir == RIGHT_TO_LEFT) {
                charsLtoV = new int[limit];
                levels = new byte[limit];
                for (int i=0; i < limit; i++) {
                    charsLtoV[i] = limit-i-1;
                    levels[i] = (byte) 1;
                }
            }
        }
        else {
            throw new IllegalArgumentException("Invalid direction flag");
        }

        initLocalOrdering(charsLtoV, levels, start, limit);
    }

    /**
     * Initialize graphicCount, also charsLtoV and levels arrays.
     */
    private void initLocalOrdering(int[] charsLtoV,
                                   byte[] levels,
                                   int start,
                                   int limit) {

        this.graphicCount = limit - start;

        if (charsLtoV == null || charsLtoV.length == graphicCount) {
            this.charsLtoV = charsLtoV;
        }
        else {
            this.charsLtoV = BidiUtils.createNormalizedMap(charsLtoV, levels, start, limit);
        }

        if (levels == null || levels.length == graphicCount) {
            this.levels = levels;
        }
        else {
            this.levels = new byte[graphicCount];
            System.arraycopy(levels, start, this.levels, 0, graphicCount);
        }
    }

    public Rectangle2D handleGetVisualBounds() {

        Rectangle2D bounds = graphic.getBounds();

        float width = (float) bounds.getWidth() +
                                 graphicAdvance * (graphicCount-1);

        return new Rectangle2D.Float((float) bounds.getX(),
                                     (float) bounds.getY(),
                                     width,
                                     (float) bounds.getHeight());
    }
    
    public LineMetrics getLineMetrics() {
        
        return lineMetrics;
    }

    public float getItalicAngle() {

        return 0;
    }

    public Rectangle2D getVisualBounds() {
        
        if (visualBounds == null) {
            visualBounds = decorator.getVisualBounds(this);
        }
        Rectangle2D.Float bounds = new Rectangle2D.Float();
        bounds.setRect(visualBounds);
        return bounds;
    }

    public Shape handleGetOutline(float x, float y) {

        // should add API to GraphicAttribute for this
        return getVisualBounds();
    }
    
    public Shape getOutline(float x, float y) {
        
        return decorator.getOutline(this, x, y);
    }

    public void handleDraw(Graphics2D g2d, float x, float y) {

        for (int i=0; i < graphicCount; i++) {

            graphic.draw(g2d, x, y);
            x += graphicAdvance;
        }
    }
    
    public void draw(Graphics2D g2d, float x, float y) {
        
        decorator.drawTextAndDecorations(this, g2d, x, y);
    }

    public Rectangle2D getCharVisualBounds(int index) {
        
        return decorator.getCharVisualBounds(this, index);
    }
    
    public int getNumCharacters() {

        return graphicCount;
    }

    public float getCharX(int index) {

        int visIndex = charsLtoV==null? index : charsLtoV[index];
        return graphicAdvance * visIndex;
    }

    public float getCharY(int index) {

        return 0;
    }

    public float getCharAdvance(int index) {

        return graphicAdvance;
    }

    public boolean caretAtOffsetIsValid(int index) {

        return true;
    }

    public Rectangle2D handleGetCharVisualBounds(int index) {

        Rectangle2D bounds = graphic.getBounds();
        // don't modify their rectangle, just in case they don't copy

        Rectangle2D.Float charBounds = new Rectangle2D.Float();
        charBounds.setRect(bounds);
        charBounds.x += graphicAdvance * index;

        return charBounds;
    }

    // measures characters in context, in logical order
    public int getLineBreakIndex(int start, float width) {

        int index = (int) (width / graphicAdvance);
        if (index > graphicCount - start) {
            index = graphicCount - start;
        }
        return index;
    }

    // measures characters in context, in logical order
    public float getAdvanceBetween(int start, int limit) {

        return graphicAdvance * (limit - start);
    }

    public Rectangle2D getLogicalBounds() {

        float left = 0;
        float top = -lineMetrics.getAscent();
        float width = graphicAdvance * graphicCount;
        float height = lineMetrics.getDescent() - top;

        return new Rectangle2D.Float(left, top, width, height);
    }

    public TextLineComponent getSubset(int start, int limit, int dir) {

        if (start < 0 || limit > graphicCount || start >= limit) {
            throw new IllegalArgumentException("Invalid range.  start="
                                               +start+"; limit="+limit);
        }

        if (start == 0 && limit == graphicCount && dir == UNCHANGED) {
            return this;
        }

        return new GraphicComponent(this, start, limit, dir);
    }

    public String toString() {

        return "[graphic=" + graphic + ":count=" + getNumCharacters() + "]";
    }
    
  /** 
   * Return the number of justification records this uses.
   */
  public int getNumJustificationInfos() {
    return 0;
  }

  /**
   * Return GlyphJustificationInfo objects for the characters between
   * charStart and charLimit, starting at offset infoStart.  Infos
   * will be in visual order.  All positions between infoStart and
   * getNumJustificationInfos will be set.  If a position corresponds
   * to a character outside the provided range, it is set to null.
   */
  public void getJustificationInfos(GlyphJustificationInfo[] infos, int infoStart, int charStart, int charLimit) {
  }

  /**
   * Apply deltas to the data in this component, starting at offset 
   * deltaStart, and return the new component.  There are two floats
   * for each justification info, for a total of 2 * getNumJustificationInfos.
   * The first delta is the left adjustment, the second is the right 
   * adjustment.
   * <p>
   * If flags[0] is true on entry, rejustification is allowed.  If
   * the new component requires rejustification (ligatures were 
   * formed or split), flags[0] will be set on exit.
   */
  public TextLineComponent applyJustificationDeltas(float[] deltas, int deltaStart, boolean[] flags) {
    return this;
  }
}
