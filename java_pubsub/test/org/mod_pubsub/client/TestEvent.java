/*
 * Created on Mar 24, 2004
 *
 */
package org.mod_pubsub.client;

import junit.framework.TestCase;

/**
 * @author rtl
 *
 */
public class TestEvent extends TestCase {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	//===========================================================================
	// TEST METHODS
	//===========================================================================
	
	public void testDefaultElementsSetupCorrectly() {
		Event anEvent = new Event();
		
		assertNotNull("Should have a generated id", anEvent.getId());
		assertTrue("Should have a reasonable length id", anEvent.getId().length() > 4);
		assertNull("Should not have a payload", anEvent.getPayload());
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
