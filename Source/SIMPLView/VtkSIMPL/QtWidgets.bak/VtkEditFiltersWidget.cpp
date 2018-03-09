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

#include "VtkEditFiltersWidget.h"

#include <QtWidgets/QMenu>

//#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkClipFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkCropFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkMaskFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkSliceFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkThresholdFilter.h"
#include "VtkSIMPL/QtWidgets/VtkMappingData.h"
//#include "VtkSIMPL/VtkSupport/VtkRenderController.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h"


#include "moc_VtkEditFiltersWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkEditFiltersWidget::VtkEditFiltersWidget(QWidget* parent)
  : QWidget(parent)
{
  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkEditFiltersWidget::~VtkEditFiltersWidget()
{
  if(m_addClipFilterAction)
  {
    delete m_addClipFilterAction;
  }
  if(m_addMaskFilterAction)
  {
    delete m_addMaskFilterAction;
  }
  if(m_addSliceFilterAction)
  {
    delete m_addSliceFilterAction;
  }
  if(m_deleteFilterAction)
  {
    delete m_deleteFilterAction;
  }

  if(m_MappingData)
  {
    delete m_MappingData;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::setupGui()
{
  createActions();

  toggleFilterProperties->setTitle("Toggle Filter Properties");
  toggleRenderProperties->setTitle("Toggle Render Properties");

  m_MappingData = new VtkMappingData(toggleRenderProperties);
  toggleRenderProperties->setWidget(m_MappingData);
  m_MappingData->show();

  connectSlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::connectSlots()
{
  connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), 
    this, SLOT(filterToggled(QTreeWidgetItem*, int)));
  connect(treeWidget, SIGNAL(itemSelectionChanged()), 
    this, SLOT(filterSelected()));

  connect(applyButton, SIGNAL(clicked()), this, SLOT(applyFilter()));
  connect(resetButton, SIGNAL(clicked()), this, SLOT(resetFilter()));
  connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteFilter()));

  if(m_MappingData != nullptr)
  {
    connect(m_MappingData->mapScalarsCheckBox, SIGNAL(stateChanged(int)), 
      this, SIGNAL(mapFilterScalars(int)));
    connect(m_MappingData->showLookupTableCheckBox, SIGNAL(stateChanged(int)), 
      this, SIGNAL(showLookupTable(int)));
    connect(m_MappingData, SIGNAL(updateMapping()), 
      this, SIGNAL(checkScalarMapping()));
  }

  treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), 
    this, SLOT(contextMenuActivated(const QPoint&)));

  connect(toggleFilterProperties, SIGNAL(resized()), this, SLOT(resizeScrollArea()));
  connect(toggleRenderProperties, SIGNAL(resized()), this, SLOT(resizeScrollArea()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::disconnectSlots()
{
  disconnect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), 
    this, SLOT(filterToggled(QTreeWidgetItem*, int)));
  disconnect(treeWidget, SIGNAL(itemSelectionChanged()), 
    this, SLOT(filterSelected()));

  disconnect(applyButton, SIGNAL(clicked()), this, SLOT(applyFilter()));
  disconnect(resetButton, SIGNAL(clicked()), this, SLOT(resetFilter()));
  disconnect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteFilter()));

  if(m_MappingData != nullptr)
  {
    disconnect(m_MappingData->mapScalarsCheckBox, SIGNAL(stateChanged(int)), 
      this, SIGNAL(mapFilterScalars(int)));
    disconnect(m_MappingData->showLookupTableCheckBox, SIGNAL(stateChanged(int)), 
      this, SIGNAL(showLookupTable(int)));
    disconnect(m_MappingData, SIGNAL(updateMapping()), 
      this, SIGNAL(checkScalarMapping()));
  }

  disconnect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), 
    this, SLOT(contextMenuActivated(const QPoint&)));

  disconnect(toggleFilterProperties, SIGNAL(resized()), 
    this, SLOT(resizeScrollArea()));
  disconnect(toggleRenderProperties, SIGNAL(resized()), 
    this, SLOT(resizeScrollArea()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::setRenderController(VtkRenderControllerShPtr renderController)
{
  disconnectSlots();

  m_VtkRenderController = renderController;
  setupFilterTreeView();
  createActions();
  connectSlots();

  scrollAreaWidgetContents->adjustSize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::setRenderWindowInteractor(vtkRenderWindowInteractor* iren)
{
  m_renderInteractor = iren;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::cleanupVisualization()
{
  disconnectSlots();

  toggleFilterProperties->setWidget(nullptr);
  toggleRenderProperties->setWidget(nullptr);

  treeWidget->clear();

  m_ContextItem = nullptr;

  if(nullptr != m_VtkRenderController)
  {
    m_VtkRenderController->clearFilters();
  }

  m_VtkRenderController = VtkRenderController::NullPointer();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::applyFilter()
{
  if(nullptr == m_VtkRenderController)
  {
    return;
  }

  m_VtkRenderController->getActiveFilter()->apply();
  emit renderVtk();

  applyButton->setEnabled(false);
  applyButton->setDefault(false);
  resetButton->setEnabled(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::resetFilter()
{
  if(nullptr == m_VtkRenderController)
  {
    return;
  }

  m_VtkRenderController->getActiveFilter()->reset();

  applyButton->setEnabled(false);
  applyButton->setDefault(false);
  resetButton->setEnabled(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::deleteFilter()
{
  if(nullptr == m_VtkRenderController)
  {
    return;
  }

  VtkAbstractFilter* currentFilter = m_VtkRenderController->getActiveFilter();

  if(m_ContextItem != nullptr)
  {
    currentFilter = getFilter(m_ContextItem);
  }

  // do not delete Data Set Filters
  if(dynamic_cast<VtkDataSetFilter*>(currentFilter) != nullptr)
  {
    return;
  }

  QTreeWidgetItem* filterItem = getTreeWidgetItem(currentFilter);
  uncheckFilterAndChildren(filterItem, true);

  treeWidget->setCurrentItem(filterItem->parent());
  filterSelected();
  filterItem->parent()->removeChild(filterItem);

  currentFilter->setParentFilter(nullptr);
  delete currentFilter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::filterModified()
{
  applyButton->setEnabled(true);
  applyButton->setDefault(true);
  resetButton->setEnabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::setupFilterTreeView()
{
  treeWidget->clear();
  treeWidget->setHeaderLabel("Data Pipeline");
  treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  m_ContextItem = nullptr;

  if(!m_VtkRenderController)
  {
    return;
  }

  QVector<VtkDataSetFilter*> dataSetFilterVector;
  for(int i = 0; i < m_VtkRenderController->getVtkDataStructs().size(); i++)
  {
    SVVtkDataSetRenderViewConfShPtr dataSetStruct = m_VtkRenderController->getVtkDataStructs()[i];

    VtkDataSetFilter* dataSetFilter = new VtkDataSetFilter(toggleFilterProperties, dataSetStruct.get(), m_renderInteractor);
    dataSetFilterVector.push_back(dataSetFilter);

    QTreeWidgetItem* treeDataSetItem = new QTreeWidgetItem(treeWidget);
    treeDataSetItem->setText(0, QString::fromStdString(dataSetStruct->getName()));

    treeDataSetItem->setFlags(treeDataSetItem->flags() | Qt::ItemIsUserCheckable);
    treeDataSetItem->setCheckState(0, Qt::Unchecked);
  }

  m_VtkRenderController->setDataSetFilters(dataSetFilterVector);

  // apply first QTreeWidgetItem
  if(treeWidget->topLevelItemCount() > 0)
  {
    QTreeWidgetItem* firstItem = treeWidget->topLevelItem(0);
    firstItem->setCheckState(0, Qt::Checked);
    treeWidget->setCurrentItem(firstItem);
    filterSelected();
    filterToggled(firstItem, 0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::setActiveFilter(VtkAbstractFilter* filter)
{
  if(nullptr == m_VtkRenderController || nullptr == filter)
  {
    return;
  }

  QWidget* lastFilter = m_VtkRenderController->getActiveFilter();
  m_VtkRenderController->setActiveFilter(filter);

  if(lastFilter != nullptr)
  {
    disconnect(lastFilter, SIGNAL(modified()), this, SLOT(filterModified()));
  }

  toggleFilterProperties->setWidget(filter);
  toggleRenderProperties->setWidget(m_MappingData);

  scrollAreaWidgetContents->update();
  scrollAreaWidgetContents->resize(scrollAreaWidgetContents->sizeHint());
  scrollArea->update();

  if(filter != nullptr)
  {
    connect(filter, SIGNAL(modified()), this, SLOT(filterModified()));
  }

  applyButton->setEnabled(filter->hasChangesWaiting());
  applyButton->setDefault(filter->hasChangesWaiting());
  resetButton->setEnabled(filter->hasChangesWaiting());

  if(m_MappingData != nullptr)
  {
    m_MappingData->setFilter(filter);
    m_MappingData->mapScalarsCheckBox->setCheckState(filter->getScalarsMapped() ? Qt::Checked : Qt::Unchecked);
    m_MappingData->showLookupTableCheckBox->setCheckState(filter->getLookupTableActive() ? Qt::Checked : Qt::Unchecked);
  }

  emit activeFilterChanged(filter);
  emit renderVtk();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::filterToggled(QTreeWidgetItem* item, int column)
{
  m_ContextItem = nullptr;
  VtkAbstractFilter* filterItem = getFilter(item);

  m_VtkRenderController->setFilterVisibility(filterItem, item->checkState(0) == Qt::Checked);

  emit renderVtk();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::filterSelected()
{
  m_ContextItem = nullptr;
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

  setActiveFilter(filterItem);

  // do not delete Data Set filters
  deleteButton->setDisabled(!filterItem->canDelete());

  // if different or new DataSetFilter, change Data View options
  if(change)
  {
    changeDataSet(treeWidget->indexOfTopLevelItem(item));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QTreeWidgetItem* VtkEditFiltersWidget::getSelectedFilterItem()
{
  QTreeWidgetItem* item = nullptr;
  if(nullptr == m_ContextItem)
  {
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if(selectedItems.size() > 0)
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
VtkAbstractFilter* VtkEditFiltersWidget::getFilter(QTreeWidgetItem* item)
{
  if(nullptr == item)
  {
    return nullptr;
  }

  if(nullptr == item->parent())
  {
    QVector<VtkDataSetFilter*> dataSetFilters = m_VtkRenderController->getDataSetFilters();
    int index = treeWidget->indexOfTopLevelItem(item);

    if(index >= dataSetFilters.size())
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
QTreeWidgetItem* VtkEditFiltersWidget::getTreeWidgetItem(VtkAbstractFilter* filter)
{
  if(nullptr == filter)
  {
    return nullptr;
  }

  if(nullptr == filter->getParentFilter())
  {
    QVector<VtkDataSetFilter*> dataSetFilters = m_VtkRenderController->getDataSetFilters();
    int index = dataSetFilters.indexOf((VtkDataSetFilter*)filter);

    if(index > dataSetFilters.size() || index < 0)
    {
      return nullptr;
    }

    QTreeWidgetItem* parentItem;
    parentItem = treeWidget->topLevelItem(index);

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
void VtkEditFiltersWidget::uncheckFilterAndChildren(QTreeWidgetItem* filterItem, bool checkParent)
{
  if(filterItem->checkState(0) == Qt::Checked)
  {
    filterItem->setCheckState(0, Qt::Unchecked);
    filterToggled(filterItem, 0);

    if(checkParent)
    {
      filterItem->parent()->setCheckState(0, Qt::Checked);
      filterToggled(filterItem->parent(), 0);
    }
  }

  for(int i = 0; i < filterItem->childCount(); i++)
  {
    uncheckFilterAndChildren(filterItem->child(i), false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::changeDataSet(int setId)
{
  SVVtkDataSetRenderViewConfShPtr dataSet = m_VtkRenderController->getActiveDataSet();
  if(nullptr == dataSet)
  {
    return;
  }

  if(dataSet->getDataArrayRenderViewCount() > 0)
  {
    m_VtkRenderController->setDataViewActive(0);
  }

  emit dataSetChanged(setId);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::resizeScrollArea()
{
  scrollAreaWidgetContents->adjustSize();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::createActions()
{
  // do not reconnect filters that already exist
  if(nullptr == m_addClipFilterAction)
  {
    m_addClipFilterAction = new QAction("Add Clip Filter", this);
    m_addClipFilterAction->setStatusTip("Adds a clip filter as a child to the target filter");
    connect(m_addClipFilterAction, SIGNAL(triggered(bool)), this, SLOT(addClipFilter()));
  }

  if(nullptr == m_addSliceFilterAction)
  {
    m_addSliceFilterAction = new QAction("Add Slice Filter", this);
    m_addSliceFilterAction->setStatusTip("Adds a slice filter as a child to the target filter");
    connect(m_addSliceFilterAction, SIGNAL(triggered(bool)), this, SLOT(addSliceFilter()));
  }

  if(nullptr == m_addMaskFilterAction)
  {
    m_addMaskFilterAction = new QAction("Add Mask Filter", this);
    m_addMaskFilterAction->setStatusTip("Adds a mask filter as a child to the target filter");
    connect(m_addMaskFilterAction, SIGNAL(triggered(bool)), this, SLOT(addMaskFilter()));
  }

  if(nullptr == m_deleteFilterAction)
  {
    m_deleteFilterAction = new QAction("Delete Filter", this);
    m_deleteFilterAction->setStatusTip("Delete the target clip and all of its children");
    connect(m_deleteFilterAction, SIGNAL(triggered(bool)), this, SLOT(deleteFilter()));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::contextMenuActivated(const QPoint& pos)
{
  QMenu menu(treeWidget);
  m_ContextItem = treeWidget->itemAt(pos);

  VtkAbstractFilter* filterClicked = getFilter(m_ContextItem);

  if(m_ContextItem != nullptr)
  {
    VtkAbstractFilter* filter = getFilter(m_ContextItem);

    if(VtkAbstractFilter::compatibleInput(filterClicked->getOutputType(), VtkClipFilter::getRequiredInputType()))
    {
      menu.addAction(m_addClipFilterAction);
    }
    if(VtkAbstractFilter::compatibleInput(filterClicked->getOutputType(), VtkSliceFilter::getRequiredInputType()))
    {
      menu.addAction(m_addSliceFilterAction);
    }
    if(VtkAbstractFilter::compatibleInput(filterClicked->getOutputType(), VtkMaskFilter::getRequiredInputType()))
    {
      menu.addAction(m_addMaskFilterAction);
    }

    if(filter->canDelete())
    {
      menu.addSeparator();
      menu.addAction(m_deleteFilterAction);
    }
  }

  menu.exec(treeWidget->mapToGlobal(pos));

  m_ContextItem = nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::addFilterItem(VtkAbstractFilter* newFilter, QTreeWidgetItem* selectedItem)
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

  treeWidget->setCurrentItem(newItem);
  filterSelected();
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::addClipFilter()
{
  QTreeWidgetItem* item = getSelectedFilterItem();
  if(nullptr == item)
  {
    return;
  }

  VtkAbstractFilter* parentFilter = getFilter(item);
  VtkClipFilter* clipFilter = new VtkClipFilter(toggleFilterProperties, parentFilter);

  addFilterItem(clipFilter, item);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::addSliceFilter()
{
  QTreeWidgetItem* item = getSelectedFilterItem();
  if(nullptr == item)
  {
    return;
  }

  VtkAbstractFilter* parentFilter = getFilter(item);
  VtkSliceFilter* sliceFilter = new VtkSliceFilter(toggleFilterProperties, parentFilter);

  addFilterItem(sliceFilter, item);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkEditFiltersWidget::addMaskFilter()
{
  QTreeWidgetItem* item = getSelectedFilterItem();
  if(nullptr == item)
  {
    return;
  }

  VtkAbstractFilter* parentFilter = getFilter(item);
  VtkMaskFilter* maskFilter = new VtkMaskFilter(toggleFilterProperties, parentFilter);

  addFilterItem(maskFilter, item);
}

