#Biblotecas
import curses
import psutil
import threading
import time
import platform

#variables
global NumHilos
global contador
global mutex
global barrer
#Variables para  sincronizar hilos
NumHilos = 8
contador = 0
mutex = threading.Semaphore(1)
barrer= threading.Semaphore(0)



#Conversion de bytes 
def convesion(num, sufijo='B'):
    for unit in ['','K','M','G','T','P','E','Z']:
        if abs(num) < 1024.0:
            return "%3.1f%s%s" % (num, unit, sufijo)
        num /= 1024.0
    return "%.1f%s%s" % (num, 'Yi', sufijo)

#procesos en ejecucion
def procesos(tamañio):
	lstproc = psutil.pids()
	lstproc.reverse()


	for x in xrange(0,tamanio):
		pantalla.addstr(10+x, 2,"%d "%lstproc[x])
		pantalla.addstr(10+x, 8,psutil.Process(lstproc[x]).name()+"         ")
		pantalla.addstr(10+x, 22,psutil.Process(lstproc[x]).status()+"  ")
		pantalla.addstr(10+x, 35,"%d   "%psutil.Process(lstproc[x]).num_threads())
		pantalla.addstr(10+x, 43,psutil.Process(lstproc[x]).username()+"      ")
		pantalla.addstr(10+x, 53,"%.2f"%psutil.Process(lstproc[x]).cpu_percent(interval=0))
		pantalla.addstr(10+x, 63,convesion(psutil.Process(lstproc[x]).memory_info().rss)+"    ")



#inicalizacion de la barrera
def dentbarre():
	global contador
	mutex.acquire()
	contador=contador+1
	mutex.release()

	if contador == NumHilos:
		barrer.release()
	barrer.acquire()
	barrer.release()

