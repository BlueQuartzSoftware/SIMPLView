#include "VtkLookupTableController.h"

#include <vtkColorTransferFunction.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

#include <limits>

std::vector<VtkLookupTableController::RgbPos_t> VtkLookupTableController::m_defaultRgbPosVector = {VtkLookupTableController::RgbPos_t{0, 0.231373, 0.298039, 0.752941},
                                                                                                   VtkLookupTableController::RgbPos_t{0.5, 0.865003, 0.865003, 0.865003},
                                                                                                   VtkLookupTableController::RgbPos_t{1, 0.705882, 0.0156863, 0.14902}};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkLookupTableController::VtkLookupTableController()
{
  createColorTransferFunction();
  m_range[0] = 0.0;
  m_range[1] = 1.0;

  m_baseRange[0] = 0.0;
  m_baseRange[1] = 1.0;

  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkLookupTableController::~VtkLookupTableController()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::createColorTransferFunction()
{
  m_colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_colorTransferFunction->SetColorSpaceToDiverging();
  m_rgbPosVector.clear();

  m_rgbPosVector = std::vector<RgbPos_t>(m_defaultRgbPosVector.size());
  m_baseRgbPosVector = std::vector<RgbPos_t>(m_defaultRgbPosVector.size());
  for(int i = 0; i < m_defaultRgbPosVector.size(); i++)
  {
    m_rgbPosVector[i] = m_defaultRgbPosVector[i];
    m_baseRgbPosVector[i] = m_defaultRgbPosVector[i];
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkColorTransferFunction> VtkLookupTableController::getColorTransferFunction()
{
  return m_colorTransferFunction;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::parseRgbJson(const QJsonObject& presetObj)
{
  QJsonArray presetArray = presetObj["RGBPoints"].toArray();

  int numColors = presetArray.count() / 4;
  int numComponents = 4;

  double minValue = std::numeric_limits<double>::max();
  double maxValue = std::numeric_limits<double>::min();

  m_rgbPosVector.clear();
  m_baseRgbPosVector.clear();

  m_colorTransferFunction->RemoveAllPoints();
  for(int i = 0; i < numColors; i++)
  {
    RgbPos_t pos;
    pos.x = static_cast<float>(presetArray[i * numComponents + 0].toDouble());

    pos.r = static_cast<float>(presetArray[i * numComponents + 1].toDouble());
    pos.g = static_cast<float>(presetArray[i * numComponents + 2].toDouble());
    pos.b = static_cast<float>(presetArray[i * numComponents + 3].toDouble());

    if(pos.x < minValue)
    {
      minValue = pos.x;
    }
    if(pos.x > maxValue)
    {
      maxValue = pos.x;
    }

    m_rgbPosVector.push_back(pos);
    m_baseRgbPosVector.push_back(pos);

    m_colorTransferFunction->AddRGBPoint(pos.x, pos.r, pos.g, pos.b);
  }

  double minRange = m_range[0];
  double maxRange = m_range[1];

  m_range[0] = minValue;
  m_range[1] = maxValue;

  m_baseRange[0] = minValue;
  m_baseRange[1] = maxValue;

  m_colorTransferFunction->SetColorSpaceToRGB();

  setRange(minRange, maxRange);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::invert()
{
  double minRange = m_range[0];
  double maxRange = m_range[1];

  normalizePositions();

  size_t size = m_rgbPosVector.size();
  std::vector<RgbPos_t> newVector(size);

  for(size_t i = 0; i < size; i++)
  {
    newVector[i] = m_rgbPosVector[size - i - 1];
    newVector[i].x = 1.0 - newVector[i].x;
  }

  m_rgbPosVector = newVector;
  m_baseRgbPosVector = newVector;

  setRange(minRange, maxRange);
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::normalizePositions()
{
  double rangeLength = m_baseRange[1] - m_baseRange[0];

  for(int i = 0; i < m_baseRgbPosVector.size(); i++)
  {
    RgbPos_t pos = m_baseRgbPosVector[i];
    pos.x = (pos.x - m_baseRange[0]) / rangeLength;
    m_rgbPosVector[i] = pos;
    m_baseRgbPosVector[i] = pos;
  }

  m_range[0] = 0.0;
  m_range[1] = 1.0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::setColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
{
  m_colorTransferFunction = colorTransferFunction;
  m_rgbPosVector.clear();

  double val[6];
  for(int i = 0; colorTransferFunction->GetNodeValue(i, val) != -1; i++)
  {
    RgbPos_t pos;
    pos.x = val[0];

    pos.r = val[1];
    pos.g = val[2];
    pos.b = val[3];

    m_rgbPosVector.push_back(pos);
  }

  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double* VtkLookupTableController::getRange()
{
  return m_range;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::setRange(double range[2])
{
  setRange(range[0], range[1]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::setRange(double min, double max)
{
  normalizePositions();

  double rangeLength = max - min;

  // skew points to fit new range
  for(int i = 0; i < m_rgbPosVector.size(); i++)
  {
    RgbPos_t pos = m_baseRgbPosVector[i];
    pos.x = pos.x * rangeLength + min;
    m_rgbPosVector[i] = pos;
  }

  m_range[0] = min;
  m_range[1] = max;

  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::addRgbPoint(double x, double r, double g, double b)
{
  RgbPos_t pos;
  pos.x = x;

  pos.r = r;
  pos.g = g;
  pos.b = b;

  addRgbPoint(pos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::addRgbPoint(double x, double rgb[3])
{
  RgbPos_t pos;
  pos.x = x;

  pos.r = rgb[0];
  pos.g = rgb[1];
  pos.b = rgb[2];

  addRgbPoint(pos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::addRgbPoint(VtkLookupTableController::RgbPos_t rgbPos)
{
  m_rgbPosVector.push_back(rgbPos);
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VtkLookupTableController::getNumberOfRgbPoints()
{
  return (int)m_rgbPosVector.size();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double* VtkLookupTableController::getRgbColor(int index)
{
  double* color = new double[3];

  if(index < m_rgbPosVector.size() && index >= 0)
  {
    RgbPos_t rgb = m_rgbPosVector[index];

    color[0] = rgb.r;
    color[1] = rgb.g;
    color[2] = rgb.b;
  }
  else
  {
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
  }

  return color;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::setColor(int index, double r, double g, double b)
{
  if(index >= m_rgbPosVector.size() || index < 0)
  {
    return;
  }

  RgbPos_t rgbPos = m_rgbPosVector[index];
  rgbPos.r = r;
  rgbPos.g = g;
  rgbPos.b = b;

  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::setColor(int index, double rgb[3])
{
  if(index >= m_rgbPosVector.size() || index < 0)
  {
    return;
  }

  RgbPos_t rgbPos = m_rgbPosVector[index];
  rgbPos.r = rgb[0];
  rgbPos.g = rgb[1];
  rgbPos.b = rgb[2];

  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double VtkLookupTableController::getRgbPosition(int index)
{
  if(index >= m_rgbPosVector.size() || index < 0)
  {
    return 0;
  }

  return m_rgbPosVector[index].x;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::setRgbPositon(int index, double x)
{
  if(index >= m_rgbPosVector.size() || index < 0)
  {
    return;
  }

  m_rgbPosVector[index].x = x;
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::removeRgbPoint(int index)
{
  if(index >= m_rgbPosVector.size() || index < 0)
  {
    return;
  }

  m_rgbPosVector.erase(m_rgbPosVector.begin() + index);
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkLookupTableController::update()
{
  if(nullptr == m_colorTransferFunction)
  {
    return;
  }

  m_colorTransferFunction->RemoveAllPoints();

  for(int i = 0; i < m_rgbPosVector.size(); i++)
  {
    RgbPos_t colorPos = m_rgbPosVector[i];

    m_colorTransferFunction->AddRGBPoint(colorPos.x, colorPos.r, colorPos.g, colorPos.b);
  }

  m_colorTransferFunction->Build();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkLookupTableController::equals(VtkLookupTableController* other)
{
  if(nullptr == other)
  {
    return false;
  }

  if(other->m_rgbPosVector.size() != m_rgbPosVector.size())
  {
    return false;
  }

  if(other->m_range[0] != m_range[0] || other->m_range[1] != m_range[1])
  {
    return false;
  }

  for(int i = 0; i < m_rgbPosVector.size(); i++)
  {
    if(m_rgbPosVector[i].x != other->m_rgbPosVector[i].x)
    {
      return false;
    }
    if(m_rgbPosVector[i].r != other->m_rgbPosVector[i].r)
    {
      return false;
    }
    if(m_rgbPosVector[i].g != other->m_rgbPosVector[i].g)
    {
      return false;
    }
    if(m_rgbPosVector[i].b != other->m_rgbPosVector[i].b)
    {
      return false;
    }
  }

  return true;
}
