/*
 * Created on Mar 23, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;

/**
 * @author rtl
 *
 */
public class PseudoQuotedPrintable {
	
	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================
	
	private PseudoQuotedPrintable() {
	}

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	public static String decodePseudoQuotedPrintable(String theText) {

		StringBuffer buffer = new StringBuffer(theText.length());
		int len = theText.length();

		for (int i = 0; i < len; i++) {
			char c = theText.charAt(i);

			if ((c == '=') && (i + 2 < len)) {
				char a = theText.charAt(++i);
				char b = theText.charAt(++i);

				int ah = HEXADECIMAL_DIGITS.indexOf(a);
				int bh = HEXADECIMAL_DIGITS.indexOf(b);
				if ((ah == -1) || (bh == -1)) {
					buffer.append('=');
					buffer.append(a);
					buffer.append(b);
				} else {
					buffer.append((char) ((ah << 4) | bh));
				}
			} else {
				buffer.append(c);
			}
		}
		return buffer.toString();
	}

	public static String encodePseudoQuotedPrintableName(String theName) {
		return encodePseudoQuotedPrintable(theName, NAME_CHARS_TO_ENCODE);
	}

	public static String encodePseudoQuotedPrintableValue(String theValue) {
		return encodePseudoQuotedPrintable(theValue, VALUE_CHARS_TO_ENCODE);
	}

	public static String encodePseudoQuotedPrintable(
		String theText,
		String theEncodingChars) {

		// right trim the text before starting
		while (theText.endsWith(" "))
			theText = theText.substring(0, theText.length() - 1);
		StringBuffer output = new StringBuffer(theText.length());
		byte[] textBytes = theText.getBytes();
		for (int i = 0; i < textBytes.length; i++) {
			byte b = textBytes[i];
			if (theEncodingChars.indexOf(b) != -1) {
				output.append('=');
				output.append(HEXADECIMAL_DIGITS.charAt(b >> 4));
				output.append(HEXADECIMAL_DIGITS.charAt(b & 0x0F));
				//output.append('=').append(Integer.toHexString(b).toUpperCase());
			} else {
				output.append((char) b);
			}
		}

		return output.toString();

		/*ByteArrayInputStream anInputStream =
			new ByteArrayInputStream(theText.getBytes());
		InputStream aDecodedStream;
		try {
			aDecodedStream = MimeUtility.decode(anInputStream, "quoted-printable");
			return SimpleEventReader.asString(aDecodedStream);
		} catch (MessagingException e) {
			throw new IOException(e.getLocalizedMessage());
		}*/
	}

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	public static String asString(InputStream theInputStream)
		throws IOException {
		InputStreamReader anInputStreamReader =
			new InputStreamReader(theInputStream);
		StringWriter aStringWriter = new StringWriter();
		final char[] buffer = new char[1048];
		int n = 0;
		while (-1 != (n = anInputStreamReader.read(buffer))) {
			aStringWriter.write(buffer, 0, n);
		}
		return aStringWriter.toString();
	}
	
	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	private static final String HEXADECIMAL_DIGITS = "0123456789ABCDEF";
	private static final String NAME_CHARS_TO_ENCODE = "=\n:";
	private static final String VALUE_CHARS_TO_ENCODE = " =\n";

}
