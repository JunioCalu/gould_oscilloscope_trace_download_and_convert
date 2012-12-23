#!/bin/bash
if [ -n "$1" ] ; then
  hp2xx $1 -d300 -p1111 -c2134 -h150 -a 1.414 -m eps -f plot_output.eps
  epstopdf plot_output.eps
else
  hp2xx dump.asci -d300 -p1111 -c2134 -h150 -a 1.414 -m eps -f plot_output.eps
  epstopdf plot_output.eps
  echo "Usage: ./hpgl2eps.sh plot.hpgl"
fi

# hp2xx hpglfile -d300 -p1111 -h150 -a 1.414 -m eps -f $2
# hp2xx test.hp -d300 -p1111 -h50 -a 1.414 -m pcx -f blah.pcx

