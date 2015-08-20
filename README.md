stat_log
========

stat_log is an extensible C++ library for logging and statistic collection. I got the idea for this after
"reinventing the wheel" on several projects at work.  I finally decided that I would create a re-usable 
library with the following top-level requirements:

1. _The user shall be able to define a hierarchy of statistics at compile time_. From the perspective of the user, the
   purpose of this hierarchy is to simply organize the statistics.  For example, suppose the user is writing software
   for a wireless network application.  Suppose, for simplicity, there are only 3 components of this software
  * **MAC**: (Multiple-Access)  This component may be responsible for relaying user traffic from an IP component sitting above it. 
    Specifically, the MAC's roles: 
      * Fragmentation and Reassembly of IP packets into MAC frames.
      * Buffering/Queuing of said frames.
      * Neighbor discovery protocol.
      * Spectral deconfliction protocol to avoid wireless contention.
  * **SIS**: (Signal-in-Space) This component is responsible for
      * Relaying MAC and control frames between the MAC and the Hardware Interface layers.
      * Collecting per neighbor wireless link statistics (drops, average snr, etc.) for the purpose of optimizing 
        the spectral deconfliction protocol. 
  * **Hardware Interface**:  This component is responsible for interfacing with the hardware (e.g., an FPGA) that is
    responsible for doing the wireless signal processing. 
    
   Given this (highly simplified) design, the user may define the following statistic hierarchy:

  ```cpp
  MAC{
    IP_PKTS_DOWN
    IP_PKTS_UP
    BUFFER_OVERFLOW
  }
  SIS{
    MAC_PKTS_DOWN
    MAC_PKTS_UP
    PER_NBR_STATS{
      LINK_QUALITY
      RECEIVE_STATUS
      LINK_STATUS
    }
  }
  HW_INTERFACE{
     MISC_FPGA_FAULT
     BUFFER_OVERFLOW
  }
  ```

  Note this hierarchy allows the user to reuse the "BUFFER_OVERFLOW" statistic name.
  For concreteness, I will be using this example throughout the documentation and the [Test Code](test).
  
2. _Extremely simple mechanism for updating statistics.  Furthermore, this mechanism should be completely agnostic to
   the details of how the statistical values are to be reported._  Here is one candidate interface which meets this
   requirement:

  ```cpp
  statLog.addStat<MAC::IP_PKTS_DOWN>(1);
  ```
  Or for single dimensioned statistics:
  
  ```cpp
  statLog.addStat<SIS::PER_NBR_STATS::LINK_QUALITY>.addStat(nbrId, thisLinkQuality);
  ```
  Or for multi-dimensioned statistics:
  
   ```cpp
  statLog.addStat<SIS::PER_NBR_STATS::LINK_STATUS>(nbrId, LINK_TYPE_SYMMETRIC, 1);
  ```
  etc.
  
  Suppose for the MAC::IP_PKTS_DOWN statistic, we initially just want to keep a simple counter of all the IP packets
  flowing down into the MAC layer.  To do this we can setup a mapping (using template magic I discuss later) between the
  MAC::IP_PKTS_DOWN tag and a "Simple Counter" statistic.  During the application debugging phase, however, we realize
  that such a simple statistic is not rich enough for troubleshooting purposes.  To this end, we can re-map the
  MAC::IP_PKTS_DOWN tag to a "Time-Series" statistic; this statistic may, for example, store time-stamped values in a
  buffer that can be graphed at a later time.  
  
  Whichever statistic we end up using, the statistic generating site would remain the same:
  ```cpp
  statLog.addStat<MAC::IP_PKTS_DOWN>(1);
  ```
  (A re-compile is necessary to switch the statistic type, however.)
  
  See [Stats](doc/statistic_types.md) for a list of statistic types provided by the stat\_log library _and_ documentation on how to create your own.
  
  
3. _Clean separation between statistic "Generation" and "Control/Observational" modes._  By "Generation" I am referring to the mechanism by which the stastics are recorded/updated in your running application.  By "Control/Observational" I am referring to the mechanism for viewing the statistics or otherwise controlling statistic behavior (e.g. clearing a statistic).  For example, we could have the Generation mode dump statistics to shared memory and this shared memory can be read by another application application (running in Control/Observational mode).  I have chosen this route in my implementation.  For more details see [Statistic Control Application](doc/stat_control_app.md).

4. _Support logging capability._   The application should be able to add a log entry by doing:
  ```cpp
  statLog.logInfo<MAC>() << "Received nbr discovery message from nbr " << nbrId;
  ```
  Or
  ```cpp
  statLog.logError<HW_INTERFACE>() << "FPGA fault : " << faultValue;
  ```
  Node the use of the tags: ```MAC ``` and ```HW_INTERFACE.```  This allows the Statistic Control Application to enable  logging by application _component_ in addition to log level (e.g. Debug, Info, Error and Fatal).
  


TODO: work in progress (I welcome feedback even at this early stage)


