# Author: SpookyProDH-Coder
set terminal png size 1200,800 enhanced font 'Verdana,14'
set grid

set key left top vertical Left reverse enhanced samplen 2 box lw 1

set style line 1 lc rgb '#0066cc' lt 1 lw 2.5 pt 7 ps 1.2
set style line 2 lc rgb '#cc0000' lt 1 lw 2.5 pt 5 ps 1.2

set output 'ComparativaPasos.png'

set title "Comparativa del Coste Algorítmico (Número de Pasos / Búsquedas)" font 'Verdana,16 Bold'
set xlabel "Tamaño del problema (N)" font 'Verdana,12 Bold'
set ylabel "Pasos totales ejecutados" font 'Verdana,12 Bold'

set format x "%.0f"
set format y "%.0f"

plot "New_Steps.data" using 1:2 with lines linestyle 1 title "Nueva Tabla (Dense Map - Linear Probing)", \
     "Old_Steps.data" using 1:2 with lines linestyle 2 title "Tabla Antigua (Chained List)"

unset output

set output 'ComparativaTiempo.png'

set title "Comparativa del Coste Temporal Medio (Tiempo de CPU por Elemento)" font 'Verdana,16 Bold'
set xlabel "Tamaño del problema (N)" font 'Verdana,12 Bold'
set ylabel "Tiempo medio por elemento (Segundos)" font 'Verdana,12 Bold'

set format y "%.2e"

plot "New_Time.data" using 1:2 with lines linestyle 1 title "Nueva Tabla (Dense Map - Linear Probing)", \
     "Old_Time.data" using 1:2 with lines linestyle 2 title "Tabla Antigua (Chained List)"

unset output

set terminal qt