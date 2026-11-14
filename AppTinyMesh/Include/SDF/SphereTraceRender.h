#pragma once

#include <QtGui/QImage>

#include "camera.h"
#include "SDF/Node.h"
#include "SDF/SphereTracing.h"

// Rend la scène SDF par sphere tracing : un rayon par pixel,
// ombrage de Lambert à partir de la normale estimée sur Value().
// params.t_max doit être adapté à l'échelle de la scène (distance caméra + taille de la scène).
QImage RenderSphereTrace(const Node& root, const Camera& camera, int width, int height,
                         const SphereTraceParams& params = SphereTraceParams());
