# Software_Ing_Project_2024
<div align="center">
  ![Logo](./backend_logo.png)
</div>


Project for the software engineering class of the third year of "informatica" degree at the Sapienza university in Rome, a.y. 2023/2024. 
The chosen project is a simulator of a backend of an e-commerce site.

## Developed by:
- Jofeth Abello 2016241
- Stefano Bollella 2025438
- Matteo Boccongelli 1908956

## Run

#### Manual startup

To run the program first initialize the databases by launching the file located at the level of the classes:
```bash
./init-dbs.sh
```

once the database is initialized, an initialization bash script; e.g. <code>init-script.sh</code> can be found in the respective folder of each runnable component, execute the file to run the component.

This project uses redis streams for the communication between parts, so if there is an error during connection, make sure redis is running.

To make sure that redis is running:
```bash
redis-cli ping # a pong should be printed
systemctl status redis # on linux to check if redis service is active
```

If you wish to run the components independently via a command line, please follow the structure used in the "init-script.sh" files, since most components require input arguments

#### All at once startup
Using the <code>system-init.sh</code>, you can skip running <code>init-dbs.sh</code>.

To run all at once, use: 
```bash
./system-init.sh
```
attention: this script uses gnome terminal to open multiple terminals at once where the various parts are run. if you do not have gnome terminal it will not work. Or you can use another terminal emulator but you must check if the arguments are the same or have the same meaning.

#### Run all monitors
Inside the <code>implementation/monitors</code> directory you can run each monitor with their respective script. You can also run all the monitors simultaneously by running:
```bash
./monitors-init.sh
```

## Credits

### Redis
To simulate communication between client-side and server-side of the various components, the model uses [redis](https://redis.io) streams. 

### yaml parser
This project uses a yaml parser that can be found at the following github [link](https://github.com/jbeder/yaml-cpp).

### time 

The project uses functions given to us by the professor of our course of Software
Engineering, [Enrico tronci](https://corsidilaurea.uniroma1.it/it/users/enricotronciuniroma1it). the code in question resides inside the Clock class (for now). 

### Tests
[Google Tests](https://github.com/google/googletest) was used to test the program during development.

