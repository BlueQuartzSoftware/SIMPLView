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

#ifndef _vtkvisiblefilters_h_
#define _vtkvisiblefilters_h_

#include <QtWidgets/QTreeWidget>



class QAction;

/**
 * @class VtkVisibleFilters VtkVisibleFilters.h 
 * SIMPLView/VtkSIMPL/QtWidgets/VtkVisibleFilters.h
 * @brief This class handles the VTK filters displayed in the QTreeWidget
 * and allows the user to toggle their visibility as well as select an active
 * filter to view and edit.
 */
class VtkVisibleFilters : public QTreeWidget
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  * @param renderController
  */
  VtkVisibleFilters(QWidget* parent, VtkRenderController::Pointer renderController);

  /**
  * @brief Deconstructor
  */
  ~VtkVisibleFilters();

  /**
  * @brief Sets the VtkRenderController to use
  * @param renderController
  */
  void setRenderController(VtkRenderController::Pointer renderController);

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
  void render();

protected:
  /**
  * @brief Performs any initial work to setup the GUI
  */
  void setupGui();

  /**
  * @brief Returns the visualization filter for the given QTreeWidgetItem
  * @param item
  * @return
  */
  VtkAbstractFilter* getFilter(QTreeWidgetItem* item);

  /**
  * @brief Returns the QTreeWidgetItem for the given visualization filter
  * @param filter
  * @return
  */
  QTreeWidgetItem* getTreeWidgetItem(VtkAbstractFilter* filter);

  /**
  * @brief Adds a new filter item for a given visualization filter belonging 
  * to a given QTreeWidgetItem parent
  * @param newFilter
  * @param selectedItem
  */
  void addFilterItem(VtkAbstractFilter* newFilter, QTreeWidgetItem* selectedItem);

  /**
  * @brief Returns the selected QTreeWidgetItem
  */
  QTreeWidgetItem* getSelectedFilterItem();

protected slots:
  /**
  * @brief Changes a visibility filter's visibility state
  * @param item
  * @param column
  */
  void filterToggled(QTreeWidgetItem* item, int column);

  /**
  * @brief Handles a filter selection change
  */
  void filterSelectionChanged();

  /**
  * @brief Creates a custom context menu at the given position
  * @param pos
  */
  void contextMenuActivated(const QPoint& pos);

private:
  VtkRenderController::Pointer m_VtkRenderController;

  QAction* m_ActionAddClipFilter;
  QAction* m_ActionAddSliceFilter;
  QAction* m_ActionAddMaskFilter;
  QAction* m_ActionDeleteFilter;

  QTreeWidgetItem* m_ContextItem;
  QWidget* m_ToggleFilterProperties;
};

#endif
