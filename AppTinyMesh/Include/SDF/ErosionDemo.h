#pragma once

#include <memory>

#include "SDF/ErosionNode.h"

// Une érosion de la sphère (taille de paquet donnée), puis affichage du rendu
// par sphere tracing et des statistiques dans une fenêtre.
// Retourne le nœud érodé, pour pouvoir le polygoniser et l'afficher dans le viewer.
std::unique_ptr<ErosionNode> ShowErosionRun(int batch_size, bool parallel);

// Banc d'essai complet (plusieurs tailles de paquet, séquentiel / parallèle,
// Value naïve / optimisée) : affiche le tableau et écrit erosion_benchmark.csv.
void ShowErosionBenchmark();
