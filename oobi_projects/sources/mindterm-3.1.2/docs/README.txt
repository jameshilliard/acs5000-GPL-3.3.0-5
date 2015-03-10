             README file for MindTerm source distribution

What is MindTerm
================
  
  MindTerm is an SSH client written in 100% java which supports both
  version 1.5 and 2.0 of the SSH protocol as well as a number of
  extensions thereof. It can be used as a standalone program or be run
  as an applet, and it works with any JRE implementing Java 1.1 or
  later. For more details see the section 'MindTerm features' below.

  MindTermLite is a light-weight version of MindTerm primarily
  intended for devices with small screens. It has a much more limited
  GUI and the terminal emulator is optional.

  MindTerm does include third party software (which is available under
  a BSD-type license). For more details see the file
  THIRD_PARTY_SW.txt

  For more info on MindTerm and related issues visit
  http://www.appgate.com/mindterm


How to build MindTerm
=====================

  MindTerm is shipped with a build.xml file for ant. Ant is a java
  based build tool from the Apache foundation (http://ant.apache.org/).
  To compile MindTerm just run:

     ant build

  This will produce a mindterm.jar-file. Other useful targets are:
    lite     - compiles the MindTerm lite client (mindtermlite.jar)
    examples - compiles the examples into examples.jar
    doc      - builds the API documentation (javadoc)
    all      - all of the above

  MindTerm requires Java 1.4 or later to build. The generated binaries
  should run on older versions of Java as well.


How to run MindTerm
===================

  With a modern java it is possible to launch MindTerm with:
    java -jar mindterm.jar

  With older versions of java it is necessary to specify the main
  class: java -cp mindterm.jar com.mindbright.application.MindTerm

  It is also possible to launch MindTerm as an applet. It can be run
  in a separate frame or embedded in a webpage. It is worth noting
  that the MindTerm application always runs on the users client
  machine. This means that any SSH connections will originate from
  this machine. For details on how to set things up see docs/Applet.txt


How to modify MindTerm
======================

  MindTerm contains code for both SSHv1.5 and SSHv2. Use the SSH2
  classes if possible. SSHv1.5 is a deprecated protocol which was
  never properly standardized and which has known security
  problems. Also the SSHv2 implementation in MindTerm is cleaner and
  better documented than the 1.5 implementation.

  The easiest way to get started is to look at the included
  examples. Some of the examples are:
    examples/BasicClient.java      - Opens a terminal window and asks
                                     the user where to connect to.
    examples/RunRemoteCommand.java - Runs a command on a remote server
    examples/SCPExample.java       - Copy a file with the SCP protocol
    examples/SFTPCopyFile          - Copy a file with the SFTP  protocol

  API documentation for MindTerm can either be downloaded from
  http://www.appgate.com/mindterm, or built using "ant doc". The "ant
  doc" method will create a directory called javadocs where the API
  documentation can be found. This documentation includes a full list
  of provided examples.


Support
=======

  Support for the free version of MindTerm is only available via
  mailing-lists. There are two mailing lists for MindTerm. The first
  one is mindterm-announce which is a list where we post announcements
  of new versions. There is no discussion on this list.

  The second list is mindterm-users where people may ask questions and
  discuss MindTerm. We at AppGate do follow this list but we do not
  promise to answer all questions.

  For more information about the lists see
  http://www.appgate.com/mindterm


MindTerm features
=================
  * 100% Java based
  * Support for SSHv1.5 & SSHv2 protocols
  * Support for tunnels and port forwards
  * X11 forwarding
  * Active tunnel display
  * Integrated, full-featured terminal emulator:
    - Full clipboard support (edit, copy, paste)
    - Send text file support
    - Save to text file support-
    - Terminal types: xterm, linux, scoansi, att6386,sun, aixterm,
      vt220, vt100, ansi, vt52, xtermcolor, linux-lat, at386, vt320,
      vt102 and tandem6530
    - Terminal colors
    - Fonts and font sizes
    - Copy-select via mouse selection
    - xterm mouse support
  * Ability to connect through HTTP & SOCKS proxies
  * Support for keep-alive packets
  * Integrated ftp proxy which allows the user to connect with a
    normal ftp client through the SSH tunnel to an ftp server
  * Integrated ftp to sftp proxy which allows the user to connect
    with a normal ftp client to an sftp enabled SSH-2 server
  * Zlib compression (including zlib@openssh.com)
  * Strict host-key checking
  * Supported Ciphers: AES (128, 192, 256), Blowfish, Twofish, Cast,
    3DES, Arcfour (modes cbc, ctr and, for arcfour, ecb)
  * Key exchange support: Diffie-Hellman group-exchange protocol &
    Diffie-Hellman group1-sha1
  * Ability to generate key pairs for DSA & RSA
  * Supported macs: hmac-md5, hmac-sha1, hmac-sha1-96, hmac-md5-96,
    hmac-ripemd160
  * MindTerm supports password authentication, Keyboard-interactive,
    SecurID token cards, public key authentication and certificates. 
