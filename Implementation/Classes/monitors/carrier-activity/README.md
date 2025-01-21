# monitor description

this monitor checks that not too many carriers are in waiting state and doing nothing. 

every operation (get, lose, deliver) gets logged into the db and if after a certain amount of 
time no new operation (or less than a certain amount) is added, the monitor sends an alert message. 

sleep_time_micros: is the sleep time after an iteration. measured in microseconds.

threshold: is the threshold of activity under whitch the monitor still sends a warning.

count_get_orders: is a bool value that if false makes the monitor NOT count getting orders as an
activity

### notes: 
if no orders are found for pickup by a carrier, it is NOT an action. 

# usage if the db is active
./exec sleep_time_micros threshold count_get_orders
in init-script.sh the sleep_time is 10 seconds (sleep_time_micros = 10000000 ) and threshold is 2. count = 1 (true)

you can run 
./init-script.sh
