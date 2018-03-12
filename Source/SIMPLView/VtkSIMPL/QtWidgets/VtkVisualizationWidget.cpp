/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "VtkVisualizationWidget.h"

#include <QtWidgets/QAction>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QVector>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtCore/QSignalMapper>

#include <vtkAutoInit.h>
#include <vtkInteractionStyleModule.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
#include <vtkAxesActor.h>
#include <vtkBMPWriter.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkJPEGWriter.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPNGWriter.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderViewBase.h>
#include <vtkProperty.h>



//#include "VtkSIMPL/QtWidgets/VtkToggleWidget.h"
//#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkClipFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkMaskFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkSliceFilter.h"
#include "VtkSIMPL/VtkSupport/SVVtkRenderView.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.h"
//#include "VtkSIMPL/VtkSupport/VtkRenderController.h"

#include "ui_VtkVisualizationWidget.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkVisualizationWidget::VtkVisualizationWidget(QWidget* parent)
  : QWidget(parent)
  , m_Ui(new Ui::VtkVisualizationWidget)
  , m_SignalMapper(new QSignalMapper)
  //, m_PopUpWidget(new PopUpWidget())
  //, m_EditFiltersWidget(nullptr)
  //, m_RenderController(nullptr)
  //, m_DataContainerArray(nullptr)
{
  m_Ui->setupUi(this);
  
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkVisualizationWidget::~VtkVisualizationWidget()
{
  if(m_PopUpWidget) {
    m_PopUpWidget->deleteLater();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::setupGui()
{
  m_Ui->qvtkWidget->SetRenderWindow(m_VtkOpenGLRenderWindow.Get());
  m_SVRenderView = vtkSmartPointer<SVVtkRenderView>::New();
  
  m_SVRenderView->InitializeRenderView(m_VtkOpenGLRenderWindow.Get());
  
  m_Ui->showAxisBtn->setChecked(true);
  m_Ui->showCenterBtn->setChecked(true);
  
//  m_EditFiltersWidget = new VtkEditFiltersWidget(nullptr);
//  m_EditFiltersWidget->setRenderWindowInteractor(m_Ui->qvtkWidget->GetRenderWindow()->GetInteractor());
//  m_EditFiltersWidget->hide();
  
//  m_PopUpWidget = new PopUpWidget(nullptr);
//  m_PopUpWidget->setWidget(m_EditFiltersWidget);
//  m_PopUpWidget->hide();
  
  
  QObject::connect(
        m_Ui->camXPlusBtn, &QPushButton::clicked,
        [this]() { this->m_SVRenderView->setCameraPosition(SVVtkRenderView::CameraPosition::XPlus); }
  );
  
  QObject::connect(
        m_Ui->camXMinusBtn, &QPushButton::clicked,
        [this]() { this->m_SVRenderView->setCameraPosition(SVVtkRenderView::CameraPosition::XMinus); }
  );
  QObject::connect(
        m_Ui->camYPlusBtn, &QPushButton::clicked,
        [this]() { this->m_SVRenderView->setCameraPosition(SVVtkRenderView::CameraPosition::YPlus); }
  );
  QObject::connect(
        m_Ui->camYMinusBtn, &QPushButton::clicked,
        [this]() { this->m_SVRenderView->setCameraPosition(SVVtkRenderView::CameraPosition::YMinus); }
  );
  QObject::connect(
        m_Ui->camZPlusBtn, &QPushButton::clicked,
        [this]() { this->m_SVRenderView->setCameraPosition(SVVtkRenderView::CameraPosition::ZPlus); }
  );
  
  QObject::connect(
        m_Ui->camZMinusBtn, &QPushButton::clicked,
        [this]() { this->m_SVRenderView->setCameraPosition(SVVtkRenderView::CameraPosition::ZMinus); }
  );
  
  QObject::connect(
        m_Ui->showAxisBtn, &QPushButton::toggled,
        [this](bool b) { this->m_SVRenderView->SetOrientationAxesVisibility(b); }
  );
  
  QObject::connect(
        m_Ui->showCenterBtn, &QPushButton::toggled,
        [this](bool b) { this->m_SVRenderView->SetCenterAxesVisibility(b); }
  );   
}

#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::addVisualizationFilter(VtkAbstractFilter* filter)
{
  filter->setInteractor(qvtkWidget->GetRenderWindow()->GetInteractor());
  m_RenderController->addFilter(filter);
}

#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::setDataContainerArray(DataContainerArray::Pointer dca)
{ 
  if(nullptr == dca)
  {
    return;
  }
  //Initialize the Heirarchy of RenderView configurations
  DataSetRenderViewConfContainer dataSetRenderViewConfigs = SIMPLVtkBridge::WrapDataContainerArray(dca);
  
  m_SVRenderView->SetDataSetRenderViewConfigs(dataSetRenderViewConfigs);
  std::cout << "--------------------------------------------------" << std::endl;
  m_SVRenderView->SetGeometryVisible(0);
  if(m_FirstRender)
  {
    m_SVRenderView->setCameraPosition(SVVtkRenderView::CameraPosition::ZPlus);
    m_FirstRender = false;
  }
  else
  {
    m_Ui->qvtkWidget->GetRenderWindow()->Render();
  }

}
  
#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::filterExecuted(AbstractFilter* filter)
{
  
  m_ActiveVisualizationFilter = dynamic_cast<VtkVisualizationFilter*>(filter);
  if(m_ActiveVisualizationFilter)
  {
    setWindowTitle("[" + QString::number(filter->getPipelineIndex() + 1) + "] " + filter->getHumanLabel());
    m_LocalDataContainerArray = m_ActiveVisualizationFilter->getDataContainerArray();
    
    VtkAbstractFilter::setInteractor(qvtkWidget->GetRenderWindow()->GetInteractor());
    
    m_RenderController = SIMPLVtkBridge::WrapDataContainerArrayAsVtkRenderController(m_LocalDataContainerArray);
    m_RenderController->setVtkRenderControllerRenderer(m_Renderer);
    
    contextItem = nullptr;
    
    setupFilterTreeView();
    setupDataComboBoxes();
    createActions();
    
    render();
    show();
    this->setVisible(true);
    
    if(m_mappingData != nullptr)
    {
      toggleRenderProperties->setWidget(m_mappingData);
    }
    
    scrollAreaWidgetContents->adjustSize();
    
    connectSlots();
  }
  
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::connectSlots()
{
  // Add VTK Filter
  connect(clipBtn, SIGNAL(clicked()), 
          this, SLOT(addClipFilter()));
  connect(sliceBtn, SIGNAL(clicked()), 
          this, SLOT(addSliceFilter()));
  connect(maskBtn, SIGNAL(clicked()), 
          this, SLOT(addMaskFilter()));
  
  connect(viewScalarComboBox, SIGNAL(currentIndexChanged(int)), 
          this, SLOT(changeDataView(int)));
  connect(viewScalarComponentComboBox, SIGNAL(currentIndexChanged(int)), 
          this, SLOT(changeDataViewComponent(int)));
  
  connect(m_EditFiltersWidget, SIGNAL(renderVtk()),
          this, SLOT(renderVtk()), Qt::UniqueConnection);
  connect(m_EditFiltersWidget, SIGNAL(activeFilterChanged(VtkAbstractFilter*)),
          this, SLOT(setActiveFilter(VtkAbstractFilter*)), Qt::UniqueConnection);
  
  // Mapping data pass-through
  connect(m_EditFiltersWidget, SIGNAL(mapFilterScalars(int)),
          this, SLOT(mapFilterScalars(int)));
  connect(m_EditFiltersWidget, SIGNAL(showLookupTable(int)),
          this, SLOT(showLookupTable(int)));
  connect(m_EditFiltersWidget, SIGNAL(checkScalarMapping()),
          this, SLOT(checkScalarMapping()));
  
  connect(saveAsBtn, SIGNAL(clicked()), this, SLOT(saveAs()));
  
  connect(camXPlusBtn, SIGNAL(clicked()), this, SLOT(camXPlus()));
  connect(camXMinusBtn, SIGNAL(clicked()), this, SLOT(camXMinus()));
  
  connect(camYPlusBtn, SIGNAL(clicked()), this, SLOT(camYPlus()));
  connect(camYMinusBtn, SIGNAL(clicked()), this, SLOT(camYMinus()));
  
  connect(camZPlusBtn, SIGNAL(clicked()), this, SLOT(camZPlus()));
  connect(camZMinusBtn, SIGNAL(clicked()), this, SLOT(camZMinus()));
  
  connect(showAxisBtn, SIGNAL(clicked()), this, SLOT(checkAxisWidget()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::disconnectSlots()
{
  // Add VTK Filter
  disconnect(clipBtn, SIGNAL(clicked()), 
             this, SLOT(addClipFilter()));
  disconnect(sliceBtn, SIGNAL(clicked()), 
             this, SLOT(addSliceFilter()));
  disconnect(maskBtn, SIGNAL(clicked()), 
             this, SLOT(addMaskFilter()));
  
  disconnect(m_EditFiltersWidget, SIGNAL(renderVtk()),
             this, SLOT(renderVtk()));
  disconnect(m_EditFiltersWidget, SIGNAL(activeFilterChanged(VtkAbstractFilter*)),
             this, SLOT(setActiveFilter(VtkAbstractFilter*)));
  
  disconnect(viewScalarComboBox, SIGNAL(currentIndexChanged(int)), 
             this, SLOT(changeDataView(int)));
  disconnect(viewScalarComponentComboBox, SIGNAL(currentIndexChanged(int)), 
             this, SLOT(changeDataViewComponent(int)));
  
  // Mapping data pass-through
  disconnect(m_EditFiltersWidget, SIGNAL(mapFilterScalars(int)),
             this, SLOT(mapFilterScalars(int)));
  disconnect(m_EditFiltersWidget, SIGNAL(showLookupTable(int)),
             this, SLOT(showLookupTable(int)));
  disconnect(m_EditFiltersWidget, SIGNAL(checkScalarMapping()),
             this, SLOT(checkScalarMapping()));
  
  disconnect(saveAsBtn, SIGNAL(clicked()), this, SLOT(saveAs()));
  
  disconnect(camXPlusBtn, SIGNAL(clicked()), this, SLOT(camXPlus()));
  disconnect(camXMinusBtn, SIGNAL(clicked()), this, SLOT(camXMinus()));
  
  disconnect(camYPlusBtn, SIGNAL(clicked()), this, SLOT(camYPlus()));
  disconnect(camYMinusBtn, SIGNAL(clicked()), this, SLOT(camYMinus()));
  
  disconnect(camZPlusBtn, SIGNAL(clicked()), this, SLOT(camZPlus()));
  disconnect(camZMinusBtn, SIGNAL(clicked()), this, SLOT(camZMinus()));
  
  disconnect(showAxisBtn, SIGNAL(clicked()), this, SLOT(checkAxisWidget()));
}

void VtkVisualizationWidget::cleanupVisualizationWindow()
{
  disconnectSlots();
  clearRenderWindow();
  
  m_EditFiltersWidget->cleanupVisualization();
  
  if(nullptr != m_RenderController)
  {
    m_RenderController->clearFilters();
  }
  
  m_RenderController = VtkRenderController::NullPointer();
  m_DataContainerArray = DataContainerArray::NullPointer();
  m_ActiveVisualizationFilter = nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::clearRenderWindow()
{
  m_SVRenderView->GetRenderer()->RemoveAllViewProps();
  m_SVRenderView->GetRenderer()->Render();
  qvtkWidget->GetRenderWindow()->GetInteractor()->Render();
  m_SVRenderView->GetRenderer()->ResetCamera();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::setupDataComboBoxes()
{
  if(!m_RenderController)
  {
    return;
  }
  
  DataSetRenderViewConfContainer vtkDataStructs = m_RenderController->getVtkDataStructs();
  
  if(vtkDataStructs.size() > 0)
  {
    m_Ui->viewScalarComboBox->setCurrentIndex(0);
    changeDataSet(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::setActiveFilter(VtkAbstractFilter* filter)
{
  if(nullptr == m_RenderController || nullptr == filter)
  {
    return;
  }
  
  // update UI
  clipBtn->setEnabled(VtkAbstractFilter::compatibleInput(filter->getOutputType(), VtkClipFilter::getRequiredInputType()));
  sliceBtn->setEnabled(VtkAbstractFilter::compatibleInput(filter->getOutputType(), VtkSliceFilter::getRequiredInputType()));
  
  viewScalarComboBox->setCurrentIndex(filter->getViewScalarId());
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::changeDataSet(int /* setId */)
{
  SVVtkDataSetRenderViewConfShPtr dataSet = m_RenderController->getActiveDataSet();
  if(nullptr == dataSet.get())
  {
    return;
  }
  
  QList<QString> dataViewNameList;
  
  DataArrayRenderViewConfContainer dataViewConfigs = dataSet->GetDataArrayRenderViewConfigs();
  for(size_t i = 0; i < dataViewConfigs.size(); i++)
  {
    dataViewNameList.push_back(QString::fromStdString(dataViewConfigs.at(i)->getName()));
  }
  
  m_Ui->viewScalarComboBox->clear();
  m_Ui->viewScalarComboBox->addItems(dataViewNameList);
  
  if(dataViewConfigs.size() > 0)
  {
    m_RenderController->setDataViewActive(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::changeDataView(int viewId)
{
  if(nullptr == m_RenderController || viewId < 0)
  {
    return;
  }
  
  VtkAbstractFilter* activeFilter = m_RenderController->getActiveFilter();
  
  if(nullptr == activeFilter)
  {
    return;
  }
  
  m_RenderController->setActiveFilterScalars(viewId);
  setViewComponentComboBox(activeFilter->getBaseDataArray(viewId));
  
  qvtkWidget->GetRenderWindow()->GetInteractor()->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::changeDataViewComponent(int componentId)
{
  if(nullptr == m_RenderController)
  {
    return;
  }
  
  VtkAbstractFilter* activeFilter = m_RenderController->getActiveFilter();
  
  if(nullptr == activeFilter)
  {
    return;
  }
  
  m_RenderController->setActiveFilterComponent(componentId);
  
  qvtkWidget->GetRenderWindow()->GetInteractor()->Render();
  qvtkWidget->GetRenderWindow()->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::setViewComponentComboBox(vtkDataArray* dataArray)
{
  if(nullptr == dataArray)
  {
    return;
  }
  
  int numComponents = dataArray->GetNumberOfComponents();
  
  viewScalarComponentComboBox->clear();
  viewScalarComponentComboBox->setEnabled(numComponents > 1);
  
  if(numComponents > 1)
  {
    QList<QString> componentNameList;
    
    for(int i = 0; i < numComponents; i++)
    {
      if(dataArray->GetComponentName(i) != nullptr)
      {
        componentNameList.append(dataArray->GetComponentName(i));
      }
      else
      {
        componentNameList.append(QString::number(i + 1));
      }
    }
    
    componentNameList.append("Magnitude");
    
    viewScalarComponentComboBox->addItems(componentNameList);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::mapFilterScalars(int state)
{
  if(nullptr == m_RenderController)
  {
    return;
  }
  
  VtkAbstractFilter* activeFilter = m_RenderController->getActiveFilter();
  
  if(nullptr == activeFilter)
  {
    return;
  }
  
  bool shouldMap = state == Qt::Checked;
  
  activeFilter->setMapScalars(shouldMap);
  
  qvtkWidget->GetRenderWindow()->GetInteractor()->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::showLookupTable(int state)
{
  if(nullptr == m_RenderController)
  {
    return;
  }
  
  VtkAbstractFilter* activeFilter = m_RenderController->getActiveFilter();
  
  if(nullptr == activeFilter)
  {
    return;
  }
  
  bool showTable = state == Qt::Checked;
  
  activeFilter->setLookupTableActive(showTable);
  
  qvtkWidget->GetRenderWindow()->GetInteractor()->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::resetCamera()
{
  m_SVRenderView->GetRenderer()->ResetCamera();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::renderVtk()
{
  m_SVRenderView->GetInteractor()->Render();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::checkScalarMapping()
{
  m_SVRenderView->GetRenderController()->checkScalarBarWidgets();
  m_SVRenderView->GetInteractor()->Render();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::on_editFiltersBtn_clicked()
{
  QPoint targetPoint = m_Ui->editFiltersBtn->mapToGlobal(QPoint(0, m_Ui->editFiltersBtn->geometry().height()));
  
  m_PopUpWidget->setWidget(m_EditFiltersWidget);
  m_PopUpWidget->resize(m_EditFiltersWidget->sizeHint());
  m_PopUpWidget->move(targetPoint);
  m_PopUpWidget->show();
}
#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::on_saveAsBtn_clicked()
{
  QString filter = "VTK Data Object (*.vtk);;PNG Image Data (*.png);;JPEG Image Data (*jpg);;Bitmap Image Data(*bmp)";
  
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), filter);
  
  if(fileName.isEmpty())
  {
    return;
  }
  
  if(fileName.endsWith(".vtk"))
  {
    // m_RenderController->getActiveFilter()->saveFile(fileName);
  }
  else if(fileName.endsWith(".png") || fileName.endsWith(".jpg") || fileName.endsWith(".bmp"))
  {
    saveScreenshot(fileName);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::saveAsVTK()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("VTK Data Object (*.vtk)"));
  
  if(fileName.isEmpty())
  {
    return;
  }
  
  //m_RenderController->getActiveFilter()->saveFile(fileName);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::saveScreenshot(QString fileName)
{
  vtkSmartPointer<vtkWindowToImageFilter> screenshotFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  screenshotFilter->SetInput(m_SVRenderView->GetRenderWindow());
  //screenshotFilter->SetMagnification(1);
  screenshotFilter->SetInputBufferTypeToRGBA();
  screenshotFilter->ReadFrontBufferOff();
  screenshotFilter->Update();
  
  vtkSmartPointer<vtkImageWriter> imageWriter;
  
  if(fileName.endsWith(".png"))
  {
    imageWriter = vtkSmartPointer<vtkPNGWriter>::New();
  }
  else if(fileName.endsWith(".jpg"))
  {
    imageWriter = vtkSmartPointer<vtkJPEGWriter>::New();
  }
  else if(fileName.endsWith(".bmp"))
  {
    imageWriter = vtkSmartPointer<vtkBMPWriter>::New();
  }
  else
  {
    return;
  }
  
  imageWriter->SetFileName(fileName.toStdString().c_str());
  imageWriter->SetInputConnection(screenshotFilter->GetOutputPort());
  imageWriter->Write();
}

#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::addClipFilter()
{
  m_EditFiltersWidget->addClipFilter();
  on_editFiltersBtn_clicked();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::addSliceFilter()
{
  m_EditFiltersWidget->addSliceFilter();
  on_editFiltersBtn_clicked();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisualizationWidget::addMaskFilter()
{
  m_EditFiltersWidget->addMaskFilter();
  on_editFiltersBtn_clicked();
}
#endif

