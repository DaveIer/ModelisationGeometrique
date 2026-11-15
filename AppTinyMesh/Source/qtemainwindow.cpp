#include "qte.h"
#include "implicits.h"
#include "ui_interface.h"

#include "SDF/SphereNode.h"
#include "SDF/BoxNode.h"
#include "SDF/TorusNode.h"
#include "SDF/CapsuleNode.h"

#include "SDF/UnionNode.h"
#include "SDF/SmoothUnionPolynomialNode.h"
#include "SDF/SmoothUnionExpNode.h"

#include "SDF/IntersectionNode.h"
#include "SDF/DifferenceNode.h"
#include "Adapters/NodeFieldAdapter.h"
#include "SDF/TranslationNode.h"
#include "SDF/RotationNode.h"

#include "SDF/SphereTraceRender.h" // David

#include "Scenes/ArchScene.h"

#include <iostream>
#include <chrono>

#include <QtWidgets/QPushButton> // David
#include <QtWidgets/QLabel> // David
#include <QtWidgets/QFileDialog>
#include <QtCore/QDir>
#include <QtGui/QPainter>

// David
//#include <QtWidgets/QPushButton> 
//#include <QtWidgets/QLabel>
//#include <QtWidgets/QApplication>
#include "SDF/SphereTraceRender.h"
#include "SDF/ErosionDemo.h"
#include "SDF/ErosionNode.h"
#include <QtCore/QElapsedTimer>
//#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPlainTextEdit>
//#include <QtWidgets/QVBoxLayout>
#include <algorithm>
#include <vector>

#include <memory>
//#include <utility>

namespace
{
	// affiche un objet et ses volumes englobants
	void AddVisualizationMesh(MeshWidget* viewer, const QString& name, const Mesh& mesh, const Color& color, bool outlines = false)
	{
		std::vector<Color> colors(mesh.Vertexes(), color);

		MeshColor coloredMesh(mesh, colors, mesh.VertexIndexes());

		viewer->AddMesh(name, coloredMesh);
		viewer->SetMaterial(name, MeshMaterial::Color);

		if (outlines)
			viewer->SetShading(name, MeshShading::Lines);
	}
static void ShowTraceBenchmark(const Camera& camera)
{
	struct Entry
	{
		QString group;               // parametre etudie
		QString value;               // valeur testee
		QString note;                // remarque affichee sous l'image
		SphereTraceParams params;
		QImage image;
		double ms = 0.0;             // temps median du rendu
	};

	QApplication::setOverrideCursor(Qt::WaitCursor);

	auto scene = CreateArchScene();

	const double d = Norm(camera.Eye());   // distance a l'origine (la scene est autour de l'origine)

	SphereTraceParams base;                // defaut : epsilon = 1e-4, max_steps = 256
	base.t_max = d + 10.0;                 // comme le bouton "Sphere Tracing"

	std::vector<Entry> entries;
	auto add = [&](const QString& group, const QString& value, const QString& note, const SphereTraceParams& params)
		{
			Entry e;
			e.group = group;
			e.value = value;
			e.note = note;
			e.params = params;
			entries.push_back(e);
		};

	SphereTraceParams p;

	// epsilon : trop grand -> surface gonflee (environ epsilon) ; trop petit -> plus de pas
	p = base; p.epsilon = 0.3;  add("epsilon", "0.3", "trop grand", p);
	p = base; p.epsilon = 1e-2; add("epsilon", "1e-2", "", p);
	add("epsilon", "1e-4", "defaut", base);
	p = base; p.epsilon = 1e-8; add("epsilon", "1e-8", "tres petit", p);

	// t_max : trop petit -> la scene est coupee ; trop grand -> les rayons qui ratent vont plus loin
	p = base; p.t_max = 0.8 * d; add("t_max", QString::number(p.t_max, 'g', 3), "trop petit", p);
	p = base; p.t_max = 1.0 * d; add("t_max", QString::number(p.t_max, 'g', 3), "scene coupee", p);
	add("t_max", QString::number(base.t_max, 'g', 3), "defaut", base);
	p = base; p.t_max = 1000.0;  add("t_max", "1000", "tres grand", p);

	// max_steps : trop petit -> surface ratee ; trop grand -> aucun effet
	p = base; p.max_steps = 8;    add("max_steps", "8", "trop petit", p);
	p = base; p.max_steps = 32;   add("max_steps", "32", "", p);
	add("max_steps", "256", "defaut", base);
	p = base; p.max_steps = 1024; add("max_steps", "1024", "tres grand", p);

	// Mesure : mediane de plusieurs rendus complets (trace + ombrage), apres un rendu d'echauffement non chronometre.
	// Plus il y a de pixels et de repetitions, moins la mesure est bruitee.
	const int size = 300;
	const int repetitions = 9;

	for (Entry& e : entries)
	{
		RenderSphereTrace(*scene, camera, size, size, e.params);   // echauffement (non retenu)

		std::vector<double> times;
		for (int r = 0; r < repetitions; ++r)
		{
			QElapsedTimer timer;
			timer.start();
			e.image = RenderSphereTrace(*scene, camera, size, size, e.params);
			times.push_back(double(timer.nsecsElapsed()) / 1.0e6);
		}
		std::sort(times.begin(), times.end());
		e.ms = times[times.size() / 2];
	}

	QApplication::restoreOverrideCursor();

	// Fenetre : une ligne d'images par parametre, puis le tableau
	QWidget* window = new QWidget;
	window->setAttribute(Qt::WA_DeleteOnClose);
	window->setWindowTitle("Benchmark sphere tracing (arche)");
	QVBoxLayout* layout = new QVBoxLayout(window);

	layout->addWidget(new QLabel(QString("Temps = mediane de %1 rendus complets de %2x%2 pixels. Un seul parametre varie par ligne.").arg(repetitions).arg(size)));

	QGridLayout* grid = new QGridLayout;
	layout->addLayout(grid);

	QString table = "Parametre | Valeur | Remarque | Temps median (ms)\n";
	QString lastGroup;
	int row = -1;
	int column = 0;

	for (const Entry& e : entries)
	{
		if (e.group != lastGroup)
		{
			++row;
			column = 0;
			lastGroup = e.group;
		}

		QLabel* image = new QLabel;
		image->setPixmap(QPixmap::fromImage(e.image));

		QString caption = QString("%1 = %2").arg(e.group).arg(e.value);
		if (!e.note.isEmpty())
			caption += " (" + e.note + ")";
		caption += QString("\n%1 ms").arg(e.ms, 0, 'f', 1);

		QLabel* text = new QLabel(caption);
		text->setAlignment(Qt::AlignHCenter);

		QVBoxLayout* cell = new QVBoxLayout;
		cell->addWidget(image);
		cell->addWidget(text);
		grid->addLayout(cell, row, column++);

		table += QString("%1 | %2 | %3 | %4\n").arg(e.group, e.value, e.note).arg(e.ms, 0, 'f', 1);
	}

	QPlainTextEdit* tableView = new QPlainTextEdit;
	tableView->setReadOnly(true);
	tableView->setPlainText(table);
	tableView->setMaximumHeight(220);
	layout->addWidget(tableView);

	window->show();
}
}

MainWindow::MainWindow() : QMainWindow(), uiw(new Ui::Assets)
{
	// Chargement de l'interface
	uiw->setupUi(this);

	// Chargement du GLWidget
	meshWidget = new MeshWidget;
	QGridLayout* GLlayout = new QGridLayout;
	GLlayout->addWidget(meshWidget, 0, 0);
	GLlayout->setContentsMargins(0, 0, 0, 0);
	uiw->widget_GL->setLayout(GLlayout);

	// Creation des connect
	CreateActions();

	meshWidget->SetCamera(Camera(Vector(10, 0, 0), Vector(0.0, 0.0, 0.0)));
}

MainWindow::~MainWindow()
{
	delete meshWidget;
}

void MainWindow::CreateActions()
{
	// Buttons
	connect(uiw->boxMesh, SIGNAL(clicked()), this, SLOT(BoxMeshExample()));
	connect(uiw->sphereImplicit, SIGNAL(clicked()), this, SLOT(SphereImplicitExample()));
	connect(uiw->sphereNode, SIGNAL(clicked()), this, SLOT(SphereNodeExample())); // SphereNode
	connect(uiw->boxNode, SIGNAL(clicked()), this, SLOT(BoxNodeExample())); // BoxNode
	connect(uiw->torusNode, SIGNAL(clicked()), this, SLOT(TorusNodeExample())); // TorusNode
	connect(uiw->capsuleNode, SIGNAL(clicked()), this, SLOT(CapsuleNodeExample())); // CapsuleNode
	connect(uiw->unionNode, SIGNAL(clicked()), this, SLOT(UnionNodeExample())); // UnionNode
	connect(uiw->blendPolyNode, SIGNAL(clicked()), this, SLOT(SmoothUnionPolynomialNodeExample())); // SmoothUnionPolynomialNode
	connect(uiw->blendExpNode, SIGNAL(clicked()), this, SLOT(SmoothUnionExpNodeExample())); // SmoothUnionExpNode
	connect(uiw->intersectionNode, SIGNAL(clicked()), this, SLOT(IntersectionNodeExample())); // IntersectionNode
	connect(uiw->differenceNode, SIGNAL(clicked()), this, SLOT(DifferenceNodeExample())); // DifferenceNode
	connect(uiw->translationNode, SIGNAL(clicked()), this, SLOT(TranslationNodeExample())); // TranslationNode
	connect(uiw->rotationNode, SIGNAL(clicked()), this, SLOT(RotationNodeExample())); // RotationNode
	connect(uiw->archScene, SIGNAL(clicked()), this, SLOT(ArchSceneExample())); // Arch Scene
	connect(uiw->resetcameraButton, SIGNAL(clicked()), this, SLOT(ResetCamera()));
	connect(uiw->wireframe, SIGNAL(clicked()), this, SLOT(UpdateMaterial()));
	connect(uiw->radioShadingButton_1, SIGNAL(clicked()), this, SLOT(UpdateMaterial()));
	connect(uiw->radioShadingButton_2, SIGNAL(clicked()), this, SLOT(UpdateMaterial()));

	// Widget edition
	connect(meshWidget, SIGNAL(_signalEditSceneLeft(const Ray&)), this, SLOT(editingSceneLeft(const Ray&)));
	connect(meshWidget, SIGNAL(_signalEditSceneRight(const Ray&)), this, SLOT(editingSceneRight(const Ray&)));

	// David
	// Sphere tracing
	QPushButton* sphereTraceButton = new QPushButton("Sphere Tracing", uiw->Objects_groupBox);
	sphereTraceButton->setGeometry(140, 50, 101, 23);
	connect(sphereTraceButton, &QPushButton::clicked, this, &MainWindow::SphereTraceExample);

	// ===============
	// arche + volumes englobants
	QPushButton* archBoundsButton = new QPushButton("Arch + Bounds", uiw->Objects_groupBox);
	archBoundsButton->setGeometry(140, 170, 101, 23);

	connect(archBoundsButton, &QPushButton::clicked, this, [this]()
		{
			auto arch = CreateArchScene();
			NodeFieldAdapter implicit(*arch);

			Mesh archMesh;
			implicit.Polygonize(96, archMesh, Box(Vector(-1.7, -0.8, -0.4), Vector(1.7, 0.8, 3.5)));

			Mesh pillarsBounds(Box(Vector(-1.38, -0.52, 0.0), Vector(1.38, 0.52, 2.05)));
			Mesh vaultBounds(Box(Vector(-1.5, -0.4, 2.0), Vector(1.5, 0.4, 3.5)));

			meshWidget->ClearAll();

			AddVisualizationMesh(meshWidget, "arch", archMesh, Color(0.8, 0.8, 0.8));
			AddVisualizationMesh(meshWidget, "pillars_bounds", pillarsBounds, Color(0.1, 0.4, 1.0), true);
			AddVisualizationMesh(meshWidget, "vault_bounds", vaultBounds, Color(1.0, 0.3, 0.1), true);

			meshWidget->SetCamera(Camera(Vector(5.0, -8.0, 4.0), Vector(0.0, 0.0, 1.5)));

		});

	// ===============
	// 2 groupes de 4 spheres + volumes englobants (boite + sphere)
	auto showGroups = [this](int mode)
		{
			std::unique_ptr<Node> groups;

			for (double centerX : {-1.0, 1.0})
			{
				for (double dx : {-0.25, 0.25})
				{
					for (double dy : {-0.25, 0.25})
					{
						for (double dz : {-0.25, 0.25})
						{
							auto sphere = std::make_unique<SphereNode>(Vector(centerX + dx, dy, dz), 0.3);

							if (!groups)
								groups = std::move(sphere);
							else
								groups = std::make_unique<UnionNode>(std::move(groups), std::move(sphere));
						}
					}
				}
			}

			NodeFieldAdapter implicit(*groups);
			Mesh groupsMesh;

			implicit.Polygonize(64,groupsMesh,Box(Vector(-1.8, -0.8, -0.8), Vector(1.8, 0.8, 0.8)));

			meshWidget->ClearAll();

			AddVisualizationMesh(meshWidget, "groups", groupsMesh, Color(0.8, 0.8, 0.8));

			if (mode == 1) // boites englobantes
			{
				Mesh leftBox(Box(Vector(-1.55, -0.55, -0.55), Vector(-0.45, 0.55, 0.55)));
				Mesh rightBox(Box(Vector(0.45, -0.55, -0.55), Vector(1.55, 0.55, 0.55)));

				AddVisualizationMesh(meshWidget, "left_box", leftBox, Color(0.1, 0.4, 1.0), true);
				AddVisualizationMesh(meshWidget, "right_box", rightBox, Color(1.0, 0.3, 0.1), true);
			}
			else if (mode == 2) // spheres englobantes
			{
				const double radius = 0.25 * std::sqrt(3.0) + 0.3;

				SphereNode leftSphere(Vector(-1.0, 0.0, 0.0), radius);
				SphereNode rightSphere(Vector(1.0, 0.0, 0.0), radius);

				NodeFieldAdapter leftField(leftSphere);
				NodeFieldAdapter rightField(rightSphere);

				Mesh leftMesh;
				Mesh rightMesh;

				leftField.Polygonize(10, leftMesh, Box(Vector(-1.8, -0.8, -0.8), Vector(-0.2, 0.8, 0.8)));
				rightField.Polygonize(10, rightMesh, Box(Vector(0.2, -0.8, -0.8), Vector(1.8, 0.8, 0.8)));

				AddVisualizationMesh(meshWidget, "left_sphere", leftMesh, Color(0.1, 0.4, 1.0), true);
				AddVisualizationMesh(meshWidget, "right_sphere", rightMesh, Color(1.0, 0.3, 0.1), true);
			}

			meshWidget->SetCamera(Camera(Vector(4.0, -6.0, 2.5), Vector(0.0, 0.0, 0.0)));
		};

	// boutons des trois configurations
	auto addGroupsButton = [this, showGroups](
		const QString& label, int y, int mode)
		{
			QPushButton* button = new QPushButton(label, uiw->Objects_groupBox);
			button->setGeometry(140, y, 135, 23);

			connect(button, &QPushButton::clicked, this, [showGroups, mode]() { showGroups(mode); });
		};

	addGroupsButton("Groups Only", 200, 0);
	addGroupsButton("Groups + Boxes", 230, 1);
	addGroupsButton("Groups + Spheres", 260, 2);

	// ===============
	// 2 spheres + volumes englobants (boites)
	auto showTwoSpheres = [this](bool showBounds)
		{
			UnionNode spheres(
				std::make_unique<SphereNode>(Vector(-1.0, 0.0, 0.0), 0.5),
				std::make_unique<SphereNode>(Vector(1.0, 0.0, 0.0), 0.5)
			);

			NodeFieldAdapter implicit(spheres);
			Mesh spheresMesh;

			implicit.Polygonize(64, spheresMesh, Box(Vector(-1.7, -0.7, -0.7), Vector(1.7, 0.7, 0.7)));

			meshWidget->ClearAll();

			AddVisualizationMesh(meshWidget, "two_spheres", spheresMesh, Color(0.8, 0.8, 0.8));

			if (showBounds)
			{
				Mesh leftBox(Box(Vector(-1.5, -0.5, -0.5), Vector(-0.5, 0.5, 0.5)));
				Mesh rightBox(Box(Vector(0.5, -0.5, -0.5), Vector(1.5, 0.5, 0.5)));

				AddVisualizationMesh(meshWidget, "left_box", leftBox, Color(0.1, 0.4, 1.0), true);
				AddVisualizationMesh(meshWidget, "right_box", rightBox, Color(1.0, 0.3, 0.1), true);
			}

			meshWidget->SetCamera(Camera(Vector(4.0, -6.0, 2.5), Vector(0.0, 0.0, 0.0)));
		};

	QPushButton* twoSpheresButton = new QPushButton("2 Spheres Only", uiw->Objects_groupBox);
	twoSpheresButton->setGeometry(140, 290, 135, 23);
	connect(twoSpheresButton, &QPushButton::clicked, this, [showTwoSpheres]() { showTwoSpheres(false); });

	QPushButton* twoSpheresBoundsButton = new QPushButton("2 Spheres + Boxes", uiw->Objects_groupBox);
	twoSpheresBoundsButton->setGeometry(140, 320, 135, 23);
	connect(twoSpheresBoundsButton, &QPushButton::clicked, this, [showTwoSpheres]() { showTwoSpheres(true); });

	// ===============
	// sphere tracing classique vs accéléré
	QPushButton* compareTracingButton = new QPushButton("ST Comparison", uiw->Objects_groupBox);

	compareTracingButton->setGeometry(140, 350, 135, 23);

	connect(compareTracingButton, &QPushButton::clicked, this, [this]()
		{
			const QString folder = QFileDialog::getExistingDirectory(this, "Dossier des images Sphere Tracing");

			if (folder.isEmpty())
				return;

			auto classic = CreateArchScene();
			auto accelerated = CreateArchSceneAccelerated();

			Camera camera(Vector(4.0, -6.0, 3.4), Vector(0.0, 0.0, 1.5));

			SphereTraceParams params;
			params.t_max = 20.0;

			constexpr int width = 512;
			constexpr int height = 512;

			QImage classicImage = RenderSphereTrace(*classic, camera, width, height, params);
			QImage acceleratedImage = RenderSphereTrace(*accelerated, camera, width, height, params);

			// sphere tracing classique a gauche et accéléré à droite
			QImage comparison(2 * width, height, QImage::Format_RGB32);

			QPainter painter(&comparison);
			painter.drawImage(0, 0, classicImage);
			painter.drawImage(width, 0, acceleratedImage);
			painter.end();

			QDir output(folder);

			const bool saved =
				classicImage.save(output.filePath("arche_st_classique.png")) &&
				acceleratedImage.save(output.filePath("arche_st_acceleree.png")) &&
				comparison.save(output.filePath("arche_st_comparaison.png"));

			std::cout << "Sphere Tracing images saved: " << (saved ? "yes" : "no") << std::endl;

			QLabel* label = new QLabel;
			label->setWindowTitle("Sphere Tracing - Classic (left) / Accelerated (right)");
			label->setAttribute(Qt::WA_DeleteOnClose);
			label->setPixmap(QPixmap::fromImage(comparison));
			label->show();
		});
	uiw->Objects_groupBox->setMinimumHeight(510);
	
	QPushButton* traceBenchButton = new QPushButton("Benchmark sphere tracing", uiw->Objects_groupBox);
	traceBenchButton->setGeometry(140, 380, 140, 23);
	connect(traceBenchButton, &QPushButton::clicked, this, [this] { ShowTraceBenchmark(meshWidget->GetCamera()); });

	QPushButton* erosionIncButton = new QPushButton("Erosion incrementale", uiw->Objects_groupBox);
	erosionIncButton->setGeometry(140, 410, 140, 23);
	connect(erosionIncButton, &QPushButton::clicked, this, [this] { ErosionExample(1, false); });

	QPushButton* erosionBatchButton = new QPushButton("Erosion par paquets", uiw->Objects_groupBox);
	erosionBatchButton->setGeometry(140, 440, 140, 23);
	connect(erosionBatchButton, &QPushButton::clicked, this, [this] { ErosionExample(3000, false); });

	QPushButton* erosionBenchButton = new QPushButton("Benchmark erosion", uiw->Objects_groupBox);
	erosionBenchButton->setGeometry(140, 470, 140, 23);
	connect(erosionBenchButton, &QPushButton::clicked, this, [] { ShowErosionBenchmark(); });
}

void MainWindow::editingSceneLeft(const Ray&)
{}

void MainWindow::editingSceneRight(const Ray&)
{}

void MainWindow::BoxMeshExample()
{
	Mesh boxMesh = Mesh(Box(1.0));

	std::vector<Color> cols;
	cols.resize(boxMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(double(i) / 6.0, fmod(double(i) * 39.478378, 1.0), 0.0);

	meshColor = MeshColor(boxMesh, cols, boxMesh.VertexIndexes());
	UpdateGeometry();
}

void MainWindow::SphereImplicitExample()
{
	AnalyticScalarField implicit;

	Mesh implicitMesh;
	implicit.Polygonize(31, implicitMesh, Box(2.0));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Imene - EXAMPLES

/* -------- - Primitives examples-------- */

// Sphere Node
void MainWindow::SphereNodeExample()
{
	SphereNode sphere(Vector(0.0, 0.0, 0.0), 1.0);
	NodeFieldAdapter implicit(sphere);

	Mesh implicitMesh;
	implicit.Polygonize(128, implicitMesh, Box(2.0));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Box Node
void MainWindow::BoxNodeExample()
{
	BoxNode box(Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
	NodeFieldAdapter implicit(box);

	Mesh implicitMesh;
	implicit.Polygonize(128, implicitMesh, Box(2.0));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Torus Node
void MainWindow::TorusNodeExample()
{
	TorusNode torus(Vector(0.0, 0.0, 0.0), 1.0, 0.35);
	NodeFieldAdapter implicit(torus);

	Mesh implicitMesh;
	implicit.Polygonize(32, implicitMesh, Box(2.2));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// CapsuleNode
void MainWindow::CapsuleNodeExample()
{
	CapsuleNode capsule(Vector(-1.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0), 0.5);
	NodeFieldAdapter implicit(capsule);

	Mesh implicitMesh;
	implicit.Polygonize(128, implicitMesh, Box(2.0));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

/* -------- Boolean operations examples -------- */

// Union Node
void MainWindow::UnionNodeExample()
{
	auto sphere = std::make_unique<SphereNode>(Vector(-0.4, 0.0, 0.0), 1.0);
	auto box = std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55));
	//auto capsule = std::make_unique<CapsuleNode>(Vector(0.0, 0.0, 0.0), Vector(1.0, 1.0, 1.0), 0.2);
	//auto torus = std::make_unique<TorusNode>(Vector(0.0, 0.0, 0.0), 1.0, 0.2);
	
	//auto unionSphereBox = std::make_unique<UnionNode>(std::move(sphere), std::move(box));
	//auto unionTorusCapsule = std::make_unique<UnionNode>(std::move(capsule), std::move(torus));

	//UnionNode unionNode(std::move(unionSphereBox), std::move(unionTorusCapsule));
	
	UnionNode unionNode(std::move(sphere), std::move(box));
	
	NodeFieldAdapter implicit(unionNode);

	Mesh implicitMesh;
	implicit.Polygonize(64, implicitMesh, Box(2.5));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Smooth Union Polynomial Node
void MainWindow::SmoothUnionPolynomialNodeExample()
{
	auto sphere = std::make_unique<SphereNode>(Vector(-0.4, 0.0, 0.0), 1.0);
	auto box = std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55));

	SmoothUnionPolynomialNode blendNode(std::move(sphere), std::move(box), 1.2);

	NodeFieldAdapter implicit(blendNode);

	Mesh implicitMesh;
	implicit.Polygonize(64, implicitMesh, Box(2.5));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Smooth Union Exp Node
void MainWindow::SmoothUnionExpNodeExample()
{
	auto sphere = std::make_unique<SphereNode>(Vector(-0.4, 0.0, 0.0), 1.0);
	auto box = std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55));

	SmoothUnionExpNode blendNode(std::move(sphere), std::move(box), 1.2);

	NodeFieldAdapter implicit(blendNode);

	Mesh implicitMesh;
	implicit.Polygonize(64, implicitMesh, Box(2.5));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Intersection Node
void MainWindow::IntersectionNodeExample()
{
	auto sphere = std::make_unique<SphereNode>(Vector(-0.4, 0.0, 0.0), 1.0);
	auto box = std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55));

	IntersectionNode intersectionNode(std::move(sphere), std::move(box));

	NodeFieldAdapter implicit(intersectionNode);

	Mesh implicitMesh;
	implicit.Polygonize(64, implicitMesh, Box(2.5));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Difference Node
void MainWindow::DifferenceNodeExample()
{
	auto sphere = std::make_unique<SphereNode>(Vector(1.0, 0.0, 0.0), 1.0);
	auto box = std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55));

	DifferenceNode differenceNode(std::move(box), std::move(sphere));

	NodeFieldAdapter implicit(differenceNode);

	Mesh implicitMesh;
	implicit.Polygonize(64, implicitMesh, Box(2.5));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Translation Node
void MainWindow::TranslationNodeExample()
{
	auto box = std::make_unique<BoxNode>(Vector(-0.8, -0.3, -0.3), Vector(0.8, 0.3, 0.3));

	TranslationNode translated(std::move(box), Vector(1.0, 0.0, 0.0));

	NodeFieldAdapter implicit(translated);

	Mesh implicitMesh;
	implicit.Polygonize(64, implicitMesh, Box(2.5));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

// Rotation Node
void MainWindow::RotationNodeExample()
{
	auto box = std::make_unique<BoxNode>(Vector(-0.8, -0.3, -0.3), Vector(0.8, 0.3, 0.3));

	RotationNode rotated(std::move(box), Vector(0.0, 0.0, 1.0), Math::DegreeToRadian(45.0));

	NodeFieldAdapter implicit(rotated);

	Mesh implicitMesh;
	implicit.Polygonize(64, implicitMesh, Box(2.5));

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());
	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();
}

/* ---------------- Arch Scene---------------- */
void MainWindow::ArchSceneExample()
{
	auto arch = CreateArchScene();
	NodeFieldAdapter implicit(*arch);

	Mesh implicitMesh;

	auto start = std::chrono::steady_clock::now();

	implicit.Polygonize(96, implicitMesh, Box(Vector(-1.7, -0.8, -0.4), Vector(1.7, 0.8, 3.5)));

	auto end = std::chrono::steady_clock::now();

	std::chrono::duration<double> duration = end - start;

	std::cout << "Polygonization time: " << duration.count() << " s\n";

	std::vector<Color> cols;
	cols.resize(implicitMesh.Vertexes());

	for (size_t i = 0; i < cols.size(); i++)
		cols[i] = Color(0.8, 0.8, 0.8);

	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());

	UpdateGeometry();
}

// David
void MainWindow::SphereTraceExample()
{
	auto scene = CreateArchScene();
	const Camera& camera = meshWidget->GetCamera();

	SphereTraceParams params;
	params.t_max = Norm(camera.Eye()) + 10.0;   // distance à l'origine + rayon de la scène

	// Demi-résolution : plus rapide, le cadrage reste identique
	const int w = meshWidget->width() / 2;
	const int h = meshWidget->height() / 2;

	QImage image = RenderSphereTrace(*scene, camera, w, h, params);

	QLabel* label = new QLabel;
	label->setWindowTitle("Sphere tracing");
	label->setAttribute(Qt::WA_DeleteOnClose);
	label->setPixmap(QPixmap::fromImage(image));
	label->show();
}

void MainWindow::UpdateGeometry()
{
	meshWidget->ClearAll();
	meshWidget->AddMesh("BoxMesh", meshColor);

	uiw->lineEdit->setText(QString::number(meshColor.Vertexes()));
	uiw->lineEdit_2->setText(QString::number(meshColor.Triangles()));

	UpdateMaterial();
}

void MainWindow::ErosionExample(int batchSize, bool parallel)
{
	// Érosion + fenêtre de rendu et de statistiques
	std::unique_ptr<ErosionNode> eroded = ShowErosionRun(batchSize, parallel);
	QApplication::processEvents();   // laisse la fenêtre s'afficher avant le calcul du mesh

	// Le même objet érodé, polygonisé pour le viewer (rotation à la souris)
	QApplication::setOverrideCursor(Qt::WaitCursor);

	NodeFieldAdapter implicit(*eroded);

	Mesh implicitMesh;
	implicit.Polygonize(128, implicitMesh, Box(1.05));

	std::vector<Color> cols(implicitMesh.Vertexes(), Color(0.8, 0.8, 0.8));
	meshColor = MeshColor(implicitMesh, cols, implicitMesh.VertexIndexes());
	UpdateGeometry();

	QApplication::restoreOverrideCursor();
}

void MainWindow::UpdateMaterial()
{
	meshWidget->UseWireframeGlobal(uiw->wireframe->isChecked());

	if (uiw->radioShadingButton_1->isChecked())
		meshWidget->SetMaterialGlobal(MeshMaterial::Normal);
	else
		meshWidget->SetMaterialGlobal(MeshMaterial::Color);
}

void MainWindow::ResetCamera()
{
	meshWidget->SetCamera(Camera(Vector(-10.0), Vector(0.0)));
}