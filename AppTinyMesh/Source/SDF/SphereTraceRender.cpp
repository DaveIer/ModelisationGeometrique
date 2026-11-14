#include "SDF/SphereTraceRender.h"

#include <algorithm>

QImage RenderSphereTrace(const Node& root, const Camera& camera, int width, int height, const SphereTraceParams& params)
{
    QImage image(width, height, QImage::Format_RGB32);

    const Vector light = Normalized(Vector(1.0, 1.0, 1.0));
    const QRgb background = qRgb(40, 40, 48);

    // Les pixels sont indépendants : active OpenMP (/openmp dans Visual Studio)
    // pour paralléliser sans autre modification. Sans OpenMP, le pragma est ignoré.
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));

        for (int x = 0; x < width; ++x) {
            const Ray ray = camera.PixelToRay(x, y, width, height);

            double t;
            Vector hit;

            if (!SphereTrace(root, ray, params, t, hit)) {
                line[x] = background;
                continue;
            }

            const Vector n = EstimateNormal(root, hit);
            // Éclairage (intensité entre 0.15 et 1)
            const double diffuse = std::max(0.0, n * light);   // '*' = produit scalaire
            const double shade = 0.15 + 0.85 * diffuse;

            /* Grey scale
            const int c = int(255.0 * std::min(shade, 1.0));
            line[x] = qRgb(c, c, c);
            */

            /* Use normale for color
            line[x] = qRgb(int(255.0 * (0.5 * n[0] + 0.5)),
                           int(255.0 * (0.5 * n[1] + 0.5)),
                           int(255.0 * (0.5 * n[2] + 0.5)));
            */
            

            // Couleur issue de la normale, ramenée dans [0, 1]
            const double r = 0.5 * n[0] + 0.5;
            const double g = 0.5 * n[1] + 0.5;
            const double b = 0.5 * n[2] + 0.5;

            line[x] = qRgb(int(255.0 * r * shade),
                           int(255.0 * g * shade),
                           int(255.0 * b * shade));
        }
    }

    return image;
}
