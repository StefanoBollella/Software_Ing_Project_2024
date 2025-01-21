#!/bin/bash
# NOTE: The options used by the terminal emulator can vary based 
# on the terminal emulator; e.g. xterm and gnome-terminal have different options
# or option meaning

terminalEmulator=gnome-terminal
cd carrier-activity && ($terminalEmulator --title "Monitor-carrier-activity" -- "./init-script.sh" &) && cd ..
cd db-disk-usage && ($terminalEmulator --title "Monitor-db-disk-usage" -- "./init-script.sh" &) && cd ..
cd ord-server-proc-time && ($terminalEmulator --title "Monitor-ord-server-proc-time" -- "./init-script.sh" &) && cd ..
cd "server Activity Checker" && ($terminalEmulator --title "Monitor-server-activity-checker" -- "./init_monitor.sh" &) && cd ..
cd supplierRequestMonitor && ($terminalEmulator --title "Monitor-supplier-request" -- "./init_supplier_monitor.sh" &) && cd ..
