/*
 * Created on Mar 22, 2004
 *
 */
package org.mod_pubsub.client.simple;

import java.io.IOException;
import java.io.InputStream;

import org.mod_pubsub.client.Constants;
import org.mod_pubsub.client.Event;

/**
 * @author rtl
 *
 */
public class SimpleEventReader implements Constants {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	public SimpleEventReader(InputStream theInputStream) {
		if (null == theInputStream)
			throw new IllegalArgumentException("The InputStream cannot be null");
		myInputStream = theInputStream;
		myCount = NO_COUNT;
	}

	//===========================================================================
	// QUERY METHODS
	//===========================================================================

	//===========================================================================
	// COMMAND METHODS
	//===========================================================================

	/**
	 * @return
	 */
	public Event readEvent() throws IOException {
		Event anEvent = null;
		int length = getEventLength();
		if (length > 0) {
			String anEventString = new String(readEncodedEvents(length));
			anEvent = new Event();
			StringBuffer aPayloadString = new StringBuffer();
			boolean payloadFound = false;
			int lineEnding = 0;
			while (!payloadFound) {
				int newLineEnding = anEventString.indexOf('\n', lineEnding);
				if (newLineEnding != -1) {
					String anEntry = anEventString.substring(lineEnding, newLineEnding);
					// if has content then is an event element
					// otherwise the next index if present is the payload  
					if (anEntry.length() > 0 && !payloadFound) {
						EventElement anEventElement = extractEventElement(anEntry);
						if (null != anEventElement)
							anEvent.set(
								anEventElement.myName,
								anEventElement.myValue);
						else {
							throw new IOException(
								"ERROR parsing event, received a null EventElement for data:"
									+ anEventString);
						}
					} else if (anEntry.length() == 0 && !payloadFound) {
						payloadFound = true;
					}
					lineEnding = newLineEnding + 1;
				} else {
					break;
				}
				if (payloadFound) {
					aPayloadString.append(anEventString.substring(lineEnding));
				}
			}
			if (payloadFound && aPayloadString.length() > 0)
				anEvent.set(KN_PAYLOAD_PARAM, aPayloadString.toString());

			// reset count
			myCount = NO_COUNT;

		}

		return anEvent;
	}
	
	//===========================================================================
	// HELPER METHODS
	//===========================================================================

	protected int getEventLength() throws IOException {

		// if no count determined as yet then attempt to do so
		if (myCount == NO_COUNT) {
			StringBuffer buffer = new StringBuffer();
			byte b[] = new byte[1];
			char c;
			// while some bytes to read then do so
			while (myInputStream.read(b) > 0) {
				// if end of line received and we have characters then stop looking
				// otherwise keep looking
				if ((b[0] == '\r') || (b[0] == '\n')) {
					if (buffer.length() > 0)
						break;
				}
				// if not white space then use it, otherwise drop it in the bit bucket
				c = (char) b[0];
				if (!Character.isWhitespace(c))
					buffer.append(c);
			}
			// convert to an integer, only if something read
			if (buffer.length() > 0) {
				try {
					myCount = Integer.parseInt(buffer.toString());
				} catch (NumberFormatException ex) {
					throw new IOException(
						"Event format error, expected integer, found:"
							+ buffer.toString()
							+ "||");
				}
			}
		}
		return myCount;
	}

	/**
	 * @param theLength
	 * @return
	 */
	protected byte[] readEncodedEvents(int theLength) throws IOException {
		byte[] buffer = new byte[theLength];
		int bytesRead = myInputStream.read(buffer, 0, theLength);
		if (bytesRead != theLength) {
			while (bytesRead < theLength) {
				bytesRead
					+= myInputStream.read(
						buffer,
						bytesRead,
						theLength - bytesRead);
			}
		}
		return buffer;
	}

	/**
	 * @param theElement - event element in 'name':'value' format (no trailing cr/lf)
	 * @return
	 */
	protected EventElement extractEventElement(String theElement)
		throws IOException {
		String[] anElementStrings = theElement.split(": ");
		if (anElementStrings.length == 2) {
			return new EventElement(anElementStrings[0], anElementStrings[1]);
		} else if (
			anElementStrings.length == 1 && theElement.indexOf(": ") != -1) {
			return new EventElement(anElementStrings[0], "");
		} else {
			return null;
		}
	};

	//===========================================================================
	// DATA MEMBERS
	//===========================================================================

	private InputStream myInputStream;
	private int myCount;

	//===========================================================================
	// STATIC DATA MEMBERS
	//===========================================================================

	public static final int NO_COUNT = -1;

	public static class EventElement {
		public String myName;
		public String myValue;

		public EventElement(String theName, String theValue)
			throws IOException {
			myName =
				PseudoQuotedPrintable.decodePseudoQuotedPrintable(
					theName.trim());
			myValue =
				PseudoQuotedPrintable.decodePseudoQuotedPrintable(
					theValue.trim());
		}

	}

}
