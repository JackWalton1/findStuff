I was not able to make my findStuff program use pipes. All the children just print 
their results to the screen. My reasoning:
        I used a signal when a child is finished to let the parent know that it's done.
    This interrupts the parent, clears stdin, and reprompts the user. I tried my best
    (see pipe.c) to mimic the parent child situation in findStuff.c, but could not open
    pipes using iteration indices. (See {cid variable in findStuff.c}/{iteration in pipe.c})
        The reason this proved difficult was because I could not give my signalHandler()
    any input, specifically the iteration variable. Because of this, I cannot open
    the specific pipe I need to using list of file descripters [iteration]. I tried
    to solve this in pipe.c by making a function that calls the signalHandler, but
    that still has not proven to be a good solution.

Also note that if you use "list" that it will clear the child's message before it is done.
(still runs, just cannot be listed twice)
This was a problem with the childmessages themselves being shared memory, but the array of 
char*'s (childMessages[10]) does not act like shared memory. I had to clear the message 
after listing it, otherwise, whatever I listed before would never clear.

Also line 131 in findStuff.c has a commented out sleep function that you can use to test 
the limit. If there are 10 children, it does not let another process run until 1st one is 
finished. 11 Searches is possible, just not simultaneous.