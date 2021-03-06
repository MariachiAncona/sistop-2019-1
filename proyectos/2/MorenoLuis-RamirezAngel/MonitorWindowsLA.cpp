// MonitorWindows.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>


#pragma comment(lib, "iphlpapi.lib")
std::mutex candado;

char c = 'a';
short imprimiendo=0, calculoCPU=0;
unsigned int usoCPU = 0, tUtilCPU, tTotalCPU;
short actualizoRAM = 0, actualizala = 0;
unsigned int porciento = 0;
float usada = 0, total = 0, disponible = 0;
short actDatagramas = 0, okData = 0;
float dTot = 0, dEot = 0;


void usoDelCPU();
void usoDeRam();
void datagramas();
void imprime();
void barraCPU();
void barraRAM();
void barraRecibe();
void barraEnvia();
void pideImpresion();
void usoMisDatos();


int main()
{


	std::thread hilos[4];


	hilos[0] = std::thread(usoDelCPU); //Lanza los hilos
	hilos[1] = std::thread(usoDeRam);
	hilos[2] = std::thread(datagramas);
	hilos[3] = std::thread(pideImpresion);

	while (c != '\n') //Hasta que de ENTER sale del ciclo y termina el programa
	{
		c = getchar();
	}

	hilos[0].join();//espera a que los hilos terminen
	hilos[1].join();
	hilos[2].join();
	hilos[3].join();

	printf("\n\tCerrando");
	Sleep(100);
	printf(".");
	Sleep(100);
	printf(".");
	Sleep(100);
	printf(".");
	Sleep(100);
	return 0;
}


void usoDelCPU()
{
	FILETIME idle, kernel, user;
	ULARGE_INTEGER aIdle, nIdle, aKernel, nKernel, aUser, nUser, tIdle, tKernel, tUser;

	GetSystemTimes(&idle, &kernel, &user); //Obtenemos datos para inicializar variables

	aIdle.LowPart = idle.dwLowDateTime; //Inicializamos las variables "antiguas"
	aIdle.HighPart = idle.dwHighDateTime;
	aKernel.LowPart = kernel.dwLowDateTime;
	aKernel.HighPart = kernel.dwHighDateTime;
	aUser.LowPart = user.dwLowDateTime;
	aUser.HighPart = user.dwHighDateTime;
	Sleep(1000); //Esperamos antes de medir los datos de nuevo

	while (c != '\n')
	{
		GetSystemTimes(&idle, &kernel, &user); //Medimos los datos "nuevos"

		nIdle.LowPart = idle.dwLowDateTime; //Guardamos los datos "nuevos"
		nIdle.HighPart = idle.dwHighDateTime;
		nKernel.LowPart = kernel.dwLowDateTime;
		nKernel.HighPart = kernel.dwHighDateTime;
		nUser.LowPart = user.dwLowDateTime;
		nUser.HighPart = user.dwHighDateTime;

		tIdle.QuadPart = nIdle.QuadPart - aIdle.QuadPart; //Calculamos los tiempos "totales" restandole los
		tUser.QuadPart = nUser.QuadPart - aUser.QuadPart; //tiempos antiguos a los tiempos nuevos
		tKernel.QuadPart = nKernel.QuadPart - aKernel.QuadPart;

		//calculamos el uso del CPU
		tUtilCPU = (tKernel.QuadPart + tUser.QuadPart - tIdle.QuadPart) * 100;
		tTotalCPU = tKernel.QuadPart + tUser.QuadPart;
		calculoCPU = 1;
		usoMisDatos();
		calculoCPU = 0;

		aIdle.QuadPart = nIdle.QuadPart;
		aKernel.QuadPart = nKernel.QuadPart;
		aUser.QuadPart = nUser.QuadPart;

		Sleep(1000);
	}
}

void usoDeRam()
{
	MEMORYSTATUSEX memoria;
	std::thread actData;

	memoria.dwLength = sizeof(memoria);


	while (c != '\n')
	{
		GlobalMemoryStatusEx(&memoria);

		actualizoRAM = 1;//Indico que quiero actualizar los datos de mi ram
		actData = std::thread(usoMisDatos);
		while (actualizala == 0 && c!= '\n')//Mientras no me den la señal para empezar a actualizar
		{
			Sleep(5);
		}
		usada = (float)(memoria.ullTotalPhys - memoria.ullAvailPhys) / 1000000000;
		total = (float)memoria.ullTotalPhys / 1000000000;
		disponible = (float)memoria.ullAvailPhys / 1000000000;
		porciento = memoria.dwMemoryLoad;
		actualizoRAM = 0;
		actualizala = 0;
		Sleep(1000);//Espera antes de volver a leer los datos
		actData.join();
	}
}

void datagramas()
{
	unsigned int rAnt, rNvo, eAnt, eNvo;
	std::thread actGramas;
	MIB_IPSTATS *pStats;
	pStats = (MIB_IPSTATS *)malloc(sizeof(MIB_IPSTATS));

	//incializamos datos =D
	GetIpStatistics(pStats);
	rAnt = pStats->dwInReceives;
	eAnt = pStats->dwOutRequests;
	Sleep(1000); //esperamos para poder tomar datos de nuevo =D


	while (c != '\n')
	{
		GetIpStatistics(pStats);
		rNvo = pStats->dwInReceives;
		eNvo = pStats->dwOutRequests;

		actDatagramas = 1;
		actGramas = std::thread(usoMisDatos);
		while (okData == 0 && c!='\n')//Mientras no me den la señal de actualizar los datagramas
		{
			Sleep(5);
		}
		dTot = (float)(rNvo - rAnt)*(1.5);
		dEot = (float)(eNvo - eAnt)*(1.5);
		actDatagramas = 0;
		okData = 0;
		
		rAnt = rNvo;
		eAnt = eNvo;
		Sleep(500);
		actGramas.join();
	}
}

void imprime()
{

	printf("\n\n");
	barraCPU();
	printf("\tCPU:\n");
	barraCPU();
	printf("\tUso %3u%%\n\n", usoCPU);

	barraRAM();
	printf("\tRAM: %.2g GB usados [ %3u%% ]\n", usada, porciento);
	barraRAM();
	printf("\t%.2g GB disponibles de %.2g GB\n\n\t", disponible, total);


	if (dTot > 999) //decide de que forma imprimir la información sobre los datos recibidos por internet
	{
		barraRecibe();
		printf("\tInternet (datagramas):\n\t");
		barraRecibe();
		dTot = dTot / 1000;
		printf("\tRecibo: %6g Mbps\n\n\t", dTot);
	}
	else
	{
		barraRecibe();
		printf("\tInternet (datagramas):\n\t");
		barraRecibe();
		printf("\tRecibo: %6g kbps\n\n\t", dTot);
	}
	if (dEot > 999) //decide de que forma imprimir la información sobre los datos enviados por internet
	{
		barraEnvia();
		printf("\tInternet (datagramas):\n\t");
		barraEnvia();
		dEot = dEot / 1000;
		printf("\tEnvio: %6g Mbps\n\n\t", dEot);
	}
	else
	{
		barraEnvia();
		printf("\tInternet (datagramas):\n\t");
		barraEnvia();
		printf("\tEnvio: %6g kbps\n\n\t", dEot);
	}
	printf("Presiona ENTER para terminar el programa =D ");

}

void barraCPU() //imprime la barra de % CPU
{
	unsigned int i;
	if (usoCPU<33)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 34);//verde
	}
	else
	{
		if (usoCPU < 66)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 238);//amarillo
		}
		else
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 204);//rojo
		}
	}

	printf("\t");
	for (i = 0; i<(usoCPU / 5); i++)
	{
		printf(" ");
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	for (i = 0; i<20 - (usoCPU / 5); i++)
	{
		printf("%c", 176);
	}
}

void barraRAM() //imprime la barra de % de RAM
{
	unsigned int i;
	printf("\t");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 83);
	for (i = 0; i<porciento / 5; i++)
	{
		printf(" ");
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	for (i = 0; i<20 - (porciento / 5); i++)
	{
		printf("%c", 176);
	}
}

void barraRecibe() //imprime la barra de % de datos que recibe por internet
{
	int i;
	int auxTot;
	if (dTot > 999)//si son Megabytes
	{
		auxTot = (int)(dTot / 1000);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 153);
		if (auxTot > 20)
		{
			for (i = 0; i < 20; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		}
		else
		{
			for (i = 0; i < auxTot; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			for (i = 0; i < 20 - auxTot; i++)
			{
				printf("%c", 176);
			}
		}
	}
	else //si son kbytes
	{
		auxTot = (int)(dTot / 50);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 51);

		for (i = 0; i < auxTot; i++)
		{
			printf(" ");
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		for (i = 0; i < 20 - auxTot; i++)
		{
			printf("%c", 176);
		}

	}


}

void barraEnvia() //imprime la barra de % de datos que envia por internet
{
	int i;
	int auxTot;
	if (dEot > 999)//si son Megabytes
	{
		auxTot = (int)(dEot / 1000);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 153);
		if (auxTot > 20)
		{
			for (i = 0; i < 20; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		}
		else
		{
			for (i = 0; i < auxTot; i++)
			{
				printf(" ");
			}
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			for (i = 0; i < 20 - auxTot; i++)
			{
				printf("%c", 176);
			}
		}
	}
	else //si son kbytes
	{
		auxTot = (int)(dEot / 50);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 51);

		for (i = 0; i < auxTot; i++)
		{
			printf(" ");
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		for (i = 0; i < 20 - auxTot; i++)
		{
			printf("%c", 176);
		}

	}


}

void pideImpresion()
{
	while (c != '\n')
	{
		system("cls");
		imprimiendo = 1;
		usoMisDatos();
		Sleep(500);
	}
}

void usoMisDatos()
{
	std::lock_guard<std::mutex> bloquea(candado);
	if (imprimiendo == 1) //Si estoy imprimiendo
	{
		imprime();
		imprimiendo = 0;//ya no estoy imprimiendo
	}
	else //Si estoy actualizando valores
	{
		if (calculoCPU == 1)
		{
			usoCPU = tUtilCPU / tTotalCPU;
			calculoCPU = 0;
		}
		else
		{
			if (actualizoRAM == 1)
			{
				actualizala = 1;
				while (actualizoRAM == 1 && c!='\n') //Mantengo bloqueda la función mientras actualizo datos
				{
					Sleep(5);
				}
			}
			else
			{
				if (actDatagramas == 1)
				{
					okData = 1;
					while (actDatagramas == 1 && c!='\n') //Mantengo bloqueda la función mientras actualizo datos
					{
						Sleep(5);
					}
				}
			}
			
		}
		
	}
	
}
