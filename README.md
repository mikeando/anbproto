# ANB Core Prototype

This is a prototype for the core of the new AuthorsNotebook infrastructure.
It also serves as a demo for the `agb` and `rxc` libraries.

Hopefully others can use it as a framework to build applications that use git as a 
persistence and collaboration platform.

## ANB Core Prototype structure

There are four major threads running in the ANBCorePrototype. The first two are both git threads.
One is for remote operations, the other for local operations.
This is so that the local operations do not block the remote operations.

The next thread is the usual GUI thread.

The final thread is the sqlite3 thread. 

Note git is only used for long-term persistance, collaboration and conflict handling. Most "day-to-day"
operations go through the sqlite3 database instead.

