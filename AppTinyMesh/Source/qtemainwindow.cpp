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

#include "Scenes/ArchScene.h"

#include <QtWidgets/QPushButton> // David
#include <QtWidgets/QLabel> // David
#include "SDF/SphereTraceRender.h" // David

//#include <memory>
//#include <utility>

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
	// Sphere tracing (button code-created)
	QPushButton* sphereTraceButton = new QPushButton("Sphere Tracing", uiw->Objects_groupBox);
	sphereTraceButton->setGeometry(140, 50, 101, 23);
	connect(sphereTraceButton, &QPushButton::clicked, this, &MainWindow::SphereTraceExample);
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
	implicit.Polygonize(156, implicitMesh, Box(3.5));

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