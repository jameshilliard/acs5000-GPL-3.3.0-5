Included here are two makefile/projects: one for *nix, under
   ACS-Web/Server/Target
and the other for Microsoft DevStudio, under
   ACS-Web/Server/Simulator
The latter is for simulation purposes.


"Server" c files
========

The project is broken into "Web" and "Server" trees, consistent with
DMF practice.  The server tree contains the API header and other
important headers, simulator code, and DMF hook-in code.  Important
headers are:
   webApi.h: the API
   Bounds.h: specify string lengths and max values
   ResultCodes.h: specify result codes for API and middleware (shared)

The localization stringtable can be found in
   ACS-Web/Server/Locale
This is a simple key-value-pair table.  Create and use these according
to the DMF user-guide.


"Web" html, asp
=====

ACS-Web/Web contains a "Wizard" and a "Normal" directory, corresponding
to the pages for Wizard and Normal mode.  Shared pages are in the 
Web/ directory itself.  The "Normal" directory contains subdirectories
corresponding to each of the high-level normal-mode menus.


"DMF" goahead server, devie manage
=====

DMF 2.1.2_b2 is required and projects are set up to find the directory
named
   Dmf2.1.2
parallel to the directory which contains ACS-Web.  In other words:
   /Cyclades
      /ACS-Web
         /Server
         ...
   /Dmf2.1.2
      /...
