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

#ifndef _VtkVisualizationWidget_h_
#define _VtkVisualizationWidget_h_

#include <QtWidgets/QWidget>

#include "SVWidgetsLib/FilterParameterWidgets/FilterParameterWidget.h"
#include "SVWidgetsLib/Widgets/PopUpWidget.h"

#include "VtkSIMPL/VtkSupport/SIMPLVtkBridge.h"
//#include "VtkSIMPL/QtWidgets/VtkEditFiltersWidget.h"

// Qt5 Forward Declare
class QAction;
class QMenu;
class QPoint;
class QSignalMapper;

// Vtk Forwad Declare
class vtkRenderer;
class vtkOrientationMarkerWidget;
class vtkGenericOpenGLRenderWindow;

// SIMPLView Forward Declare
class VtkAbstractFilter;
class VtkMappingData;
class VtkToggleWidget;
class VtkRenderController;
class SVVtkRenderView;
class VtkEditFiltersWidget;

// SIMPLib Forward Declare
class AbstractFilter;


namespace Ui {
  class VtkVisualizationWidget;
}


/**
 * @class VtkVisualizationWidget VtkVisualizationWidget.h 
 * SIMPLView/VtkSIMPL/QtWidgets/VtkVisualizationWidget.h
 * @brief This class is used to visualize a DataContainerArray through VTK 
 * without the need to export the data to another program.
 */
class VtkVisualizationWidget : public QWidget
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  */
  VtkVisualizationWidget(QWidget* parent = nullptr);

  /**
  * @brief Deconstructor
  */
  virtual ~VtkVisualizationWidget();

  /**
  * @brief Performs initial setup work for the GUI
  */
  virtual void setupGui();

#if 0
  /**
  * @brief Adds a visualization filter
  * @param filter
  */
  void addVisualizationFilter(VtkAbstractFilter* filter);

#endif

public slots:
  /**
  * @brief Sets the DataContainerArray::Pointer to visualize
  * @param dca
  */
  void setDataContainerArray(DataContainerArray::Pointer dca);  

  /**
   * @brief on_editFiltersBtn_toggled
   * @param checked
   */
//  void on_editFiltersBtn_clicked();
  
  /**
   * @brief on_saveAsBtn_clicked
   */
  void on_saveAsBtn_clicked();


 #if 0
  /**
  * @brief Changes the DataSet to use
  * @param setId
  */
  void changeDataSet(int setId);

  /**
  * @brief Changes the DataArray to render
  * @param viewId
  */
  void changeDataView(int viewId);

  /**
  * @brief Changes the DataArray component to render
  * @param componentId
  */
  void changeDataViewComponent(int componentId);

  /**
  * @brief Sets whether or not scalars should be mapped for the active filter
  * @param state
  */
  void mapFilterScalars(int state);

  /**
  * @brief Sets whether or not the lookup table should be shown for the active filter
  */
  void showLookupTable(int state);

  
  /**
  * @brief Checks the vtkScalarBarWidget positions and visibility
  */
  void checkScalarMapping();

  void checkAxisWidget();

#endif

protected slots:

#if 0
  void filterExecuted(AbstractFilter* filter);

  /**
  * @brief Cleans and empties the VTK visualization window
  */
  void cleanupVisualizationWindow();

  /**
  * @brief Resets the camera distance from the VTK data set
  */
  void resetCamera();

  /**
  * @brief Forces a render of the VTK widget
  */
  void renderVtk();


  void addClipFilter();
  void addSliceFilter();
  void addMaskFilter();
#endif

protected:

#if 0
  /**
  * @brief Connects Qt signals and slots
  */
  void connectSlots();

  /**
  * @brief Disconnects Qt signals and slots
  */
  void disconnectSlots();

  /**
  * @brief Sets the active visualization filter
  * @param filter
  */
  void setActiveFilter(VtkAbstractFilter* filter);

  /**
  * @brief Clears the render window
  */
  void clearRenderWindow();

  /**
  * @brief Sets up the QComboBoxes for determining the data to display
  */
  void setupDataComboBoxes();


  /**
  * @brief Sets the DataArray component combo box for the given vtkDataArray
  * @param dataArray
  */
  void setViewComponentComboBox(vtkDataArray* dataArray);
#endif

  /**
  * @brief Opens a dialog box for saving the VTK visualization widget as a VTK file
  */
  void saveAsVTK();
  

  /**
  * @brief Saves a screenshot of the VTK visualization
  * @param fileName
  */
  void saveScreenshot(QString fileName);
  
private:
  Q_DISABLE_COPY(VtkVisualizationWidget)
  
  QScopedPointer<Ui::VtkVisualizationWidget> m_Ui;
  QScopedPointer<QSignalMapper>         m_SignalMapper;
  vtkNew<vtkGenericOpenGLRenderWindow> m_VtkOpenGLRenderWindow;
  vtkSmartPointer<SVVtkRenderView>     m_SVRenderView;
  vtkOrientationMarkerWidget*          OrientationWidget = nullptr;

  PopUpWidget*                        m_PopUpWidget = nullptr;
  
  VtkEditFiltersWidget*               m_EditFiltersWidget = nullptr;
  bool                                m_FirstRender = true;
#if 0
  VtkRenderControllerShPtr            m_RenderController;
  AbstractFilter*                     m_ActiveVisualizationFilter;
  DataContainerArray::Pointer         m_DataContainerArray;
  QMenu*                              m_contextMenu;
#endif

};

#endif /* _VtkVisualizationWidget_h_ */
