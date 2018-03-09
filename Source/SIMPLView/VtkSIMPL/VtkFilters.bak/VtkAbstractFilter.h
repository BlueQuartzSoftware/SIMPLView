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

#ifndef _VtkAbstractFilter_h_
#define _VtkAbstractFilter_h_

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
#endif

#include <vtkSmartPointer.h>
#include <vtkTrivialProducer.h>

#include <QString>
#include <QVector>
#include <QWidget>


class vtkAlgorithm;
class vtkUnstructuredGridAlgorithm;
class vtkAlgorithmOutput;
class vtkDataObject;
class vtkDataSet;
class vtkRenderWindowInteractor;
class vtkActor;
class vtkDataSetMapper;
class vtkDataArray;
class vtkScalarsToColors;
class vtkScalarBarActor;
class vtkScalarBarWidget;
class vtkColorTransferFunction;


class VtkAbstractWidget;
class VtkDataSetFilter;
class VtkLookupTableController;
class SVVtkDataSetRenderViewConf;

using SVVtkDataSetRenderViewConfShPtr = std::shared_ptr<SVVtkDataSetRenderViewConf>;

/**
 * @brief The VtkAbstractFilter class
 */
class VtkAbstractFilter : public QWidget
{
  Q_OBJECT

public:
  enum dataType_t
  {
    IMAGE_DATA,
    UNSTRUCTURED_GRID,
    POLY_DATA,
    ANY_DATA_SET
  };

  VtkAbstractFilter(QWidget* parent, vtkSmartPointer<vtkRenderWindowInteractor> interactor);
  ~VtkAbstractFilter();

  virtual void setFilter() = 0;
  void setInteractor(vtkRenderWindowInteractor* interactor);

  VtkAbstractFilter* getParentFilter();
  void setParentFilter(VtkAbstractFilter* parent);
  VtkAbstractFilter* getAncestor();
  QVector<VtkAbstractFilter*> getChildren() const;
  VtkAbstractFilter* getChild(int index);
  int getIndexOfChild(VtkAbstractFilter* child) const;

  virtual SVVtkDataSetRenderViewConf* getDataSetStruct();

  virtual void setBounds(double* bounds) = 0;
  virtual double* getBounds();

  virtual void setInputData(vtkSmartPointer<vtkDataSet> inputData) = 0;
  virtual void calculateOutput() = 0;
  vtkDataSet* getOutput();

  void refresh();
  bool isDirty();
  bool hasChangesWaiting();

  virtual const QString getFilterName() = 0;

  vtkSmartPointer<vtkActor> getActor();
  vtkSmartPointer<vtkDataSetMapper> getMapper();
  vtkSmartPointer<vtkScalarBarWidget> getScalarBarWidget();
  vtkSmartPointer<vtkScalarBarWidget> getScalarBarWidget(int id);
  bool sharesScalarBar(VtkAbstractFilter* other);

  virtual VtkAbstractWidget* getWidget();

  int getViewScalarId();
  void setViewScalarId(int id);
  int getViewScalarComponentId();
  void setViewScalarComponentId(int id);

  const char* scalarIdToName(int scalarId);

  void saveFile(QString fileName);

  vtkSmartPointer<vtkDataArray> getBaseDataArray(int id);

  virtual bool canDelete();

  virtual void apply();
  virtual void reset();

  bool getScalarsMapped();
  void setMapScalars(bool map);
  bool getLookupTableActive();
  void setLookupTableActive(bool show);
  void setFilterActive(bool active);

  void setJsonRgbArray(const QJsonObject& preset);

  virtual dataType_t getOutputType() = 0;
  static bool compatibleInput(dataType_t inputType, dataType_t requiredType);

  void invertLookupTable();
  vtkSmartPointer<vtkColorTransferFunction> getColorTransferFunction();

  vtkSmartPointer<vtkRenderWindowInteractor> getInteractor();

signals:
  void modified();
  void resized(bool shouldRepaint = false);

protected slots:
  void changesWaiting();

protected:
  VtkAbstractFilter* m_parentFilter;
  QVector<VtkAbstractFilter*> m_children;

  vtkSmartPointer<vtkDataSet> m_dataSet;
  vtkSmartPointer<vtkDataSetMapper> m_filterMapper;
  vtkSmartPointer<vtkActor> m_filterActor;

  vtkSmartPointer<vtkDataArray> getScalarSet(int id);
  vtkSmartPointer<vtkDataArray> getScalarSet();

  VtkLookupTableController* m_lookupTable;

  void updateMapperScalars();

  VtkDataSetFilter* getDataSetFilter();

  double* getScalarRange();
  vtkScalarsToColors* getScalarLookupTable();

  bool m_isDirty;
  bool m_changesWaiting;
  void setDirty();

  bool m_ConnectedInput = false;
  vtkSmartPointer<vtkTrivialProducer> m_ParentProducer;

private:
  void addChild(VtkAbstractFilter* child);
  void removeChild(VtkAbstractFilter* child);

  int m_viewScalarId;
  int m_viewScalarComponentId;

  bool m_mapScalars;
  bool m_showLookupTable;

  bool m_active;

  vtkSmartPointer<vtkScalarBarWidget> m_scalarBarWidget;

  vtkSmartPointer<vtkRenderWindowInteractor> m_interactor;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
