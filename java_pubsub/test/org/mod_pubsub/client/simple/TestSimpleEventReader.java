/*
 * Created on Mar 22, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;

import org.mod_pubsub.client.Event;

import junit.framework.TestCase;

/**
 * @author rtl
 *
 */
public class TestSimpleEventReader extends TestCase {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	//===========================================================================
	// TEST METHODS
	//===========================================================================

	public void testCanReadByteCount() throws IOException {

		String expectedElements = "element1:value1\n";
		String aDummyEvent =
			expectedElements.length() + "\n" + expectedElements;
		InputStream anInputStream =
			new ByteArrayInputStream(aDummyEvent.getBytes());

		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		assertEquals(
			"Incorrect event length",
			expectedElements.length(),
			aSimpleEventReader.getEventLength());
	}

	public void testDoesNotReadByteCountIfEmptyLine() {

		String expectedElements = "element1:value1\n";
		String aDummyEvent = "\n" + expectedElements;
		InputStream anInputStream =
			new ByteArrayInputStream(aDummyEvent.getBytes());

		try {
			SimpleEventReader aSimpleEventReader =
				new SimpleEventReader(anInputStream);
			aSimpleEventReader.getEventLength();
			fail("Should throw exception here");
		} catch (IOException ex) {
			// expected
		}
	}

	public void testCanReadByteCountWithWhitespace() throws IOException {

		String expectedElements = "element1:value1\n";
		String aDummyEvent =
			"  \t" + expectedElements.length() + "\t \n" + expectedElements;
		InputStream anInputStream =
			new ByteArrayInputStream(aDummyEvent.getBytes());

		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		assertEquals(
			"Incorrect event length",
			expectedElements.length(),
			aSimpleEventReader.getEventLength());
	}

	public void testCanReadByteCountWithLeadingEmptyLines()
		throws IOException {

		String expectedElements = "element1:value1\n";
		String aDummyEvent =
			"  \n" + expectedElements.length() + "\n" + expectedElements;
		InputStream anInputStream =
			new ByteArrayInputStream(aDummyEvent.getBytes());

		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		assertEquals(
			"Incorrect event length",
			expectedElements.length(),
			aSimpleEventReader.getEventLength());
	}

	public void testDoesNotReadByteCountIfNonInteger() throws IOException {

		String expectedElements = "element1:value1\n";
		String aDummyEvent = "xyz\n" + expectedElements;
		InputStream anInputStream =
			new ByteArrayInputStream(aDummyEvent.getBytes());

		try {
			SimpleEventReader aSimpleEventReader =
				new SimpleEventReader(anInputStream);
			aSimpleEventReader.getEventLength();
			fail("Should throw exception ");
		} catch (IOException ex) {
			// expected
		}
	}

	public void testCanReadEncodedEvents() throws IOException {
		String expectedElements = "element1:value1\n" + "element2:value2\n";
		InputStream anInputStream =
			new ByteArrayInputStream(expectedElements.getBytes());
		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		byte data[] =
			aSimpleEventReader.readEncodedEvents(expectedElements.length());
		assertEquals(
			"Incorrect length",
			expectedElements.length(),
			data.length);
		assertEquals("Incorrect read", expectedElements, new String(data));
	}

	public void testCanReadEncodedEventsWithPartialArrival()
		throws IOException {
		final String expectedElements =
			"element1:value1\n" + "element2:value2\n";

		final PipedInputStream anInputStream = new PipedInputStream();
		final PipedOutputStream anOutputStream = new PipedOutputStream();
		anOutputStream.connect(anInputStream);

		// setup a thread for the output stream to do the writing 
		Thread aThread = new Thread() {
			public void run() {
				final int idx = 11;
				String partElement = expectedElements.substring(0, idx);
				String restElement = expectedElements.substring(idx);
				try {
					anOutputStream.write(partElement.getBytes());
					sleep(1000);
					anOutputStream.write(restElement.getBytes());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		aThread.start();
		byte data[] =
			aSimpleEventReader.readEncodedEvents(expectedElements.length());

		assertEquals(
			"Incorrect length",
			expectedElements.length(),
			data.length);
		assertEquals("Incorrect read", expectedElements, new String(data));
	}

	public void testExtractEventElementNoEncoding() throws IOException {
		String anElemName = "element1";
		String anElemValue = "value1";
		String anElement =
			"   " + anElemName + " " + ":" + "  " + anElemValue + "      ";
		InputStream anInputStream =
			new ByteArrayInputStream(anElement.getBytes());
			
		SimpleEventReader aSimpleEventReader = new SimpleEventReader(anInputStream);

		SimpleEventReader.EventElement anEventElement =
			aSimpleEventReader.extractEventElement(anElement);
		assertNotNull(anEventElement);
		assertEquals("Incorrect name", anElemName, anEventElement.myName);
		assertEquals("Incorrect value", anElemValue, anEventElement.myValue);
	}

	public void testExtractEventElementWithNoValue() throws IOException {
		String anElemName = "kn_route_location";
		String anElement = anElemName + ": ";
		InputStream anInputStream =
			new ByteArrayInputStream(anElement.getBytes());
		
		SimpleEventReader aSimpleEventReader = new SimpleEventReader(anInputStream);

		SimpleEventReader.EventElement anEventElement =
			aSimpleEventReader.extractEventElement(anElement);
		assertNotNull(anEventElement);
		assertEquals("Incorrect name", anElemName, anEventElement.myName);
		assertEquals("Incorrect value", "", anEventElement.myValue);
	}

	public void testFailsExtractEventElementWithBadJoin() throws IOException {
		String anElemName = "element1";
		String anElemValue = "value1";
		String anElement =
			"   " + anElemName + " " + ":" + anElemValue + "      ";
		InputStream anInputStream =
			new ByteArrayInputStream(anElement.getBytes());
		
		SimpleEventReader aSimpleEventReader = new SimpleEventReader(anInputStream);
		SimpleEventReader.EventElement anEventElement =
			aSimpleEventReader.extractEventElement(anElement);
		assertNull(anEventElement);
	}

	public void testExtractEventElementWithEncodedNameValue()
		throws IOException {
		String anElemName = "element\nthree is  long";
		String anEncodedElemName = "element=0Athree=20is=20=20long";
		String anElemValue = "  a long\nelement:=value";
		String anEncodedElemValue = "=20=20a long=0Aelement:=3Dvalue  ";
		String anElement = anEncodedElemName + " " + ": " + anEncodedElemValue;
		InputStream anInputStream =
			new ByteArrayInputStream(anElement.getBytes());
		
		SimpleEventReader aSimpleEventReader = new SimpleEventReader(anInputStream);

		SimpleEventReader.EventElement anEventElement =
			aSimpleEventReader.extractEventElement(anElement);
		assertNotNull(anEventElement);
		assertEquals("Incorrect name", anElemName, anEventElement.myName);
		assertEquals("Incorrect value", anElemValue, anEventElement.myValue);
	}

	public void testExtractEventWithoutPayload() throws IOException {
		String anElemName = "element\nthree is  long";
		String anEncodedElemName =
			PseudoQuotedPrintable.encodePseudoQuotedPrintableName(anElemName);
		String anElemValue = "  a long\nelement:=value";
		String anEncodedElemValue =
			PseudoQuotedPrintable.encodePseudoQuotedPrintableValue(anElemValue);
		String anElement = anEncodedElemName + " " + ": " + anEncodedElemValue;
		String anEventString = anElement.length() + 1 + "\n" + anElement + "\n";

		InputStream anInputStream =
			new ByteArrayInputStream(anEventString.getBytes());
		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		Event anEvent = aSimpleEventReader.readEvent();
		assertNotNull(anEvent);
		assertEquals("Incorrect no. of entries - includes auto id", 2, anEvent.getElementCount());
		assertEquals(
			"Incorrect element value",
			anElemValue,
			anEvent.get(anElemName));
		assertNull(anEvent.getPayload());
	}

	public void testExtractEventWithPayload() throws IOException {
		final int maxElements = 5;
		StringBuffer anElementList = new StringBuffer();
		for (int i = 0; i < maxElements; i++) {
			String anElemName = "name" + i;
			String anEncodedElemName =
				PseudoQuotedPrintable.encodePseudoQuotedPrintableName(
					anElemName);
			String anElemValue = "value" + i;
			String anEncodedElemValue =
				PseudoQuotedPrintable.encodePseudoQuotedPrintableValue(
					anElemValue);
			anElementList
				.append(anEncodedElemName)
				.append(": ")
				.append(anEncodedElemValue)
				.append("\n");
		}

		String aPayload = "This is a test payload";
		String anEventString =
			anElementList.length()
				+ aPayload.length()
				+ 1
				+ "\n"
				+ anElementList.toString()
				+ "\n"
				+ aPayload;

		InputStream anInputStream =
			new ByteArrayInputStream(anEventString.getBytes());
		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		Event anEvent = aSimpleEventReader.readEvent();
		assertNotNull(anEvent);
		assertEquals(
			"Incorrect no. of entries - includes auto id",
			maxElements + 2,
			anEvent.getElementCount());
		for (int i = 0; i < maxElements; i++) {
			assertEquals(
				"Incorrect element value",
				"value" + i,
				anEvent.get("name" + i));
		}
		assertEquals("Incorrect payload", aPayload, anEvent.getPayload());
	}

	public void testExtractEventWithPayloadIncludingLinefeeds()
		throws IOException {
		String anElemName = "element1";
		String anEncodedElemName =
			PseudoQuotedPrintable.encodePseudoQuotedPrintableName(anElemName);
		String anElemValue = "value1";
		String anEncodedElemValue =
			PseudoQuotedPrintable.encodePseudoQuotedPrintableValue(anElemValue);
		String anElement = anEncodedElemName + " " + ": " + anEncodedElemValue;
		String aPayload = "This is a payload\nspread across 2 lines";
		String anEventString =
			anElement.length()
				+ aPayload.length()
				+ 2
				+ "\n"
				+ anElement
				+ "\n\n"
				+ aPayload;

		InputStream anInputStream =
			new ByteArrayInputStream(anEventString.getBytes());
		SimpleEventReader aSimpleEventReader =
			new SimpleEventReader(anInputStream);

		Event anEvent = aSimpleEventReader.readEvent();
		assertNotNull(anEvent);
		assertEquals("Incorrect no. of entries - includes auto id", 3, anEvent.getElementCount());
		assertEquals(
			"Incorrect element value",
			anElemValue,
			anEvent.get(anElemName));
		assertEquals(aPayload, anEvent.getPayload());
	}

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
