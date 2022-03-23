# TDT4186-e2
The second exercise for TDT4186, Operating systems

To build project, run (requires make and gcc):

```
make mtwwwd 
```

Then (on unix systems) run the webserver with

```
./mtwwwd.out document-path-root #port #threads #bufferslots
```

>*Passing 'default' as document-path-root assigns a working default path to the webserver.*

While running, the webserver will respond to requests adhering to http/0.9 with the requested document, if it exists. Otherwise the response will be a coresponding http error. 

Our solutions to task *e* is implemented and documented in mtwwwd.c. Specifically they are described on lines 64-76.