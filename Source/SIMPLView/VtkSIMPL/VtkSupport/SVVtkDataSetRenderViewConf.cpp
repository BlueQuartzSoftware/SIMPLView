/* ============================================================================
 * Copyright (c) 2017 BlueQuartz Software, LLC
 * All rights reserved.
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
 * Neither the names of any of the BlueQuartz Software contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "SVVtkDataSetRenderViewConf.h"


#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkTrivialProducer.h>

#include "VtkSIMPL/VtkSupport/VtkLookupTableController.h"


////#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"
//#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkDataSetRenderViewConf::SVVtkDataSetRenderViewConf()
: m_filterMapper(vtkSmartPointer<vtkDataSetMapper>(nullptr))
, m_filterActor(vtkSmartPointer<vtkActor>(nullptr))
, m_lookupTable(std::shared_ptr<VtkLookupTableController>(new VtkLookupTableController))
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkDataSetRenderViewConf::~SVVtkDataSetRenderViewConf()
{
  
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkDataSetRenderViewConf::setName(const std::string &name)
{
  m_Name = name;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::string SVVtkDataSetRenderViewConf::getName() const
{
  return m_Name;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkDataSetRenderViewConf::setIsActive(bool active)
{
  m_IsActive = active;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SVVtkDataSetRenderViewConf::isActive() const
{
  return m_IsActive;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------   
void SVVtkDataSetRenderViewConf::setUniqueId(const int uuid)
{
  m_UniqueId = uuid;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVVtkDataSetRenderViewConf::getUniqueId() const
{
  return m_UniqueId;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkDataSetRenderViewConf::setDataSet(vtkSmartPointer<vtkDataSet> dataset)
{
  m_DataSet = dataset;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SVVtkDataSetRenderViewConf::getDataSet() const
{
  return m_DataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkDataSetRenderViewConf::setDataContainer(DataContainerShPtr dca)
{
  m_DataContainer = dca;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerShPtr SVVtkDataSetRenderViewConf::getDataContainer()
{ 
  return m_DataContainer;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
size_t SVVtkDataSetRenderViewConf::getDataArrayRenderViewCount() const
{
  return m_DataArrayRenderViewConfContainer.size();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkDataArrayRenderViewConfShPtr SVVtkDataSetRenderViewConf::getDataArrayRenderViewConf(size_t index)
{
  return m_DataArrayRenderViewConfContainer[index];
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkDataSetRenderViewConf::SetDataArrayRenderViewConfigs(DataArrayRenderViewConfContainer& configs)
{
  m_DataArrayRenderViewConfContainer = configs;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArrayRenderViewConfContainer SVVtkDataSetRenderViewConf::GetDataArrayRenderViewConfigs()
{
  return  m_DataArrayRenderViewConfContainer;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkActor> SVVtkDataSetRenderViewConf::getActor()
{
  m_DataSet->ComputeBounds();
  
  vtkCellData* cellData = m_DataSet->GetCellData();
  if(cellData)
  {
    vtkDataArray* dataArray = cellData->GetArray(0);
    if(dataArray)
    {
      char* name = dataArray->GetName();
      cellData->SetActiveScalars(name);
    }
    
  }
  
  m_trivialProducer = vtkSmartPointer<vtkTrivialProducer>::New();
  m_trivialProducer->SetOutput(m_DataSet);
  
    if(nullptr == m_filterActor.Get())
  {
    m_filterMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    m_filterMapper->ScalarVisibilityOn();
    m_filterActor = vtkSmartPointer<vtkActor>::New();
    m_filterActor->SetMapper(m_filterMapper);
  }
  
  m_filterMapper->SetInputConnection(m_trivialProducer->GetOutputPort());
  setMapScalars(true);

  vtkSmartPointer<vtkDataArray> colorArray = m_DataSet->GetCellData()->GetScalars();
  if(colorArray != nullptr)
  {
    m_filterMapper->SetScalarRange(colorArray->GetRange()[0], colorArray->GetRange()[1]);
  }
  
  return m_filterActor;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSetMapper> SVVtkDataSetRenderViewConf::getMapper()
{
  return m_filterMapper;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SVVtkDataSetRenderViewConf::getScalarsMapped() const
{
  return m_mapScalars;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkDataSetRenderViewConf::setMapScalars(bool map)
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
void SVVtkDataSetRenderViewConf::setViewScalarId(int id)
{
  if(nullptr == m_DataSet)
  {
    return;
  }

  if(id < 0)
  {
    id = 0;
  }
  m_viewScalarId = id;
  m_viewScalarComponentId = 0;

  SVVtkDataSetRenderViewConf* dataSetStruct = this;

  vtkSmartPointer<vtkDataArray> dataArray = dataSetStruct->getDataSet()->GetCellData()->GetArray(id);
  
  vtkCellData* cellData = m_DataSet->GetCellData();
  if(cellData && dataArray.Get() )
  {
    cellData->SetActiveScalars(dataArray->GetName());
    m_filterMapper->SetScalarRange(dataArray->GetRange()[0], dataArray->GetRange()[1]);
    m_filterMapper->SetScalarModeToDefault();
    
    m_lookupTable->setRange(dataArray->GetRange());
    
    m_filterMapper->Update();
    
    //m_scalarBarWidget->GetScalarBarActor()->SetTitle(dataArray->GetName());
  }

}
