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
#ifndef _svvtkdatasetrenderviewconf_H_
#define _svvtkdatasetrenderviewconf_H_

#include <string>
#include <map>
#include <memory>
#include <vector>

#include <QtCore/QScopedPointer>

#include <vtkSmartPointer.h>


// VTK Forward Declarations
class vtkDataSet;
class vtkDataSetMapper;
class vtkActor;
class vtkTrivialProducer;

// SIMPLVtk Forward Declarations
class SVVtkDataArrayRenderViewConf;
//class VtkAbstractFilter;
class VtkLookupTableController;

//using VtkAbstractFilterShPtr = std::shared_ptr<VtkAbstractFilter>;
using SVVtkDataArrayRenderViewConfShPtr = std::shared_ptr<SVVtkDataArrayRenderViewConf>;
using DataArrayRenderViewConfContainer = std::vector<SVVtkDataArrayRenderViewConfShPtr>;

// SIMPLib forward Declarations
class DataContainer;
using DataContainerShPtr = std::shared_ptr<DataContainer>;

/**
 * @brief The SVVtkDataSetRenderViewConf class
 */
class SVVtkDataSetRenderViewConf
{
  public:
    SVVtkDataSetRenderViewConf();
    virtual ~SVVtkDataSetRenderViewConf();
    
    void setName(const std::string &name);
    std::string getName() const;
    
    void setIsActive(bool active);
    bool isActive() const;
    
    void setUniqueId(const int uuid);
    int getUniqueId() const;
    
    void setDataSet(vtkSmartPointer<vtkDataSet> dataset);
    vtkSmartPointer<vtkDataSet> getDataSet() const;
    
    void setDataContainer(DataContainerShPtr dca);
    DataContainerShPtr getDataContainer();
    
    size_t getDataArrayRenderViewCount() const;
    SVVtkDataArrayRenderViewConfShPtr getDataArrayRenderViewConf(size_t index);
    
    void SetDataArrayRenderViewConfigs(DataArrayRenderViewConfContainer& configs);
    DataArrayRenderViewConfContainer GetDataArrayRenderViewConfigs();
    
    
    vtkSmartPointer<vtkActor> getActor();
    vtkSmartPointer<vtkDataSetMapper> getMapper();
    
    bool getScalarsMapped() const;
    void setMapScalars(bool map);
    
    int getViewScalarId() const;
    void setViewScalarId(int id);
  
  protected:
  
  
  private:
    std::string                       m_Name;
    bool                              m_IsActive = false;
    bool                              m_mapScalars = false;    
    int                               m_viewScalarId = 0;
    int                               m_viewScalarComponentId = 0;
    int                               m_UniqueId = -1;
    vtkSmartPointer<vtkDataSet>       m_DataSet;
    DataContainerShPtr                m_DataContainer;


    vtkSmartPointer<vtkDataSetMapper>   m_filterMapper;
    vtkSmartPointer<vtkActor>           m_filterActor;
    
    DataArrayRenderViewConfContainer           m_DataArrayRenderViewConfContainer;
    std::shared_ptr<VtkLookupTableController>  m_lookupTable;
    vtkSmartPointer<vtkTrivialProducer> m_trivialProducer;
    
    SVVtkDataSetRenderViewConf(const SVVtkDataSetRenderViewConf&) = delete; // Copy Constructor Not Implemented
    void operator=(const SVVtkDataSetRenderViewConf&) = delete; // Operator '=' Not Implemented
    
};

#endif /* _svvtkdatasetrenderviewconf_H_ */
