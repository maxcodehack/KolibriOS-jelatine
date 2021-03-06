/*
 * MIDPath - Copyright (C) 2006-2007 Guillaume Legris, Mathieu Legris
 * 
 * Odonata - Copyright (C) 2002-2006 Stephane Meslin-Weber <steph@tangency.co.uk>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation. 
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt). 
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA 
 * 
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */
package org.thenesis.midpath.font.bdf;

/**
 * Represents a single font Glyph.
 * @author Stephane Meslin-Weber
 */
public class BDFGlyph {
	public String name;
	public BDFParser.Rectangle bbx = new BDFParser.Rectangle();
	public int[] data;
	public StringBuffer rawData;
	private BDFFontContainer font;

	/**
	 * Decodes a BDF font BITMAP/ENDCHAR sequence of packed bits.
	 * @param depth bits per pixel from parent font
	 * @param data bit data encoded in text bytes
	 * @return raw bit values
	 */
	public int[] decode(int depth, StringBuffer data) {
		int[] result = new int[(data.length() / 2) * (8 / depth)];

		for (int i = 0, offset = 0; i < data.length(); i += 2, offset += (8 / depth)) {
			String cut = data.toString().substring(i, i + 2);
			int value = Integer.parseInt(cut, 16);

			int bits[] = null;

			//
			// Bit long, and not optimised...
			//
			switch (depth) {
			case 1:
				bits = new int[] { (value & 0x80) >> 7, (value & 0x40) >> 6, (value & 0x20) >> 5, (value & 0x10) >> 4,
						(value & 0x08) >> 3, (value & 0x04) >> 2, (value & 0x02) >> 1, (value & 0x01), };
				break;
			case 2:
				bits = new int[] { (value & 0xC0) >> 6, (value & 0x30) >> 4, (value & 0x0C) >> 2, (value & 0x03), };
				break;
			case 4:
				bits = new int[] { (value & 0xF0) >> 4, (value & 0x0F), };
				break;
			case 8:
				bits = new int[] { value & 0xFF, };
				break;
			}

			for (int k = 0; k < bits.length; k++)
				result[offset + k] = bits[k];
		}

		return result;
	}

	public BDFParser.Rectangle getBbx() {
		return new BDFParser.Rectangle(bbx);
	}

	void init(BDFFontContainer font) {
		this.font = font;
		this.data = decode(getFont().getDepth(), rawData);
		rawData = null;
	}

	public void setBbx(BDFParser.Rectangle bbx) {
		this.bbx = bbx;
	}

	public int[] getData() {
		return data;
	}

	public void setRawData(StringBuffer rawData) {
		this.rawData = rawData;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public BDFFontContainer getFont() {
		return font;
	}

	public BDFGlyph(String name) {
		this.name = name;
	}

	public void setSWidth(int swx0, int swy0) {
		//		System.err.println("SWidth: "+swx0+","+swy0);
	}

	private BDFParser.Dimension dsize = new BDFParser.Dimension();

	public void setDWidth(int dwx0, int dwy0) {
		dsize.setSize(dwx0, dwy0);
	}

	public BDFParser.Dimension getDWidth() {
		return dsize;
	}

	public void setBBX(int x, int y, int width, int height) {
		bbx.setBounds(x, y, width, height);
	}

	public String toString() {
		return "BDFGlyph[name=" + name + ", bbx=" + bbx + ", dsize=" + dsize + "]";
	}
}