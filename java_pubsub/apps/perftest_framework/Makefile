JAVAC = javac

CLASSES = Manager.class 		\
          Agent.class 			\
          Heartbeat.class 		\
          HeartbeatData.class 		\
          RandomObject.class 		\
          KSerializer.class 		\
	  AgentData.class 		\
	  AgentDataMap.class 		\
	  Common.class 			\
	  Dev.class 			\
	  Log.class 			\
	  StatusUpdater.class 		\
	  CommandDownload.class 	\
	  CommandExit.class 		\
	  CommandRun.class 		\
	  StatusRun.class 		\
	  Router.class 			\
	  CommandRoute.class 		\
	  StatusRoute.class 		\
	  CommandPublish.class 		\
	  StatusPublish.class 		\
	  PublisherThread.class 	\
	  CommandSubscribe.class 	\
	  RouterMap.class 		\
	  StatusSubscribe.class 	\
	  Counter.class 		\
	  CounterMap.class 		\
	  CommandResetCounters.class	\
	  CommandDeleteCounters.class	\
	  StatusCounter.class		\
	  CommandReportCounters.class	\
	  CommandStartRouter.class	\
	  CommandStopRouter.class	\
	  CommandClearRouter.class	\
	  StatusDownload.class	  	\
	  StatusStart.class		\
	  StatusConfig.class		\
	  StatusClear.class		\
	  PublisherReporter.class	\
	  RouterWindows.class		\
	  RouterSolaris.class		\
	  RouterLinux.class		\
	  TestSpec.class		\
	  CommandWaitIdle.class		\
	  CommandPrint.class		\
	  AgentSpec.class		\
	  CommandSleep.class

%.class: %.java
	$(JAVAC)  $<

all: $(CLASSES)

clean:
	rm -f *.class *~ #* .#*
