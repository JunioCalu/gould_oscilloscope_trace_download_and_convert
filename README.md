# RS-423/RS-232 Oscilloscope downloader and track-file converter for GOULD Datasys series (DSO 650 and others)

Command line program which connects to a GOULD DSO 650 oscilloscope (most likely GOULD DSO 7xx, 8xx and 9xx as well) via
RS-423 on scope side and RS-232 on PC side. Screenshots can be made (plot screen) and data files (tracks) can be downloaded.
The screenshots (HPGL format) can be postprocessed into encapsulated postscript (eps). The track files (FAMOS/.DAT) can be converted
into Excel-csv.

  * `dso_serial` connects via RS-232/RS-423 to the DSO. A helpscreen on startup shows all the possible options (tested with GOULD DSO 650)
  * `dat2csv.pl` is a perl script which converts a trace file (.DAT) into a excel-csv file. Usage: `./dat2csv.pl trackfile.dat > trackfile.csv` (successfully tested with GOULD DSO 650 and GOULD DSO 740 track files)
  * `hpgl2eps.sh` is a shell script which forces a conversion from HPGL-Plot into eps & pdf. Usage: `./hpgl2eps.sh plot.hpgl`
  * `pltHist.pl` is a perl script which plots the csv file. Usage: `./pltHist.pl trackfile.csv`

## Building

  * type `make`

## Prerequisites on hardware

USB to RS-232 adapter (FTDI) and a connection cable.

Between scope (RS-423 connector) and netbook (RS-232 connector) three connections are necessary:
GND-GND, TX-RX and RX-TX. RS-423 employs somewhat smaller signal levels than RS-232. However the scope manual claims that a
direct connection (without any divider and level shifter) is possible. However i am clipping the TX line which comes
from the PC (RS-232) with a 1k series resistor and two antiparallel zenerdiode/diode down to +/-6V before i connect the
signal to the RX terminal on the scopes connector (RS-423). The RS-423 TX terminal on scope and the RS-232 RX terminal on
PC side is short out whitout any level shifter.

On the scope the signals can be found as follows:

  * Ground: DSUB-9 male #5
  * TXD: DSUB-9 male #2
  * RXD: DSUB-9 male #3

## Set up the GOULD DSO 650 for screen plots

  * I/O MASTER MENU
    * External Plotter: RS423
    * Bulk Transfer: BINARY
  * RS423 PORT MENU
    * Baud Rate: 9600
    * Parity: OFF
    * Data Bits: 8
    * Stop Bits: 1
    * Handshake: OFF
  * SAVE/RECALL MASTER MENU
    * Plot/Save Key: PLOT

Start `./dso_serial -d /dev/ttyUSB -o plot.hpgl -s` then press the plot key on the scope and after finishing the download
press ESC to write down the hpgl data. The data can be converted to eps via hp2xx.

## Set up the GOULD DSO 650 to download trace files

Instead of creating hpgl plots tracks can be stored on the scope mass storage if the plot/save button is configured as "save".
To download such a tracefile from the mass storage the Remote Controler option must be set to RS423 in the I/O MASTER MENU.
You need to know the runnumber and the tracefile name (case sensitive!). You will find these informations on the scope in
the save/recall menu. Assume you want to download and convert the tracedata to excel csv from a file "TR1_5K0.DAT"
which is stored under runnnumber "20" you have to call the program via `./dso_serial -d /dev/ttyUSB -o trace.dat -n 20 -p TR1_5K0.DAT`

## Software and system requirements

dso_serial can be build and run on linux host systems. "dat2csv.pl" should work on Windows as well.
Necessary packages are:

  * [gcc][gcc]
  * [hp2xx][hp2xx]

[gcc]:       http://gcc.gnu.org/
             "GNU Compiler Collection"
[hp2xx]:     http://www.gnu.org/software/hp2xx/
             "Hewlett-Packard's HP-GL plotter format converter"

## The License

LGPLv2.1+

 
