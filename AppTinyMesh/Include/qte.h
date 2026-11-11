#ifndef __Qte__
#define __Qte__

#include <QtWidgets/qmainwindow.h>
#include "realtime.h"
#include "meshcolor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Assets; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	Ui::Assets* uiw;           //!< Interface

	MeshWidget* meshWidget;   //!< Viewer
	MeshColor meshColor;		//!< Mesh.

public:
	MainWindow();
	~MainWindow();
	void CreateActions();
	void UpdateGeometry();

public slots:
	void editingSceneLeft(const Ray&);
	void editingSceneRight(const Ray&);
	void BoxMeshExample();
	void SphereImplicitExample();
	void ResetCamera();
	void UpdateMaterial();

	// Imene
	// primitives examples
	void SphereNodeExample();
	void BoxNodeExample();
	void TorusNodeExample();
	void CapsuleNodeExample();

	// simple operations examples
	void UnionNodeExample();
	void IntersectionNodeExample();
	void DifferenceNodeExample();
	
	// smooth union examples
	void SmoothUnionPolynomialNodeExample();
	void SmoothUnionExpNodeExample();
};

#endif
