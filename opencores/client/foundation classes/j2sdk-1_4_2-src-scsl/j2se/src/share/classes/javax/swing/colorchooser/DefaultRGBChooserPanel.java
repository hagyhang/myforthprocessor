/*
 * @(#)DefaultRGBChooserPanel.java	1.31 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.swing.colorchooser;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;

/**
 * The standard RGB chooser.
 * <p>
 * <strong>Warning:</strong>
 * Serialized objects of this class will not be compatible with
 * future Swing releases. The current serialization support is
 * appropriate for short term storage or RMI between applications running
 * the same version of Swing.  As of 1.4, support for long term storage
 * of all JavaBeans<sup><font size="-2">TM</font></sup>
 * has been added to the <code>java.beans</code> package.
 * Please see {@link java.beans.XMLEncoder}.
 *
 * @version 1.31 01/23/03
 * @author Steve Wilson
 * @author Mark Davidson
 * @see JColorChooser
 * @see AbstractColorChooserPanel
 */
class DefaultRGBChooserPanel extends AbstractColorChooserPanel implements ChangeListener {

    protected JSlider redSlider;
    protected JSlider greenSlider;
    protected JSlider blueSlider;
    protected JSpinner redField;
    protected JSpinner blueField;
    protected JSpinner greenField;

    private final int minValue = 0;
    private final int maxValue = 255;

    private boolean isAdjusting = false; // indicates the fields are being set internally

    public DefaultRGBChooserPanel() {
        super();
    }

    /** 
     * Sets the values of the controls to reflect the color
     */
    private void setColor( Color newColor ) {
        int red = newColor.getRed();
        int blue = newColor.getBlue();
        int green = newColor.getGreen();

        if (redSlider.getValue() != red) {
            redSlider.setValue(red);
        }
        if (greenSlider.getValue() != green) {
            greenSlider.setValue(green);
        }
        if (blueSlider.getValue() != blue) {
            blueSlider.setValue(blue);
        }

        if (((Integer)redField.getValue()).intValue() != red)
            redField.setValue(new Integer(red));
        if (((Integer)greenField.getValue()).intValue() != green)
            greenField.setValue(new Integer(green));
        if (((Integer)blueField.getValue()).intValue() != blue )
            blueField.setValue(new Integer(blue));
    }
    
    public String getDisplayName() {
        return UIManager.getString("ColorChooser.rgbNameText");
    }

    /**
     * Provides a hint to the look and feel as to the
     * <code>KeyEvent.VK</code> constant that can be used as a mnemonic to
     * access the panel. A return value <= 0 indicates there is no mnemonic.
     * <p>
     * The return value here is a hint, it is ultimately up to the look
     * and feel to honor the return value in some meaningful way.
     * <p>
     * This implementation looks up the value from the default
     * <code>ColorChooser.rgbMnemonic</code>, or if it 
     * isn't available (or not an <code>Integer</code>) returns -1.
     * The lookup for the default is done through the <code>UIManager</code>:
     * <code>UIManager.get("ColorChooser.rgbMnemonic");</code>.
     *
     * @return KeyEvent.VK constant identifying the mnemonic; <= 0 for no
     *         mnemonic
     * @see #getDisplayedMnemonicIndex
     * @since 1.4
     */
    public int getMnemonic() {
        return getInt("ColorChooser.rgbMnemonic", -1);
    }

    /**
     * Provides a hint to the look and feel as to the index of the character in
     * <code>getDisplayName</code> that should be visually identified as the
     * mnemonic. The look and feel should only use this if
     * <code>getMnemonic</code> returns a value > 0.
     * <p>
     * The return value here is a hint, it is ultimately up to the look
     * and feel to honor the return value in some meaningful way. For example,
     * a look and feel may wish to render each
     * <code>AbstractColorChooserPanel</code> in a <code>JTabbedPane</code>,
     * and further use this return value to underline a character in
     * the <code>getDisplayName</code>.
     * <p>
     * This implementation looks up the value from the default
     * <code>ColorChooser.rgbDisplayedMnemonicIndex</code>, or if it 
     * isn't available (or not an <code>Integer</code>) returns -1.
     * The lookup for the default is done through the <code>UIManager</code>:
     * <code>UIManager.get("ColorChooser.rgbDisplayedMnemonicIndex");</code>.
     *
     * @return Character index to render mnemonic for; -1 to provide no
     *                   visual identifier for this panel.
     * @see #getMnemonic
     * @since 1.4
     */
    public int getDisplayedMnemonicIndex() {
        return getInt("ColorChooser.rgbDisplayedMnemonicIndex", -1);
    }

    public Icon getSmallDisplayIcon() {
        return null;
    }

    public Icon getLargeDisplayIcon() {
        return null;
    }
       
    /**
     * The background color, foreground color, and font are already set to the
     * defaults from the defaults table before this method is called.
     */                                                                        
    public void installChooserPanel(JColorChooser enclosingChooser) {
        super.installChooserPanel(enclosingChooser);
    }

    protected void buildChooser() {
      
        String redString = UIManager.getString("ColorChooser.rgbRedText");
        String greenString = UIManager.getString("ColorChooser.rgbGreenText");
        String blueString = UIManager.getString("ColorChooser.rgbBlueText");

        setLayout( new BorderLayout() );
        Color color = getColorFromModel();


        JPanel enclosure = new JPanel();
        enclosure.setLayout( new SmartGridLayout( 3, 3 ) );

        // The panel that holds the sliders

        add( enclosure, BorderLayout.CENTER );
        //        sliderPanel.setBorder(new LineBorder(Color.black));

        // The row for the red value
        JLabel l = new JLabel(redString);
        l.setDisplayedMnemonic(UIManager.getInt("ColorChooser.rgbRedMnemonic"));
        enclosure.add(l);
        redSlider = new JSlider(JSlider.HORIZONTAL, 0, 255, color.getRed());
        redSlider.setMajorTickSpacing( 85 );
        redSlider.setMinorTickSpacing( 17 );
        redSlider.setPaintTicks( true );
        redSlider.setPaintLabels( true );
        enclosure.add( redSlider );
        redField = new JSpinner(
            new SpinnerNumberModel(color.getRed(), minValue, maxValue, 1));
        l.setLabelFor(redSlider);
        JPanel redFieldHolder = new JPanel(new CenterLayout());
        redField.addChangeListener(this);
        redFieldHolder.add(redField);
        enclosure.add(redFieldHolder);


        // The row for the green value
        l = new JLabel(greenString);
        l.setDisplayedMnemonic(UIManager.getInt("ColorChooser.rgbGreenMnemonic"));
        enclosure.add(l);
        greenSlider = new JSlider(JSlider.HORIZONTAL, 0, 255, color.getGreen());
        greenSlider.setMajorTickSpacing( 85 );
        greenSlider.setMinorTickSpacing( 17 );
        greenSlider.setPaintTicks( true );
        greenSlider.setPaintLabels( true );
        enclosure.add(greenSlider);
        greenField = new JSpinner(
            new SpinnerNumberModel(color.getGreen(), minValue, maxValue, 1));
        l.setLabelFor(greenSlider);
        JPanel greenFieldHolder = new JPanel(new CenterLayout());
        greenFieldHolder.add(greenField);
        greenField.addChangeListener(this);
        enclosure.add(greenFieldHolder);

        // The slider for the blue value
        l = new JLabel(blueString);
        l.setDisplayedMnemonic(UIManager.getInt("ColorChooser.rgbBlueMnemonic"));
        enclosure.add(l);
        blueSlider = new JSlider(JSlider.HORIZONTAL, 0, 255, color.getBlue());
        blueSlider.setMajorTickSpacing( 85 );
        blueSlider.setMinorTickSpacing( 17 );
        blueSlider.setPaintTicks( true );
        blueSlider.setPaintLabels( true );
        enclosure.add(blueSlider);
        blueField = new JSpinner(
            new SpinnerNumberModel(color.getBlue(), minValue, maxValue, 1));
        l.setLabelFor(blueSlider);
        JPanel blueFieldHolder = new JPanel(new CenterLayout());
        blueFieldHolder.add(blueField);
        blueField.addChangeListener(this);
        enclosure.add(blueFieldHolder);

        redSlider.addChangeListener( this );
        greenSlider.addChangeListener( this );
        blueSlider.addChangeListener( this );  
        
        redSlider.putClientProperty("JSlider.isFilled", Boolean.TRUE);
        greenSlider.putClientProperty("JSlider.isFilled", Boolean.TRUE);
        blueSlider.putClientProperty("JSlider.isFilled", Boolean.TRUE);
    }

    public void uninstallChooserPanel(JColorChooser enclosingChooser) {
        super.uninstallChooserPanel(enclosingChooser);
    }

    public void updateChooser() {
        if (!isAdjusting) {
            isAdjusting = true;

            setColor(getColorFromModel());

            isAdjusting = false;
        }
    }

    public void stateChanged( ChangeEvent e ) {
        if ( e.getSource() instanceof JSlider && !isAdjusting) {

            int red = redSlider.getValue();
            int green = greenSlider.getValue();
            int blue = blueSlider.getValue() ;
            Color color = new Color (red, green, blue);
    
            getColorSelectionModel().setSelectedColor(color);
        } else if (e.getSource() instanceof JSpinner && !isAdjusting) {

            int red = ((Integer)redField.getValue()).intValue();
            int green = ((Integer)greenField.getValue()).intValue();
            int blue = ((Integer)blueField.getValue()).intValue();
            Color color = new Color (red, green, blue);
            
            getColorSelectionModel().setSelectedColor(color);
        }
    }

}


