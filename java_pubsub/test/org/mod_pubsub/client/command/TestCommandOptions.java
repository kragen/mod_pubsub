/*
 * Created on Mar 24, 2004
 *
 */
package org.mod_pubsub.client.command;

import junit.framework.TestCase;

/**
 * @author rtl
 *
 */
public class TestCommandOptions extends TestCase {

	//===========================================================================
	// CONSTRUCTORS
	//===========================================================================

	//===========================================================================
	// TEST METHODS
	//===========================================================================
	
	public void testConvenienceMethods() {
		
		CommandOptions someCommandOptions = new CommandOptions();
		
		assertNull("Should have no age option", someCommandOptions.getMaxAge());
		assertEquals("Should have no count option", -1, someCommandOptions.getMaxCount());
		assertNull("Should have no checkpoint option", someCommandOptions.getCheckpoint());
		assertEquals("Should have no options at all", 0, someCommandOptions.getAllOptions().size());
		
		final String aMaxAge = "30";
		someCommandOptions.setMaxAge(aMaxAge);
		assertEquals("Should have an age option", aMaxAge, someCommandOptions.getMaxAge());
		final int aMaxCnt = 369;
		someCommandOptions.setMaxCount(aMaxCnt);
		assertEquals("Should have a count option", aMaxCnt, someCommandOptions.getMaxCount());
		final String aChkPnt = "a-checkpoint";
		someCommandOptions.setCheckpoint(aChkPnt);
		assertEquals("Should have a cp option", aChkPnt, someCommandOptions.getCheckpoint());		
		
		someCommandOptions.set("name1", "value1");
		someCommandOptions.set("name2", "value2");
		assertEquals("Should have correct no. of options", 5, someCommandOptions.getAllOptions().size());
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
