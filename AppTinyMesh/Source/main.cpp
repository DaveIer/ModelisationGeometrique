#include "qte.h"
#include <QtWidgets/qapplication.h>

#include "Benchmarks/SDFBenchmark.h"
#include "Benchmarks/AccelerationBenchmark.h"

#include <cstring>
#include <iostream>

int main(int argc, char *argv[])
{
	// Mode Benchmark - pour lancer les 10^7 appels de fonction sans l'interface Qt
	if (argc > 1 && std::strcmp(argv[1], "--benchmark-sdf") == 0)
	{
		RunSDFBenchmarks();
		
		std::cout << "Appuyer sur Entree pour quitter la console." << std::endl;
		std::cin.get();

		return 0;
	}

	if (argc > 1 && std::strcmp(argv[1], "--benchmark-acceleration") == 0)
	{
		RunAccelerationBenchmarks();

		std::cout << "Appuyer sur Entree pour quitter la console." << std::endl;
		std::cin.get();
		
		return 0;
	}

	// Mode de base - interface Qt
	QApplication app(argc, argv);

	MainWindow mainWin;
	mainWin.showMaximized();

	return app.exec();
}
