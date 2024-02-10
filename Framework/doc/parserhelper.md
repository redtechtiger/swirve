# Swirve API Parse helper
*A table with functions calls compatibel with Swirve Framework.*  
|Command|Primary Argument|Data Argument|Description|
|-------|----------------|-------------|-----------|
|getval|pwr|n/a|Return EPowerState of assigned loaded module.|
|getval|log|n/a|Returns server process log of assigned loaded module.|
|getval|slog|n/a|Returns a server process log of assigned loaded module, cut off at 20KB.|
|getval|cpu|n/a|Returns a tuple of systemwide CPU usage along with server process CPU usage by PID.|
|getval|mem|n/a|Returns a tuple of systemwide memory usage along with server process memory usage by PID.|
|getval|sws|n/a|Returns all the above.|
|getval|cred|n/a|Returns credentials for the (FTP) server.|
|getval|mod|n/a|Returns a module array, consisting of currently loaded modules in ram.|
|setval|pwr|EPowerAction|EPowerAction is forwarded to the power handler in the assigned moodule. Returns function status.|
|setval|log|String|Data argument is piped into the assigned modules' server process. Returns function status.|
|setval|mod|mod.nr|Module requested as per mod.nr is assigned to the current connection instance. Returns function status.|
|dynamicmod|add|name ram launchpath|An archive as per archivedata is written to the disk to be loaded next bootup. Currently no changes will be commited until the next framework reboot.|
|dynamicmod|del|archive.id|The archive as per archive.id is deleted from the disk to be reflected next bootup. Currently no changes will be commited until the next framework reboot.|
|dynamicmod|overwrite|archive.id name ram launchpath|The archive as per archive.id is overwritten with data from archivedata. Currently no changes will be commited until the next framework reboot.|
|dynamicmod|get|archive.id|Get the archive data as per archive id
|elevatereq|authorise|Pass|Authorises the current connection to make use of elevated API calls, to modulate framework power along with other critical administrative features.|
|elevatereq|frameworkshutdown|n/a|Powers off the Swirve Framework daemon along with all loaded servers and modules. Framework needs to be manually rebooted through system reset or by rebooting the SystemCTL service.|
|elevatereq|frameworkrestart|n/a|Restarts the Swirve Framework daemon along with all loaded servers and modules.
|elevatereq|frameworkpowerdown|n/a|Powers down the entire Swirve Framework and it's related infrastructure, before completely powering down the server. All loaded servers and modules will be abruptly shut down with a 10 second notice.|
|elevatereq|frameworkreboot|n/a|Reboots the entire server infrastructure and physical hardware and performs a hot reload.|
|elevatereq|frameworkkillall|n/a|Kills all current connections to Framework. Returns function status.