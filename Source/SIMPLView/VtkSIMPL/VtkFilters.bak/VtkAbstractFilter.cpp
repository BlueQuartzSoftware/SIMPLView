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

#include "VtkAbstractFilter.h"

#include <QString>

#include <vtkActor.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGridAlgorithm.h>

#include <vtkGenericDataObjectWriter.h>

#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
#include "VtkSIMPL/VtkSupport/SIMPLVtkBridge.h"
#include "VtkSIMPL/VtkSupport/VtkLookupTableController.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.h"
#include "VtkSIMPL/VtkWidgets/VtkAbstractWidget.h"
#include "VtkSIMPL/VtkFilters/moc_VtkAbstractFilter.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::VtkAbstractFilter(QWidget* parent, vtkSmartPointer<vtkRenderWindowInteractor> interactor)
: QWidget(parent)
, m_interactor(interactor)
{
  m_parentFilter = nullptr;

  m_dataSet = nullptr;

  m_filterMapper = vtkSmartPointer<vtkDataSetMapper>::New();
  m_filterMapper->ScalarVisibilityOn();
  setMapScalars(true);

  m_lookupTable = new VtkLookupTableController();
  m_filterMapper->SetLookupTable(m_lookupTable->getColorTransferFunction());

  m_filterActor = vtkSmartPointer<vtkActor>::New();
  m_filterActor->SetMapper(m_filterMapper);

  m_viewScalarId = 0;
  m_viewScalarComponentId = 0;

  m_isDirty = false;
  m_changesWaiting = true;

  m_mapScalars = true;
  m_showLookupTable = true;
  m_active = true;

  m_scalarBarWidget = vtkSmartPointer<vtkScalarBarWidget>::New();
  m_scalarBarWidget->SetInteractor(interactor);
  vtkSmartPointer<vtkScalarBarActor> scalarBarActor = m_scalarBarWidget->GetScalarBarActor();

  scalarBarActor->SetLookupTable(m_lookupTable->getColorTransferFunction());

  vtkTextProperty* titleProperty = scalarBarActor->GetTitleTextProperty();
  titleProperty->SetJustificationToCentered();
  titleProperty->SetFontSize(titleProperty->GetFontSize() * 1.5);

  scalarBarActor->UnconstrainedFontSizeOn();

  scalarBarActor->SetTitleRatio(0.75);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter::~VtkAbstractFilter()
{
  while(m_children.count() > 0)
  {
    VtkAbstractFilter* child = m_children[0];
    removeChild(child);
    delete child;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setInteractor(vtkRenderWindowInteractor* iren)
{
  m_interactor = iren;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter* VtkAbstractFilter::getParentFilter()
{
  return m_parentFilter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setParentFilter(VtkAbstractFilter* parent)
{
  if(m_parentFilter != nullptr)
  {
    m_parentFilter->removeChild(this);
  }

  if(nullptr == parent)
  {
    this->m_parentFilter = nullptr;
    this->m_dataSet = nullptr;
    return;
  }
  else
  {
    m_parentFilter = parent;
    m_dataSet = m_parentFilter->getOutput();
    m_parentFilter->m_children.push_back(this);

    m_ParentProducer = vtkSmartPointer<vtkTrivialProducer>::New();
    m_ParentProducer->SetOutput(parent->getOutput());
    m_filterMapper->SetInputConnection(m_ParentProducer->GetOutputPort());

    if(nullptr != getDataSetStruct())
    {
      if(nullptr != getDataSetStruct()->getDataSet())
      {
        if(nullptr != getDataSetStruct()->getDataSet()->GetCellData())
        {
          if(nullptr != getDataSetStruct()->getDataSet()->GetCellData()->GetArray(m_viewScalarId))
          {
            m_lookupTable->setRange(getDataSetStruct()->getDataSet()->GetCellData()->GetArray(m_viewScalarId)->GetRange());
          }
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkDataSet* VtkAbstractFilter::getOutput()
{
  if(m_isDirty)
  {
    calculateOutput();
  }

  return m_dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::refresh()
{
  if(m_isDirty)
  {
    calculateOutput();
  }

  for(int i = 0; i < m_children.length(); i++)
  {
    m_children[i]->refresh();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkAbstractFilter::isDirty()
{
  return m_isDirty;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkAbstractFilter::hasChangesWaiting()
{
  return m_changesWaiting;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::addChild(VtkAbstractFilter* child)
{
  if(m_children.contains(child))
  {
    return;
  }

  m_children.push_back(child);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::removeChild(VtkAbstractFilter* child)
{
  if(!m_children.contains(child))
  {
    return;
  }

  m_children.removeAll(child);
  child->setParent(nullptr);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter* VtkAbstractFilter::getAncestor()
{
  if(nullptr == m_parentFilter)
  {
    return this;
  }

  return m_parentFilter->getAncestor();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<VtkAbstractFilter*> VtkAbstractFilter::getChildren() const
{
  return m_children;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VtkAbstractFilter::getIndexOfChild(VtkAbstractFilter* child) const
{
  return m_children.indexOf(child);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractFilter* VtkAbstractFilter::getChild(int index)
{
  return m_children.at(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkDataSetRenderViewConf* VtkAbstractFilter::getDataSetStruct()
{
  VtkDataSetFilter* dataSetFilter = getDataSetFilter();

  if(nullptr == dataSetFilter)
  {
    return nullptr;
  }

  return dataSetFilter->getDataSetStruct();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkActor> VtkAbstractFilter::getActor()
{
  return m_filterActor;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSetMapper> VtkAbstractFilter::getMapper()
{
  return m_filterMapper;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double* VtkAbstractFilter::getBounds()
{
  if(nullptr == m_parentFilter)
  {
    return nullptr;
  }

  return m_parentFilter->getBounds();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkAbstractWidget* VtkAbstractFilter::getWidget()
{
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkScalarBarWidget> VtkAbstractFilter::getScalarBarWidget()
{
  return getScalarBarWidget(m_viewScalarId);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkScalarBarWidget> VtkAbstractFilter::getScalarBarWidget(int id)
{
  m_scalarBarWidget->SetInteractor(m_interactor);
  return m_scalarBarWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkAbstractFilter::sharesScalarBar(VtkAbstractFilter* other)
{
  if(nullptr == other)
  {
    return false;
  }

  if(other->getDataSetFilter() != getDataSetFilter())
  {
    return false;
  }

  if(other->m_viewScalarId != m_viewScalarId || other->m_viewScalarComponentId != m_viewScalarComponentId)
  {
    return false;
  }

  return m_lookupTable->equals(other->m_lookupTable);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setViewScalarId(int id)
{
  if(nullptr == m_dataSet)
  {
    return;
  }

  if(id < 0)
  {
    id = 0;
  }
  m_viewScalarId = id;
  m_viewScalarComponentId = 0;

  SVVtkDataSetRenderViewConf* dataSetStruct = getDataSetFilter()->getDataSetStruct();

  vtkSmartPointer<vtkDataArray> dataArray = dataSetStruct->getDataSet()->GetCellData()->GetArray(id);
  
  vtkCellData* cellData = m_dataSet->GetCellData();
  if(cellData && dataArray.Get() )
  {
    cellData->SetActiveScalars(dataArray->GetName());
    m_filterMapper->SetScalarRange(dataArray->GetRange()[0], dataArray->GetRange()[1]);
    m_filterMapper->SetScalarModeToDefault();
    
    m_lookupTable->setRange(dataArray->GetRange());
    
    m_filterMapper->Update();
    
    m_scalarBarWidget->GetScalarBarActor()->SetTitle(dataArray->GetName());
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VtkAbstractFilter::getViewScalarId()
{
  return m_viewScalarId;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VtkAbstractFilter::getViewScalarComponentId()
{
  return m_viewScalarComponentId;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setViewScalarComponentId(int id)
{
  if(nullptr == m_dataSet || nullptr == m_filterMapper)
  {
    return;
  }

  if(id < 0)
  {
    id = 0;
  }

  m_viewScalarComponentId = id;

  vtkSmartPointer<vtkDataArray> dataArray = m_dataSet->GetCellData()->GetArray(m_viewScalarId);
  int numComponents = dataArray->GetNumberOfComponents();
  vtkSmartPointer<vtkScalarsToColors> lookupTable = m_filterMapper->GetLookupTable();

  if(numComponents == 1)
  {
    m_scalarBarWidget->GetScalarBarActor()->SetTitle(dataArray->GetName());
  }
  else if(id < numComponents)
  {
    double* range = dataArray->GetRange(id);

    m_filterMapper->SetScalarModeToUseCellFieldData();

    m_filterMapper->ColorByArrayComponent(m_viewScalarId, m_viewScalarComponentId);
    m_filterMapper->SetScalarRange(range);
    m_filterMapper->Update();

    m_lookupTable->setRange(range);

    m_scalarBarWidget->GetScalarBarActor()->SetTitle(dataArray->GetComponentName(id));
  }
  else if(id == numComponents)
  {
    double* range = dataArray->GetRange(-1);

    m_filterMapper->SetScalarModeToUseCellFieldData();

    m_filterMapper->ColorByArrayComponent(m_viewScalarId, -1);
    m_filterMapper->SetScalarRange(range);
    m_filterMapper->Update();

    lookupTable->SetVectorModeToMagnitude();
    m_lookupTable->setRange(range);

    m_scalarBarWidget->GetScalarBarActor()->SetTitle((QString(dataArray->GetName()) + " Magnitude").toStdString().c_str());
  }

  lookupTable->Build();
  getScalarBarWidget()->GetScalarBarActor()->SetLookupTable(lookupTable);

  m_filterMapper->Update();
  m_interactor->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const char* VtkAbstractFilter::scalarIdToName(int scalarId)
{
  if(nullptr == m_dataSet)
  {
    return "";
  }

  vtkDataArray* dataArray = m_dataSet->GetCellData()->GetArray(scalarId);
  if(nullptr == dataArray)
  {
    return "";
  }

  return dataArray->GetName();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataArray> VtkAbstractFilter::getBaseDataArray(int id)
{
  VtkAbstractFilter* ancestor = getAncestor();
  if(nullptr == ancestor)
  {
    return nullptr;
  }

  if(nullptr == ancestor->m_dataSet)
  {
    return nullptr;
  }

  return ancestor->m_dataSet->GetCellData()->GetArray(id);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkDataSetFilter* VtkAbstractFilter::getDataSetFilter()
{
  VtkDataSetFilter* cast = dynamic_cast<VtkDataSetFilter*>(this);

  if(cast != nullptr)
  {
    return cast;
  }

  if(nullptr == m_parentFilter)
  {
    return nullptr;
  }

  return m_parentFilter->getDataSetFilter();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataArray> VtkAbstractFilter::getScalarSet(int id)
{
  return m_dataSet->GetCellData()->GetArray(id);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataArray> VtkAbstractFilter::getScalarSet()
{
  if(m_dataSet->GetCellData()->GetVectors() != nullptr)
  {
    return m_dataSet->GetCellData()->GetVectors();
  }

  return m_dataSet->GetCellData()->GetScalars();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::apply()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::reset()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkAbstractFilter::canDelete()
{
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setDirty()
{
  m_isDirty = true;

  for(int i = 0; i < m_children.size(); i++)
  {
    m_children[i]->setDirty();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::updateMapperScalars()
{
  vtkSmartPointer<vtkDataArray> dataArray = getScalarSet();
  if(nullptr == dataArray)
  {
    return;
  }

  m_dataSet->GetCellData()->SetActiveScalars(dataArray->GetName());
  m_filterMapper->SetInputData(m_dataSet);
  m_filterMapper->SetScalarRange(getScalarRange());
  m_filterMapper->Update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double* VtkAbstractFilter::getScalarRange()
{
  VtkAbstractFilter* ancestorFilter = getAncestor();
  if(nullptr == ancestorFilter)
  {
    return nullptr;
  }

  vtkDataArray* dataArray = ancestorFilter->m_dataSet->GetCellData()->GetArray(m_viewScalarId);

  return dataArray->GetRange();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkScalarsToColors* VtkAbstractFilter::getScalarLookupTable()
{
  if(nullptr == m_lookupTable)
  {
    return nullptr;
  }

  return m_lookupTable->getColorTransferFunction();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkAbstractFilter::compatibleInput(VtkAbstractFilter::dataType_t inputType, VtkAbstractFilter::dataType_t requiredType)
{
  if(requiredType == ANY_DATA_SET)
  {
    return true;
  }

  return requiredType == inputType;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::changesWaiting()
{
  m_changesWaiting = true;

  emit modified();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkAbstractFilter::getScalarsMapped()
{
  return m_mapScalars;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setMapScalars(bool map)
{
  if(map)
  {
    m_filterMapper->SetColorModeToMapScalars();
    m_filterMapper->UseLookupTableScalarRangeOn();
    setViewScalarId(m_viewScalarId);
  }
  else
  {
    m_filterMapper->UseLookupTableScalarRangeOff();
    m_filterMapper->SetColorModeToDirectScalars();
  }

  m_mapScalars = map;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkAbstractFilter::getLookupTableActive()
{
  return m_showLookupTable;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setLookupTableActive(bool show)
{
  m_showLookupTable = show;

  if(nullptr == getDataSetStruct())
  {
    return;
  }

  m_scalarBarWidget->SetEnabled(m_showLookupTable && m_active);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setFilterActive(bool active)
{
  m_active = active;

  if(nullptr == getDataSetStruct())
  {
    return;
  }

  m_scalarBarWidget->SetEnabled(m_showLookupTable && m_active);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::saveFile(QString fileName)
{
  vtkDataSet* output = getOutput();

  vtkSmartPointer<vtkGenericDataObjectWriter> writer = vtkSmartPointer<vtkGenericDataObjectWriter>::New();

  writer->SetFileName(fileName.toStdString().c_str());
  writer->SetInputData(output);
  writer->Write();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::setJsonRgbArray(const QJsonObject& preset)
{
  if(nullptr == m_lookupTable)
  {
    return;
  }

  m_lookupTable->parseRgbJson(preset);
  m_filterMapper->SetLookupTable(m_lookupTable->getColorTransferFunction());
  m_filterMapper->Update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkAbstractFilter::invertLookupTable()
{
  if(nullptr == m_lookupTable)
  {
    return;
  }

  m_lookupTable->invert();
  m_filterMapper->Update();

  if(m_interactor != nullptr)
  {
    m_interactor->GetRenderWindow()->Render();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkColorTransferFunction> VtkAbstractFilter::getColorTransferFunction()
{
  if(nullptr == m_lookupTable)
  {
    return nullptr;
  }

  return m_lookupTable->getColorTransferFunction();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkRenderWindowInteractor> VtkAbstractFilter::getInteractor()
{
  return m_interactor;
}
