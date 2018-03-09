#include "VtkMappingData.h"

#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkScalarsToColors.h>

#include "SVWidgetsLib/Dialogs/ColorPresetsDialog.h"

//#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "VtkSIMPL/QtWidgets/moc_VtkMappingData.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkMappingData::VtkMappingData(QWidget* parent)
: QWidget(parent)
{
  setupUi(this);

  m_filter = nullptr;
  m_lookupTable = nullptr;

  m_presetsDialog = new ColorPresetsDialog();

  connectSlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkMappingData::~VtkMappingData()
{
  disconnectSlots();

  delete m_presetsDialog;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::selectPreset()
{
  m_presetsDialog->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::load(const QJsonObject& preset, const QPixmap& pixmap)
{
  m_presetsDialog->hide();

  if(nullptr != m_filter)
  {
    m_filter->setJsonRgbArray(preset);
  }

  emit updateMapping();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::invert()
{
  if(nullptr == m_filter)
  {
    return;
  }

  m_filter->invertLookupTable();

  emit updateMapping();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::setDefaults()
{
  if(nullptr == m_lookupTable)
  {
    m_lookupTable = vtkSmartPointer<vtkColorTransferFunction>::New();
  }

  double start, end, middle;

  double blue[3] = {0.231373, 0.298039, 0.752941};
  double white[3] = {0.865003, 0.865003, 0.865003};
  double red[3] = {0.705882, 0.0156863, 0.14902};

  if(m_filter != nullptr)
  {
    vtkSmartPointer<vtkDataArray> array = m_filter->getOutput()->GetCellData()->GetScalars();

    start = array->GetRange()[0];
    end = array->GetRange()[1];
  }
  else
  {
    start = -1;
    end = 1;
  }

  middle = (start + end) / 2.0;

  m_lookupTable->AddRGBPoint(start, blue[0], blue[1], blue[2]);
  m_lookupTable->AddRGBPoint(middle, white[0], white[1], white[2]);
  m_lookupTable->AddRGBPoint(end, red[0], red[1], red[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::mapScalars(bool map)
{
  if(m_filter != nullptr)
  {
    m_filter->setMapScalars(map);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkMappingData::shouldMapScalars()
{
  if(nullptr == m_filter)
  {
    return true;
  }

  return m_filter->getScalarsMapped();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::setFilter(VtkAbstractFilter* filter)
{
  m_filter = filter;

  mapScalarsCheckBox->setChecked(shouldMapScalars());

  if(filter != nullptr)
  {
    m_lookupTable = vtkColorTransferFunction::SafeDownCast(filter->getMapper()->GetLookupTable());

    if(nullptr == m_lookupTable)
    {
      setDefaults();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::connectSlots()
{
  connect(m_presetsDialog, SIGNAL(applyPreset(const QJsonObject&, const QPixmap&)), this, SLOT(load(const QJsonObject&, const QPixmap&)));

  connect(loadPresetPushButton, SIGNAL(clicked()), this, SLOT(selectPreset()));
  connect(invertTablePushButton, SIGNAL(clicked()), this, SLOT(invert()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkMappingData::disconnectSlots()
{
  disconnect(m_presetsDialog, SIGNAL(applyPreset(const QJsonObject&, const QPixmap&)), this, SLOT(load(const QJsonObject&, const QPixmap&)));

  disconnect(loadPresetPushButton, SIGNAL(clicked()), this, SLOT(selectPreset()));
  disconnect(invertTablePushButton, SIGNAL(clicked()), this, SLOT(invert()));
}
