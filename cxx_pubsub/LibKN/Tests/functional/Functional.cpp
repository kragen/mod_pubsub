// TransportT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Globals.h"	//TestUtil.
#include "TestUtil.h"	//TestUtil.
#include <stdio.h>
#include <vector>
using std::vector;
using std::cout;

int main(int argc, char* argv[])
{
	// Process the arguments						//TestUtil.
	string error = TU_INIT_TESTSUITE(argc, argv);	//TestUtil.
	if(error.size() != 0)							//TestUtil.
	{												//TestUtil.
		cout << error;								//TestUtil.
		exit(1);									//TestUtil.
	}												//TestUtil.


	// Get the top level suite from the registry
	CPPUNIT_NS::Test* suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
	
	// Adds the test to the list of test to run
	CPPUNIT_NS::TextUi::TestRunner runner;
	runner.addTest(suite);
	
	// Change the default outputter to a compiler error format outputter
	runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(), std::cerr));
	
	// Run the test.
	bool wasSucessful = runner.run();
	
	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;
}

