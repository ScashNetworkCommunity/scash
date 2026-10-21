Scash (s/atoshi/cash)
=====================

Intro
-----
Scash is a free open source peer-to-peer electronic cash system that is
completely decentralized, without the need for a central server or trusted
parties.  Users hold the crypto keys to their own money and transact directly
with each other, with the help of a P2P network to check for double-spending.


Setup
-----
Unpack the files into a directory and run scash-qt.exe.

Scash is the original Scash client and it builds the backbone of the network.
However, it downloads and stores the entire history of Scash transactions;
depending on the speed of your computer and network connection, the synchronization
process can take anywhere from a few hours to a day or more.

See the Scash docs at:
  https://github.com/scash-project/gitbook-scash
for more help and information.

Data Directory Note (Scash 2.1.0+)
----------------------------------
Starting in Scash 2.1.0 (Bitcoin Core 28.4), fresh installations on Windows create the default data directory at %LOCALAPPDATA%\Scash (C:\Users\USERNAME\AppData\Local\Scash). Existing installations in %APPDATA%\Scash (AppData\Roaming\Scash) continue working automatically.
