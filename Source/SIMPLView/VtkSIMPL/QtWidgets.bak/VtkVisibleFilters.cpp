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

#include "VtkVisibleFilters.h"

#include <QtCore/QVector>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"
#include "VtkSIMPL/VtkFilters/VtkClipFilter.h"
#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
#include "VtkSIMPL/VtkFilters/VtkMaskFilter.h"
#include "VtkSIMPL/VtkFilters/VtkSliceFilter.h"

#include "moc_VtkVisibleFilters.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkVisibleFilters::VtkVisibleFilters(QWidget* parent, VtkRenderController::Pointer renderController)
  : QTreeWidget(parent)
{
  setupGui();

  if(nullptr != renderController)
  {
    setRenderController(renderController);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkVisibleFilters::~VtkVisibleFilters()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::setupGui()
{
  connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), 
    this, SLOT(filterToggled(QTreeWidgetItem*, int)));
  connect(this, SIGNAL(itemSelectionChanged()), 
    this, SLOT(filterSelectionChanged()));

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), 
    this, SLOT(contextMenuActivated(const QPoint&)));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::setRenderController(VtkRenderController::Pointer renderController)
{
  this->clear();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::filterToggled(QTreeWidgetItem* item, int column)
{
  VtkAbstractFilter* filterItem = getFilter(item);

  m_VtkRenderController->setFilterVisibility(filterItem, item->checkState(0) == Qt::Checked);

  emit render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter* VtkVisibleFilters::getFilter(QTreeWidgetItem* item)
{
  if(nullptr == item)
  {
    return nullptr;
  }

  if(nullptr == item->parent())
  {
    QVector<VtkDataSetFilter*> dataSetFilters = m_VtkRenderController->getDataSetFilters();
    int index = this->indexOfTopLevelItem(item);

    if(index >= dataSetFilters.length())
    {
      return nullptr;
    }

    VtkAbstractFilter* parentFilter;
    parentFilter = dataSetFilters.at(index);

    return parentFilter;
  }

  VtkAbstractFilter* parentFilter = getFilter(item->parent());

  if(nullptr == parentFilter)
  {
    return nullptr;
  }

  int index = item->parent()->indexOfChild(item);
  return parentFilter->getChild(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QTreeWidgetItem* VtkVisibleFilters::getTreeWidgetItem(VtkAbstractFilter* filter)
{
  if(nullptr == filter)
  {
    return nullptr;
  }

  if(nullptr == filter->getParentFilter())
  {
    QVector<VtkDataSetFilter*> dataSetFilters = m_VtkRenderController->getDataSetFilters();
    int index = dataSetFilters.indexOf((VtkDataSetFilter*)filter);

    if(index > dataSetFilters.length() || index < 0)
    {
      return nullptr;
    }

    QTreeWidgetItem* parentItem;
    parentItem = this->topLevelItem(index);

    return parentItem;
  }

  QTreeWidgetItem* parentItem = getTreeWidgetItem(filter->getParentFilter());

  if(nullptr == parentItem)
  {
    return nullptr;
  }

  int index = filter->getParentFilter()->getChildren().indexOf(filter);
  return parentItem->child(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QTreeWidgetItem* VtkVisibleFilters::getSelectedFilterItem()
{
  QTreeWidgetItem* item = nullptr;
  if(nullptr == m_ContextItem)
  {
    QList<QTreeWidgetItem*> selectedItems = this->selectedItems();

    if(selectedItems.length() > 0)
    {
      item = selectedItems.at(0);
    }
  }
  else
  {
    item = m_ContextItem;
  }

  return item;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::contextMenuActivated(const QPoint& pos)
{
  QMenu menu(this);
  QTreeWidgetItem* contextItem = this->itemAt(pos);

  VtkAbstractFilter* filterClicked = getFilter(contextItem);

  if(nullptr != contextItem)
  {
    VtkAbstractFilter* filter = getFilter(contextItem);

    if(VtkAbstractFilter::compatibleInput(filterClicked->getOutputType(), VtkClipFilter::getRequiredInputType()))
    {
      menu.addAction(m_ActionAddClipFilter);
    }
    if(VtkAbstractFilter::compatibleInput(filterClicked->getOutputType(), VtkSliceFilter::getRequiredInputType()))
    {
      menu.addAction(m_ActionAddSliceFilter);
    }
    if(VtkAbstractFilter::compatibleInput(filterClicked->getOutputType(), VtkMaskFilter::getRequiredInputType()))
    {
      menu.addAction(m_ActionAddMaskFilter);
    }

    if(filter->canDelete())
    {
      menu.addSeparator();
      menu.addAction(m_ActionDeleteFilter);
    }
  }

  menu.exec(this->mapToGlobal(pos));

  delete contextItem;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::addClipFilter()
{
  QTreeWidgetItem* item = getSelectedFilterItem();
  if(nullptr == item)
  {
    return;
  }

  VtkAbstractFilter* parentFilter = getFilter(item);
  VtkClipFilter* clipFilter = new VtkClipFilter(m_ToggleFilterProperties, parentFilter);

  addFilterItem(clipFilter, item);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::addSliceFilter()
{
  QTreeWidgetItem* item = getSelectedFilterItem();
  if(nullptr == item)
  {
    return;
  }

  VtkAbstractFilter* parentFilter = getFilter(item);
  VtkSliceFilter* sliceFilter = new VtkSliceFilter(m_ToggleFilterProperties, parentFilter);

  addFilterItem(sliceFilter, item);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::addMaskFilter()
{
  QTreeWidgetItem* item = getSelectedFilterItem();
  if(nullptr == item)
  {
    return;
  }

  VtkAbstractFilter* parentFilter = getFilter(item);
  VtkMaskFilter* maskFilter = new VtkMaskFilter(m_ToggleFilterProperties, parentFilter);

  addFilterItem(maskFilter, item);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::addFilterItem(VtkAbstractFilter* newFilter, QTreeWidgetItem* selectedItem)
{
  if(nullptr == selectedItem)
  {
    return;
  }

  QTreeWidgetItem* newItem = new QTreeWidgetItem(selectedItem);
  newItem->setText(0, newFilter->getFilterName());

  newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
  newItem->setCheckState(0, Qt::Checked);

  newItem->parent()->setCheckState(0, Qt::Unchecked);

  this->setCurrentItem(newItem);
  filterSelectionChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkVisibleFilters::filterSelectionChanged()
{
  QTreeWidgetItem* item = getSelectedFilterItem();
  if(nullptr == item)
  {
    return;
  }

  VtkAbstractFilter* filterItem = getFilter(item);
  bool change = false;

  if(nullptr == filterItem)
  {
    return;
  }

  // check if working with new Data Set
  VtkAbstractFilter* lastActiveFilter = m_VtkRenderController->getActiveFilter();

  if(nullptr == lastActiveFilter)
  {
    change = true;
  }
  else if(lastActiveFilter->getAncestor() != filterItem->getAncestor())
  {
    change = true;
  }

  //setActiveFilter(filterItem);

  // do not delete Data Set filters
  //deleteButton->setDisabled(!filterItem->canDelete());

  // if different or new DataSetFilter, change Data View options
  if(change)
  {
    //changeDataSet(this->indexOfTopLevelItem(item));
  }
}
