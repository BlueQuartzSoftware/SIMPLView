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

#include "SIMPLVtkBridge.h"

#include <vtkRectilinearGrid.h>
#include <vtkImageData.h>
#include <vtkCellData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedLongLongArray.h>
#include <vtkLongLongArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>


//#include <vtkColorTransferFunction.h>
//#include <vtkDataArray.h>
//#include <vtkIntArray.h>
//#include <vtkLine.h>
//#include <vtkLongLongArray.h>
//#include <vtkLookupTable.h>
//#include <vtkMappedUnstructuredGrid.h>
//#include <vtkNamedColors.h>
//#include <vtkPoints.h>
//#include <vtkPolyData.h>
//#include <vtkPolygon.h>
//#include <vtkScalarBarActor.h>
//#include <vtkScalarBarWidget.h>
//#include <vtkTextProperty.h>
//#include <vtkUnsignedCharArray.h>
//#include <vtkUnstructuredGrid.h>
//#include <vtkVertexGlyphFilter.h>


//#include "VtkSIMPL/VtkFilters/VtkDataSetFilter.h"
#include "VtkSIMPL/VtkSupport/VtkTetrahedralGeom.h"
#include "VtkSIMPL/VtkSupport/VtkTetrahedralGeom.h"
#include "VtkSIMPL/VtkSupport/VtkTriangleGeom.h"
#include "VtkSIMPL/VtkSupport/VtkEdgeGeom.h"
#include "VtkSIMPL/VtkSupport/VtkVertexGeom.h"
#include "VtkSIMPL/VtkSupport/VtkQuadGeom.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLVtkBridge::SIMPLVtkBridge()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLVtkBridge::~SIMPLVtkBridge() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataSetRenderViewConfContainer SIMPLVtkBridge::WrapDataContainerArray(DataContainerArray::Pointer dca)
{
  DataSetRenderViewConfContainer container;
  if(!dca)
  {
    return container;
  }
  
  QList<DataContainer::Pointer> dcs = dca->getDataContainers();

  for(QList<DataContainer::Pointer>::Iterator dc = dcs.begin(); dc != dcs.end(); ++dc)
  {
    if(!(*dc))
    {
      continue;
    }
    if(!(*dc)->getGeometry())
    {
      continue;
    }
    vtkSmartPointer<vtkDataSet> dataSet = WrapGeometry((*dc)->getGeometry());
    if(!dataSet)
    {
      continue;
    }
    else
    {      
      std::shared_ptr<SVVtkDataSetRenderViewConf> dataSetRenderViewConf(new SVVtkDataSetRenderViewConf);
      if(dcs.begin() == dc)
      {
        dataSetRenderViewConf->setIsActive(true);
      }
      dataSetRenderViewConf->setDataSet(dataSet);
      dataSetRenderViewConf->setDataContainer(*dc);
      dataSetRenderViewConf->setName((*dc)->getName().toStdString());
      DataArrayRenderViewConfContainer dataArrayRenderViewConfContainer;
      DataContainer::AttributeMatrixMap_t attrMats = (*dc)->getAttributeMatrices();
      for(DataContainer::AttributeMatrixMap_t::Iterator attrMat = attrMats.begin(); attrMat != attrMats.end(); ++attrMat)
      {
        if(!(*attrMat))
        {
          continue;
        }
        // For now, only support Cell, Edge, and Vertex AttributeMatrices
        if((*attrMat)->getType() != AttributeMatrix::Type::Cell
          && (*attrMat)->getType() != AttributeMatrix::Type::Edge
          && (*attrMat)->getType() != AttributeMatrix::Type::Face
          && (*attrMat)->getType() != AttributeMatrix::Type::Vertex)
        {
          continue;
        }
        else
        {
          // If the attribute matrix is of type cell but the elements and tuples do not
          // match up, just continue ; this really should never happen!
          if((*attrMat)->getNumberOfTuples() != dataSet->GetNumberOfCells())
          {
            continue;
          }

          QStringList arrayNames = (*attrMat)->getAttributeArrayNames();
          int arrayIndex = 0;
          for(QStringList::Iterator arrayName = arrayNames.begin(); arrayName != arrayNames.end(); ++arrayName)
          {
            IDataArray::Pointer array = (*attrMat)->getAttributeArray((*arrayName));
            if(!array)
            {
              continue;
            }
            // Do not support bool arrays for the moment...
            // else
            {
              vtkSmartPointer<vtkDataArray> vtkArray = WrapIDataArray(array);
              if(!vtkArray)
              {
                continue;
              }
              else
              {
                vtkArray->SetName(array->getName().toStdString().c_str());
                std::shared_ptr<SVVtkDataArrayRenderViewConf> newView(new SVVtkDataArrayRenderViewConf);
                newView->setName(array->getName().toStdString());
                newView->setUniqueId(arrayIndex++);
                newView->setDataArray(vtkArray.Get());
                
                dataArrayRenderViewConfContainer.push_back(newView);
                dataSet->GetCellData()->AddArray(vtkArray);
              }
            }
            
          }
        }
      }
      
      dataSetRenderViewConf->SetDataArrayRenderViewConfigs(dataArrayRenderViewConfContainer);
      vtkCellData* cellData = dataSet->GetCellData();
      if(cellData->GetNumberOfArrays() > 0)
      {
        cellData->SetActiveScalars(cellData->GetArray(0)->GetName());
      }
      container.push_back(dataSetRenderViewConf);
    }
  }
  return container;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataArray> SIMPLVtkBridge::WrapVertices(SharedVertexList::Pointer vertexArray)
{
  vtkSmartPointer<vtkFloatArray> vtkArray = vtkSmartPointer<vtkFloatArray>::New();
  vtkArray->SetNumberOfComponents(vertexArray->getNumberOfComponents());
  vtkArray->SetNumberOfTuples(vertexArray->getNumberOfTuples());

  vtkArray->SetVoidArray(vertexArray->getVoidPointer(0), vertexArray->getSize(), 1);

  return vtkArray;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(EdgeGeom::Pointer geom)
{
  vtkSmartPointer<VtkEdgeGrid> dataSet = vtkSmartPointer<VtkEdgeGrid>::New();
  VtkEdgeGeom* edgeGeom = dataSet->GetImplementation();
  edgeGeom->SetGeometry(geom);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkDataArray> vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(QuadGeom::Pointer geom)
{
  vtkSmartPointer<VtkQuadGrid> dataSet = vtkSmartPointer<VtkQuadGrid>::New();
  VtkQuadGeom* quadGeom = dataSet->GetImplementation();
  quadGeom->SetGeometry(geom);
  
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkDataArray> vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(RectGridGeom::Pointer geom)
{
  // vtkRectilinearGrid requires 3 arrays with the x, y, and z coordinates of points
  vtkSmartPointer<vtkDataArray> xCoords = WrapIDataArray(geom->getXBounds());
  vtkSmartPointer<vtkDataArray> yCoords = WrapIDataArray(geom->getYBounds());
  vtkSmartPointer<vtkDataArray> zCoords = WrapIDataArray(geom->getZBounds());

  int x = static_cast<int>(xCoords->GetNumberOfTuples());
  int y = static_cast<int>(yCoords->GetNumberOfTuples());
  int z = static_cast<int>(zCoords->GetNumberOfTuples());

  // Create the vtkRectilinearGrid and apply values
  vtkSmartPointer<vtkRectilinearGrid> vtkRectGrid = vtkSmartPointer<vtkRectilinearGrid>::New();
  vtkRectGrid->SetXCoordinates(xCoords);
  vtkRectGrid->SetYCoordinates(yCoords);
  vtkRectGrid->SetZCoordinates(zCoords);
  vtkRectGrid->SetExtent(0, x, 0, y, 0, z);
  vtkRectGrid->SetDimensions(x, y, z);

  return vtkRectGrid;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(TetrahedralGeom::Pointer geom)
{
  vtkSmartPointer<VtkTetrahedralGrid> dataSet = vtkSmartPointer<VtkTetrahedralGrid>::New();
  VtkTetrahedralGeom* tetGeom = dataSet->GetImplementation();
  tetGeom->SetGeometry(geom);
  
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkDataArray> vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(TriangleGeom::Pointer geom)
{
  vtkSmartPointer<VtkTriangleGrid> dataSet = vtkSmartPointer<VtkTriangleGrid>::New();
  VtkTriangleGeom* triGeom = dataSet->GetImplementation();
  triGeom->SetGeometry(geom);
  
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkDataArray> vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(VertexGeom::Pointer geom)
{
  vtkSmartPointer<VtkVertexGrid> dataSet = vtkSmartPointer<VtkVertexGrid>::New();
  VtkVertexGeom* vertGeom = dataSet->GetImplementation();
  vertGeom->SetGeometry(geom);
  
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkDataArray> vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(ImageGeom::Pointer image)
{
  size_t dims[3] = {0, 0, 0};
  float res[3] = {0.0f, 0.0f, 0.0f};
  float origin[3] = {0.0f, 0.0f, 0.0f};

  image->getDimensions(dims);
  image->getResolution(res);
  image->getOrigin(origin);

  vtkSmartPointer<vtkImageData> vtkImage = vtkSmartPointer<vtkImageData>::New();
  vtkImage->SetExtent(0, dims[0], 0, dims[1], 0, dims[2]);
  vtkImage->SetDimensions(dims[0] + 1, dims[1] + 1, dims[2] + 1);
  vtkImage->SetSpacing(res[0], res[1], res[2]);
  vtkImage->SetOrigin(origin[0], origin[1], origin[2]);

  return vtkImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataSet> SIMPLVtkBridge::WrapGeometry(IGeometry::Pointer geom)
{
  if(std::dynamic_pointer_cast<EdgeGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<EdgeGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<ImageGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<ImageGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<QuadGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<QuadGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<RectGridGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<RectGridGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<TetrahedralGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<TetrahedralGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<TriangleGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<TriangleGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<VertexGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<VertexGeom>(geom));
  }

  // Default to nullptr if the type does not match a supported geometry
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkDataArray> SIMPLVtkBridge::WrapIDataArray(IDataArray::Pointer array)
{
  if(std::dynamic_pointer_cast<UInt8ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedCharArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int8ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkCharArray>(array);
  }
  else if(std::dynamic_pointer_cast<UInt16ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedShortArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int16ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkShortArray>(array);
  }
  else if(std::dynamic_pointer_cast<UInt32ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedIntArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int32ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkIntArray>(array);
  }
  else if(std::dynamic_pointer_cast<UInt64ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedLongLongArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int64ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkLongLongArray>(array);
  }
  else if(std::dynamic_pointer_cast<FloatArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkFloatArray>(array);
  }
  else if(std::dynamic_pointer_cast<DoubleArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkDoubleArray>(array);
  }
  else
  {
    return nullptr;
  }
}
