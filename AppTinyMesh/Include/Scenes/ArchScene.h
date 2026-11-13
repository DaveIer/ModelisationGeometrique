#pragma once

#include "SDF/Node.h"
#include <memory>

/**
 * X : largeur de l'arche.
 * Y : profondeur.
 * Z : hauteur.
 */
std::unique_ptr<Node> CreateArchScene();
