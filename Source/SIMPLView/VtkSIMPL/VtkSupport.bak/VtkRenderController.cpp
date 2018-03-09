/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
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

#include "VtkRenderController.h"

#include <cmath>

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkAlgorithmOutput.h>
#include <vtkBorderRepresentation.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>

#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <QtAlgorithms>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"
#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
#include "VtkSIMPL/VtkWidgets/VtkAbstractWidget.h"
#include "VtkSIMPL/VTKExtensions/Rendering/vtkPVCenterAxesActor.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkRenderController::VtkRenderController()
{
  m_dataSetFilters.clear();
  m_activeFilter = nullptr;

  m_visibleFilters.clear();
  m_visibleScalarBarFilters.clear();

  m_dataSetId = 0;
  m_dataViewId = 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkRenderController::~VtkRenderController()
{
  m_VtkDataStructs.clear();
}

#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkRenderController::Pointer VtkRenderController::CreateVtkRenderController(VtkDataStructVector_t vtkDataStructs)
{
  VtkRenderController* d = new VtkRenderController;
  d->setVtkDataStructs(vtkDataStructs);
  d->m_dataSetId = 0;
  d->m_dataViewId = 0;
  d->initializeMappersAndActors();

  Pointer ptr(d);
  return ptr;
}
#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::setDataViewActive(int dataViewId)
{
  std::shared_ptr<SVVtkDataSetRenderViewConf> dataSetStruct = getActiveDataSet();
  if(nullptr == dataSetStruct.get())
  {
    return;
  }

  if(static_cast<size_t>(dataViewId) >= dataSetStruct->getDataArrayRenderViewCount() || dataViewId < 0)
  {
    return;
  }

  refreshActors(dataViewId);

  // update qvtkWidget
  m_VtkRenderer->GetRenderWindow()->GetInteractor()->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkDataSetRenderViewConfShPtr VtkRenderController::getActiveDataSet()
{
  if(m_dataSetId >= m_VtkDataStructs.size() || m_dataSetId < 0)
  {
    return nullptr;
  }

  return m_VtkDataStructs[m_dataSetId];
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkDataArrayRenderViewConfShPtr VtkRenderController::getActiveDataView()
{
  std::shared_ptr<SVVtkDataSetRenderViewConf> dataSet = getActiveDataSet();
  if(nullptr == dataSet)
  {
    return nullptr;
  }

  if(m_dataSetId >= dataSet->getDataArrayRenderViewCount() || m_dataViewId < 0)
  {
    return nullptr;
  }

  return dataSet->getDataArrayRenderViewConf(m_dataViewId);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::colorDataSetByPointArray()
{
  std::shared_ptr<SVVtkDataArrayRenderViewConf> view = getActiveDataView();
  std::shared_ptr<SVVtkDataSetRenderViewConf> dataSet = getActiveDataSet();


  view->setColorMethod(SVVtkDataArrayRenderViewConf::ColorMethod::Point);

  refreshMappersAndActors();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::colorDataSetByCellArray()
{
  std::shared_ptr<SVVtkDataArrayRenderViewConf> view = getActiveDataView();
  std::shared_ptr<SVVtkDataSetRenderViewConf> dataSet = getActiveDataSet();

  vtkCellData* cellData = dataSet->getDataSet()->GetCellData();

  if(view == nullptr || dataSet == nullptr)
  {
    return;
  }

  view->setColorMethod(SVVtkDataArrayRenderViewConf::ColorMethod::Cell);
  cellData->SetActiveScalars(cellData->GetArray(m_dataViewId)->GetName());

  refreshMappersAndActors();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::removeColoring()
{
  std::shared_ptr<SVVtkDataArrayRenderViewConf> view = getActiveDataView();
  view->setColorMethod(SVVtkDataArrayRenderViewConf::ColorMethod::Point);

  refreshMappersAndActors();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::refreshMappersAndActors()
{
  std::shared_ptr<SVVtkDataArrayRenderViewConf> view = getActiveDataView();
  if(view == nullptr)
  {
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::refreshActors(int viewId)
{
  std::shared_ptr<SVVtkDataArrayRenderViewConf> view = getActiveDataView();
  if(view.get() != nullptr)
  {
    if(m_activeFilter != nullptr)
    {
      m_VtkRenderer->RemoveActor(m_activeFilter->getActor());
    }
  }

  m_dataViewId = viewId;
  view = getActiveDataView();

  if(view.get() == nullptr)
  {
    return;
  }

  if(m_activeFilter != nullptr && m_visibleFilters.contains(m_activeFilter))
  {
    m_VtkRenderer->AddActor(m_activeFilter->getActor());
  }

  std::shared_ptr<SVVtkDataSetRenderViewConf> dataSetStruct = getActiveDataSet();

  //m_VtkRenderer->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::addFilter(VtkAbstractFilter* filter)
{
  std::shared_ptr<SVVtkDataSetRenderViewConf> dataSetStruct = getActiveDataSet();
  if(nullptr != dataSetStruct)
  {
    filter->setBounds(dataSetStruct->getDataSet()->GetBounds());
  }

  filter->setParentFilter(m_activeFilter);
  setActiveFilter(filter);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::clearFilters()
{
  // purge all filters
  while(m_dataSetFilters.size() > 0)
  {
    if(m_dataSetFilters[0] != nullptr)
    {
      delete m_dataSetFilters[0];
      m_dataSetFilters[0] = nullptr;
    }

    m_dataSetFilters.remove(0);
  }

  m_activeFilter = nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::initializeMappersAndActors()
{
  for(size_t i = 0; i < m_VtkDataStructs.size(); i++)
  {
    SVVtkDataSetRenderViewConf* dataSet = m_VtkDataStructs.at(i).get();

    dataSet->getDataSet()->ComputeBounds();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::setActiveFilter(VtkAbstractFilter* active)
{
  vtkScalarBarWidget* lastScalarWidget = nullptr;

  // disable old widget
  if(nullptr != m_activeFilter)
  {
    if(nullptr != m_activeFilter->getWidget())
    {
      m_activeFilter->getWidget()->disable();
    }

    lastScalarWidget = m_activeFilter->getScalarBarWidget();
  }

  m_activeFilter = active;
  active->setViewScalarId(active->getViewScalarId());
  active->refresh();

  // only check active's scalar bar widget if the filter is visible
  if(m_visibleFilters.contains(active))
  {
    if(!checkScalarsVisible(active))
    {
      addVisibleScalars(active);
    }
  }

  VtkAbstractWidget* widget = active->getWidget();
  if(nullptr != widget)
  {
    widget->enable();
  }

  active->getInteractor()->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::setFilterVisibility(VtkAbstractFilter* filter, bool visible)
{
  if(nullptr == filter)
  {
    return;
  }

  filter->setFilterActive(visible);

  if(visible)
  {
    m_visibleFilters.append(filter);

    if(!checkScalarsVisible(filter) && filter->getLookupTableActive())
    {
      addVisibleScalars(filter);
    }

    m_VtkRenderer->AddActor(filter->getActor());
  }
  else
  {
    m_visibleFilters.removeAll(filter);

    removeVisibleScalars(filter);

    m_VtkRenderer->RemoveActor(filter->getActor());

    m_visibleFilters.removeAll(filter);
    m_visibleScalarBarFilters.removeAll(filter);
  }

  checkScalarBarWidgets();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::setActiveFilterScalars(int scalarId)
{
  if(nullptr == m_activeFilter)
  {
    return;
  }

  //	int previousScalarId = m_activeFilter->getViewScalarId();
  m_activeFilter->setViewScalarId(scalarId);

  if(!m_visibleFilters.contains(m_activeFilter))
  {
    return;
  }

  if(!checkScalarsVisible(m_activeFilter))
  {
    addVisibleScalars(m_activeFilter);
  }

  checkScalarBarWidgets();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::setActiveFilterComponent(int componentId)
{
  if(nullptr == m_activeFilter)
  {
    return;
  }

  m_activeFilter->setViewScalarComponentId(componentId);

  checkScalarBarWidgets();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter* VtkRenderController::getActiveFilter()
{
  return m_activeFilter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::setDataSetFilters(QVector<VtkDataSetFilter*> dataSetFilters)
{
  m_dataSetFilters = dataSetFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<VtkDataSetFilter*> VtkRenderController::getDataSetFilters()
{
  return m_dataSetFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkRenderController::checkFiltersVisible(SVVtkDataSetRenderViewConf* dataSetStruct, int scalarId)
{
  for(int i = 0; i < m_visibleFilters.length(); i++)
  {
    if(m_visibleFilters[i]->getDataSetStruct() == dataSetStruct)
    {
      if(m_visibleFilters[i]->getViewScalarId() == scalarId)
      {
        return true;
      }
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkRenderController::checkFiltersVisible(VtkAbstractFilter* filter)
{
  return checkFiltersVisible(filter->getDataSetStruct(), filter->getViewScalarId());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkRenderController::checkScalarsVisible(VtkAbstractFilter* filter)
{
  for(int i = 0; i < m_visibleScalarBarFilters.length(); i++)
  {
    if(m_visibleScalarBarFilters[i]->sharesScalarBar(filter))
    {
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::addVisibleScalars(VtkAbstractFilter* filter)
{
  if(!checkScalarsVisible(filter))
  {
    vtkScalarBarWidget* widget = filter->getScalarBarWidget().GetPointer();
    widget->On();

    int position = findScalarWidgetPosition(m_visibleScalarBarFilters.length());
    setScalarWidgetPosition(widget, position);

    m_visibleScalarBarFilters.append(filter);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint32_t VtkRenderController::findScalarWidgetPosition(uint32_t defaultPos)
{
  size_t numPositions = 4;
  for(size_t i = 0; i < numPositions; i++)
  {
    vtkSmartPointer<vtkBorderRepresentation> compareRepresentation = getScalarWidgetPosition(i);
    double* comparePos = compareRepresentation->GetPosition();

    bool found = true;

    for(int j = 0; j < m_visibleScalarBarFilters.length() && found; j++)
    {
      vtkBorderRepresentation* borderRep = m_visibleScalarBarFilters[j]->getScalarBarWidget()->GetBorderRepresentation();

      double* borderPos = borderRep->GetPosition();
      double offsetX = std::abs(borderPos[0] - comparePos[0]);
      double offsetY = std::abs(borderPos[1] - comparePos[1]);

      double threshold = 0.001;

      if(offsetX < threshold && offsetY < threshold)
      {
        found = false;
      }
    }

    if(found)
    {
      return static_cast<uint32_t>(i);
    }
  }

  return defaultPos;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkBorderRepresentation> VtkRenderController::getScalarWidgetPosition(unsigned int position)
{
  vtkSmartPointer<vtkBorderRepresentation> borderRep = vtkSmartPointer<vtkBorderRepresentation>::New();

  double height = 0.44;
  double width = 0.2;
  double xOffset = 0.02;
  double yOffset = 0.05;

  int numRows = 1 / height;

  int row = position % numRows;
  int col = position / numRows % 2;

  double posX, posY;
  if(col == 0)
  {
    posX = 1 - xOffset - width;
  }
  else
  {
    posX = xOffset;
  }

  posY = row * (height + yOffset);

  borderRep->SetPosition(posX, posY);
  borderRep->SetPosition2(width, height);

  return borderRep;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::setScalarWidgetPosition(vtkScalarBarWidget* widget, unsigned int position)
{
  vtkBorderRepresentation* borderRep = widget->GetBorderRepresentation();
  vtkSmartPointer<vtkBorderRepresentation> placementRep = getScalarWidgetPosition(position);

  borderRep->SetPosition(placementRep->GetPosition());
  borderRep->SetPosition2(placementRep->GetPosition2());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::removeVisibleScalars(VtkAbstractFilter* filter)
{
  vtkScalarBarWidget* widget = filter->getScalarBarWidget().GetPointer();
  widget->Off();
  m_visibleScalarBarFilters.removeAll(filter);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkRenderController::checkScalarBarWidgets()
{
  for(int i = 0; i < m_visibleFilters.length(); i++)
  {
    if(m_visibleFilters[i]->getLookupTableActive())
    {
      if(!checkScalarsVisible(m_visibleFilters[i]))
      {
        m_visibleScalarBarFilters.append(m_visibleFilters[i]);
        m_visibleFilters[i]->getScalarBarWidget()->On();

        int position = findScalarWidgetPosition(m_visibleScalarBarFilters.length() - 1);
        setScalarWidgetPosition(m_visibleFilters[i]->getScalarBarWidget(), position);
      }
      else if(!m_visibleScalarBarFilters.contains(m_visibleFilters[i]))
      {
        m_visibleFilters[i]->getScalarBarWidget()->Off();
      }
    }
  }

  for(int i = 0; i < m_visibleScalarBarFilters.length(); i++)
  {
    bool overlap = false;
    for(int j = 0; j < i && !overlap; j++)
    {
      if(m_visibleScalarBarFilters[i]->sharesScalarBar(m_visibleScalarBarFilters[j]))
      {
        m_visibleScalarBarFilters[i]->getScalarBarWidget()->Off();
        m_visibleScalarBarFilters.erase(m_visibleScalarBarFilters.begin() + i);

        overlap = true;
        i--;
      }
    }
  }
}

