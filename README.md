This is the source code for the initial version of "Red CS," a popular BBS in Uruguay in the late 1980s.

We didn’t have much money, so the first version ran on an XT motherboard screwed onto a wooden board.

PCs at the time had two COM ports. We started with two lines, but as word spread and the system became more popular, we needed more capacity. We tinkered with the PC boards to remap the IRQs (12/13) to two additional ones (I believe 2 and 9) to support more ports. We eventually handled five lines, but we still needed more. Later, we built "terminal managers" with five lines each, connected via NetBIOS. We ended up with 20 lines at our office, but the local PTT didn’t have the capacity to provide more lines, so we leased a line to another location, where we added 20 more lines, bringing the total to 40.

These were the DOS days—no TCP/IP. The connections between nodes and remote front ends were handcrafted using a protocol inspired by SLIP.

We had leased lines with news agencies like REUTERS and IPS, dial-out lines to other systems (such as DELPHI), and access to AX.25 packet radio for users with a ham radio license.

After this initial version, which we called CS/1, we developed two more versions. CS/2 was written in C++ and still ran on DOS. CS/3, the real breakthrough version, was written in C on AIX and made extensive use of IPC. We had two 16-line RANs to handle all the modems and added UUCP, using a Telebit Trailblazer to call COM!CTS!CRASH several times a day (quite expensive international calls; we used to monitor the modem during the initial calls to prevent anything from going wrong and resulting in a giant bill).

This CS/1 code was made possible thanks to the beauty of CTask, a public-domain library that implemented both preemptive and cooperative multitasking.

After CS/3, which had X.25 access to the Internet and hosted the first web pages in our country, I completely missed what people actually wanted to use. Instead, I focused on creating a distributed VRML-based system with "doors" between nodes (a metaverse—sounds familiar?). I then turned my attention to the VRSPACE project and contributed code to the server side, in Java, using a nice protocol based on LISP S-expressions.

This code doesn't build. It's placed here as a reference only, to share it with old BBS enthusiasts.

