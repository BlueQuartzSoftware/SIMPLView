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

#ifndef _vtkeditfilterswidget_h_
#define _vtkeditfilterswidget_h_

#include <QtWidgets/QWidget>

//#include "SIMPLView/VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

class vtkRenderWindowInteractor;


class VtkMappingData;
class VtkRenderController;
class VtkAbstractFilter;
using VtkRenderControllerShPtr = std::shared_ptr<VtkRenderController>;


#include "ui_VtkEditFiltersWidget.h"

/**
 * @class VtkEditFiltersWidget VtkEditFiltersWidget.h 
 * SIMPLView/VtkSIMPL/QtWidgets/VtkEditFiltersWidget.h
 * @brief This class allows the user to edit visibility filter values and 
 * visibility, and change the active visibility filter.  This widget is 
 * meant to be paired with VtkVisualizationWidget as a way to have more control
 * over data visualization.
 */
class VtkEditFiltersWidget : public QWidget, private Ui::VtkEditFiltersWidget
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  */
  VtkEditFiltersWidget(QWidget* parent);

  /**
  * @brief Deconstructor
  */
  ~VtkEditFiltersWidget();

  /**
  * @brief Sets the render controller to use
  * @param renderController
  */
  void setRenderController(VtkRenderControllerShPtr renderController);

  /**
  * @brief Sets the vtkRenderWindowInteractor to be used by new VtkDataSetFilters
  */
  void setRenderWindowInteractor(vtkRenderWindowInteractor* iren);

  /**
  * @brief Cleans up the visualization widget
  */
  void cleanupVisualization();

public slots:
  /**
  * @brief Adds a Clip visualization filter
  */
  void addClipFilter();

  /**
  * @brief Adds a Slice visualization filter
  */
  void addSliceFilter();

  /**
  * @brief Adds a Mask visualization filter
  */
  void addMaskFilter();

signals:
  /**
  * @brief Signals that a VTK render should be forced
  */
  void renderVtk();

  /**
  * @brief Signals that the active filter has been changed
  * @param newFilter
  */
  void activeFilterChanged(VtkAbstractFilter* newFilter);

  /**
  * @brief Signals that the DataSet has been changed
  * @param id
  */
  void dataSetChanged(int id);

  /**
  * @brief Pass-through signal for mapping data widget
  * @param checkboxValue
  */
  void mapFilterScalars(int checkboxValue);

  /**
  * @brief Pass-through signal for mapping data widget
  * @param checkboxValue
  */
  void showLookupTable(int checkboxValue);

  /**
  * @brief Pass-through signal for mapping data widget
  */
  void checkScalarMapping();

protected:
  /**
  * @brief Performs any initial setup for the GUI
  */
  void setupGui();

  /**
  * @brief Connects Qt signals and slots
  */
  void connectSlots();

  /**
  * @brief Disconnects Qt signals and slots
  */
  void disconnectSlots();

  /**
  * @brief Creates actions for the context menu
  */
  void createActions();

  /**
  * @brief Sets up the tree view of visibility filters
  */
  void setupFilterTreeView();
  
  /**
  * @brief Returns the selected QTreeWidgetItem for visibility filters
  * @return
  */
  QTreeWidgetItem* getSelectedFilterItem();

  /**
  * @brief Returns the visibilty filter matching the given QTreeWidgetItem
  * @param item
  * @return
  */
  VtkAbstractFilter* getFilter(QTreeWidgetItem* item);

  /**
  * @brief Returns teh QTreeWidgetItem matching the given visibility filter
  * @param filter
  * @return
  */
  QTreeWidgetItem* getTreeWidgetItem(VtkAbstractFilter* filter);

  /**
  * @brief Changes the checked state for a QTreeWidgetItem and all its children
  * @param item
  * @param checkParent
  */
  void uncheckFilterAndChildren(QTreeWidgetItem* item, bool checkParent = false);

  /**
  * @brief Changes the DataSet to match the given ID
  * @param setId
  */
  void changeDataSet(int setId);

protected slots:
  /**
  * @brief Applies changes to the active filter
  */
  void applyFilter();

  /**
  * @brief Resets changes to the active filter
  */
  void resetFilter();

  /**
  * @brief Deletes the active filter
  */
  void deleteFilter();

  /**
  * @brief Creates and opens a context menu at the given position
  * @param pos
  */
  void contextMenuActivated(const QPoint& pos);

  /**
  * @brief Adds a new filter item for the given visibility filter belonging 
  * to a parent QTreeWidgetItem
  * @param newFilter
  * @param selectedItem
  */
  void addFilterItem(VtkAbstractFilter* newFilter, QTreeWidgetItem* selectedItem);

  /**
  * @brief Checks the visibilty of a visibility filter belonging to the 
  * given QTreeWidgetItem
  * @param item
  * @param column
  */
  void filterToggled(QTreeWidgetItem* item, int column);

  /**
  * @brief Handles the change in visibility filter selection
  */
  void filterSelected();

  /**
  * @brief Handles the active filter being modified
  */
  void filterModified();

  /**
  * @brief Changes the active filter
  * @param filter
  */
  void setActiveFilter(VtkAbstractFilter* filter);

  /**
  * @brief Updates the scroll area size
  */
  void resizeScrollArea();

private:
  VtkRenderControllerShPtr m_VtkRenderController;

  VtkMappingData* m_MappingData = nullptr;
  QTreeWidgetItem* m_ContextItem = nullptr;
  QAction* m_addClipFilterAction = nullptr;
  QAction* m_addSliceFilterAction = nullptr;
  QAction* m_addMaskFilterAction = nullptr;
  QAction* m_deleteFilterAction = nullptr;
  vtkRenderWindowInteractor* m_renderInteractor = nullptr;
  
  
};

#endif
