// PubSubAppender.java
// 2003-03-07 17:25 by Fred Sanchez
//
// Here's some code for a pubsub log4j appender, so you can configure your
// logging to issue events.  This code calls the kn_publish command from
// c_pubsub/libkn . It can easily be changed to use java_pubsub instead.
 
import org.apache.log4j.*;
import org.apache.log4j.spi.*;
import org.apache.log4j.helpers.LogLog;

public class PubSubAppender extends AppenderSkeleton {

     public String getPubCommand      () { return pubCommand     ; }
     public String getPubSubServerURI () { return pubSubServerURI; }
     public String topic              () { return topic          ; }

     public void setPubCommand      (String value) { pubCommand      = value; }
     public void setPubSubServerURI (String value) { pubSubServerURI = value; }
     public void setTopic           (String value) { topic           = value; }

     private String pubCommand      = null;
     private String pubSubServerURI = null;
     private String topic           = null;

     public boolean requiresLayout () { return false; }

     public boolean ready () {
         if (! ready) {
             ready = true;
             if (pubCommand == null) {
                 LogLog.error("unable to locate publish command; can't send HTTP events.");
                 ready = false;
             }
             if (pubSubServerURI == null) {
                 LogLog.error("no server URI; can't send HTTP events.");
                 ready = false;
             }
             if (topic == null) {
                 LogLog.error("no topic; can't send HTTP events.");
                 ready = false;
             }
         }
         return ready;
     }
     private boolean ready = false;

     public void append (LoggingEvent event) {
         if (! ready()) return;

         String level            = event.getLevel().toString();
         String loggerName       = event.getLoggerName();
         String message          = event.getRenderedMessage();
         String startTime        = String.valueOf(event.getStartTime());
         String threadName       = event.getThreadName();
         String className        = "";
         String fileName         = "";
         String lineNumber       = "";
         String methodName       = "";
         String formattedMessage = (this.layout == null) ? "" : this.layout.format(event);

         LocationInfo locationInfo = event.getLocationInformation();

         if (locationInfo != null) {
             className  = locationInfo.getClassName();
             fileName   = locationInfo.getFileName();
             lineNumber = locationInfo.getLineNumber();
             methodName = locationInfo.getMethodName();
         }

         String command[] = new String[]{
             pubCommand,
             "-H", "level"            , level           ,
             "-H", "class"            , className       ,
             "-H", "file"             , fileName        ,
             "-H", "line"             , lineNumber      ,
             "-H", "method"           , methodName      ,
             "-H", "logger"           , loggerName      ,
             "-H", "message"          , message         ,
             "-H", "start time"       , startTime       ,
             "-H", "thread"           , threadName      ,
             "-H", "formatted message", formattedMessage,
             pubSubServerURI, topic, "/dev/null"
         };

         try {
             if (Runtime.getRuntime().exec(command).waitFor() != 0) {
                 LogLog.warn("unable to log event: " + event.getMessage());
             }
         } catch (Exception e) {
             LogLog.warn ("exception while logging event: " + event.getMessage());
             LogLog.error("exception while logging: " + e);
         }
     }

     public void close () {}
}

// End of PubSubAppender.java
