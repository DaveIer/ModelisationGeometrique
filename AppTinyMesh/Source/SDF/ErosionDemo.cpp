#include "SDF/ErosionDemo.h"

#include "SDF/Erosion.h"
#include "SDF/SphereNode.h"
#include "SDF/SphereTraceRender.h"
#include "camera.h"

#include <QtGui/QFontDatabase>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <memory>
#include <vector>

namespace {

// Paramètres communs à tous les essais : modifie-les ici.
ErosionParams DemoParams()
{
    ErosionParams p;
    p.ray_count = 3000; // 3000
    p.seed = 12345;
    p.center = Vector(0.0, 0.0, 0.0);
    p.scene_radius = 1.6;
    p.aim_radius = 1.0;
    p.radius_min = 0.02; // 0.02
    p.radius_max = 0.08; // 0.08
    p.depth_ratio = 0.6; // 0.6
    return p;
}

// Objet de départ : une sphère de rayon 1 centrée à l'origine.
std::unique_ptr<Node> MakeScene()
{
    return std::make_unique<SphereNode>(Vector(0.0, 0.0, 0.0), 1.0);
}

const int kVolumeSamples = 200000;
const unsigned int kVolumeSeed = 777;

// Fenêtre avec une image optionnelle et un texte à chasse fixe.
void ShowWindow(const QString& title, const QImage* image, const QString& text, int width, int height)
{
    QWidget* window = new QWidget;
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowTitle(title);

    QVBoxLayout* layout = new QVBoxLayout(window);

    if (image) {
        QLabel* label = new QLabel;
        label->setPixmap(QPixmap::fromImage(*image));
        layout->addWidget(label);
    }

    QPlainTextEdit* edit = new QPlainTextEdit;
    edit->setReadOnly(true);
    edit->setLineWrapMode(QPlainTextEdit::NoWrap);
    edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    edit->setPlainText(text);
    layout->addWidget(edit);

    window->resize(width, height);
    window->show();
}

} // namespace

std::unique_ptr<ErosionNode> ShowErosionRun(int batch_size, bool parallel)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ErosionParams params = DemoParams();
    params.batch_size = batch_size;
    params.parallel = parallel;
    params.optimized = true;

    auto eroded = std::make_unique<ErosionNode>(MakeScene());
    const ErosionStats stats = Erode(*eroded, params);

    const double removed = RemovedVolumePercent(MakeScene, *eroded, params.center, params.scene_radius,
                                                kVolumeSamples, kVolumeSeed);

    // Rendu (hors chronométrage) avec une caméra dédiée, cadrée sur la sphère.
    eroded->SetOptimized(true);
    const Camera camera(Vector(3.5, 0.0, 0.0), Vector(0.0, 0.0, 0.0));

    SphereTraceParams trace;
    trace.t_max = 5.0;

    const QImage image = RenderSphereTrace(*eroded, camera, 300, 300, trace);

    QApplication::restoreOverrideCursor();

    const QString title = (batch_size == 1)
        ? QString("Erosion incrementale")
        : QString("Erosion par paquets de %1").arg(batch_size);

    ShowWindow(title, &image, QString::fromStdString(FormatErosionStats(stats, removed)), 460, 640);

    return eroded;
}

void ShowErosionBenchmark()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const ErosionParams params = DemoParams();
    const int n = params.ray_count;

    // {taille de paquet, parallele, Value optimisee}
    const std::vector<ErosionConfig> configs = {
        { 1,    false, false },   // reference : incremental, Value naive
        { 1,    false, true  },   // incremental, Value optimisee
        { 10,   false, true  },
        { 100,  false, true  },
        { 1000, false, true  },
        { n,    false, true  },   // un seul paquet
        { 10,   true,  true  },
        { 100,  true,  true  },
        { 1000, true,  true  },
        { n,    true,  true  },
    };

    const std::string report = RunErosionBenchmark(MakeScene, params, configs, 3, kVolumeSamples,
                                                   "erosion_benchmark.csv");

    QApplication::restoreOverrideCursor();

    ShowWindow("Benchmark d'erosion", nullptr, QString::fromStdString(report), 1100, 420);
}
