/*
 * Created on Mar 23, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.io.IOException;

import junit.framework.TestCase;

/**
 * @author rtl
 *
 */
public class TestPseudoQuotedPrintable extends TestCase {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public void testQuotedPrintable() throws IOException {
		String aNotrailingSpaceElemName = " a long\nelement:=name";
		String anElemName = aNotrailingSpaceElemName + "  ";

		String anEncodedName =
			PseudoQuotedPrintable.encodePseudoQuotedPrintableName(anElemName);
		assertTrue(anEncodedName.indexOf(':') == -1);
		assertTrue(anEncodedName.indexOf('\n') == -1);
		assertTrue(anEncodedName.indexOf("=name") == -1);
		assertTrue(anEncodedName.endsWith("name"));
		// make sure that decoded value is same as encoded value
		assertEquals(
			aNotrailingSpaceElemName,
			PseudoQuotedPrintable.decodePseudoQuotedPrintable(anEncodedName));

		String aNotrailingSpaceElemValue = "  a long\nelement:=value";
		String anElemValue = aNotrailingSpaceElemValue + "    ";
		String anEncodedValue =
			PseudoQuotedPrintable.encodePseudoQuotedPrintableValue(anElemValue);
		assertTrue(anEncodedValue.indexOf(':') > -1);
		assertTrue(anEncodedValue.indexOf('\n') == -1);
		assertTrue(anEncodedValue.indexOf("=value") == -1);
		assertTrue(anEncodedValue.startsWith("=20=20"));
		assertTrue(anEncodedValue.endsWith("value"));
		// make sure that decoded value is same as encoded value
		assertEquals(
			aNotrailingSpaceElemValue,
			PseudoQuotedPrintable.decodePseudoQuotedPrintable(anEncodedValue));

		// test with JavaMail's quoted-printable encode/decode
		/* comment out, so I don't have to ship the JavaMail jar file
		String anEncodedElemName = "element=0Athree=20is=20=3Along=3D";
		ByteArrayInputStream anInputStream =
			new ByteArrayInputStream(anEncodedElemName.getBytes());
		InputStream aDecodedStream =
			MimeUtility.decode(anInputStream, "quoted-printable");
		String aJavamailDecodedName =
			SimpleEventReader.asString(aDecodedStream);
		
		String aSimpleDecodedName =
			SimpleEventReader.decodePseudoQuotedPrintable(anEncodedElemName);
		assertEquals(
			"Not the same decode as JavaMail",
			aJavamailDecodedName,
			aSimpleDecodedName);
		*/
	}

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

}
