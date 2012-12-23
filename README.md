# Digital storage oscilloscope RS-423 downloader and dataconverter

This is a small command line program collection which may interface Gould DSO 650 or Datasys 900 series oscilloscopes via
RS-423 on scope side and RS-232 on your PC side. Screenshots can be made (plot) but also data files (tracks) can be downloaded.
The screenshots (HPGL format) can be postprocessed into pdf and the track files (FAMOS style) can be converted
into xls-csv or can be plot via gnuplot.

  * dso_serial connects via RS-232/RS-423 to your scope. A helpscreen on startup will show you all the possible options
  * tracks2csv.pl is a perl script which may convert the track files into csv format. Usage: `tracks2csv.pl trackfile.dat > trackfile.csv`
  * hpgl2eps.sh is a shell script which forces a conversion from HPGL-Plot into eps & pdf. Usage: `hpgl2eps plot.hpgl`
  * pltHist.pl is a perl script which plots the csv file. Usage: `pltHist.pl trackfile.csv`

## Building

  * type `make`

## Recommendations on hardware

Just for your information: I only have USB ports on my netbook. Therfore i use a USB to RS-232 converter (FTDI) which is driven
from linux like an ordinary serial interface. The software will not see that there is a USB interface in between scope and
computer.

Between scope (RS-423 connector) and netbook (RS-232 connector) three connections are necessary:
GND-GND, TX-RX and RX-TX. RS-423 employs somewhat smaller signal levels than RS-232. I am clipping the TX line which comes
from the netbook (RS-232) with a 1k series resistor and two zenerdiodes/diodes down to +/-6V before i connect the signal to the
RX terminal on the scopes connector (RS-423). The RS-423 TX terminal on scope and the RS-232 RX terminal on PC side is short together
whitout any level shifter.

On my digital storage scope these three signals can be found as follows:

  * Ground: DSUB-9 male #5
  * TXD: DSUB-9 male #2
  * RXD: DSUB-9 male #3

One last note: Don't forget to setup RS-423 correctly on your oscilloscope menu (9600baud, 8N1 - 8 bits, no parity, 1 stop bit).

## Software and system requirements

dso_serial can be build and run on linux host systems. necessary packages are:

  * [gcc][gcc]
  * [hp2xx][hp2xx]

[gcc]:       http://gcc.gnu.org/
             "GNU Compiler Collection"
[hp2xx]:     http://www.gnu.org/software/hp2xx/
             "Hewlett-Packard's HP-GL plotter format converter"

## The License

LGPLv2.1+

 
