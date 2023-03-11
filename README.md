# Thread Prority Driver

this is a software kernel driver (non-PnP driver) that can be used by user-mode services to post-up the priority of threads as you want (from 0 -> 31). 

> attached with it a user-mode program that used to apply the functionality of the driver.

## How to run
- first load the driver as a service using (sc.exe) 
- then run the UseThreadPriorityDriver app with no arguments, so it will print to you the help page
