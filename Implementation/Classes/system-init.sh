#!/bin/bash

# Executes the init script for the DBs (including log db)
./init-dbs.sh

# NOTE: The options used by the terminal emulator can vary based 
# on the terminal emulator; e.g. xterm and gnome-terminal have different options
# or option meaning
terminalEmulator=gnome-terminal

cd Clock && ./make-script.sh && cd ..
cd con2redis && ./make-script.sh && cd ..
cd con2db && ./make-script.sh && cd ..
cd logger && ./make-script.sh && cd ..

cd Supplier-shared && ./make-script.sh && cd ..
cd Supplier && ($terminalEmulator --title "Supplier-Client" -- "./init_supplier.sh" &) && cd ..
cd Supplier-server && ($terminalEmulator --title "Supplier-Server" -- "./init_supplier_server.sh" &) && cd ..

sleep 3
cd Customer-shared && ./make-script.sh && cd ..
cd Customer && ($terminalEmulator --title "Customer-Client" -- "./init-script.sh" &) && cd ..
cd Customer-server && ($terminalEmulator --title "Customer-Server" -- "./init-script.sh" &) && cd ..

sleep 3
cd Carrier-shared && ./make-script.sh && cd ..
cd Carrier && ($terminalEmulator --title "Carrier-Client" -- "./init-script.sh" &) && cd ..
cd Carrier-server && ($terminalEmulator --title "Carrier-Server" -- "./init-script.sh" &) && cd ..
