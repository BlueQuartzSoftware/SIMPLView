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

#include "PasteCommand.h"

#include <QtCore/QObject>

#include "SIMPLViewWidgetsLib/Widgets/PipelineFilterWidget.h"
#include "SIMPLViewWidgetsLib/Widgets/PipelineViewWidget.h"

#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PasteCommand::PasteCommand(const QString &jsonString, PipelineViewWidget* destination, int startIndex, QUndoCommand* parent) :
  QUndoCommand(parent),
  m_JsonString(jsonString),
  m_Destination(destination),
  m_StartIndex(startIndex)
{
  if (m_StartIndex < -1)
  {
    m_StartIndex = -1;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PasteCommand::~PasteCommand()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PasteCommand::undo()
{
  int index;
  if (m_StartIndex != -1)
  {
    index = m_StartIndex + m_TotalFiltersPasted - 1;
  }
  else
  {
    index = m_Destination->filterCount() - 1;
  }

  for (int i = 0; i < m_TotalFiltersPasted; i++)
  {
    m_Destination->removeFilterWidget(m_Destination->filterWidgetAt(index));
    index--;
  }

  m_Destination->preflightPipeline();
  emit m_Destination->pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PasteCommand::redo()
{
  FilterPipeline::Pointer pipeline = JsonFilterParametersReader::ReadPipelineFromString(m_JsonString);
  FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();

  m_TotalFiltersPasted = container.size();
  setText(QObject::tr("\"Paste %1 Filter Widgets\"").arg(m_TotalFiltersPasted));

//  // Record current selections
//  QList<PipelineFilterWidget*> selected = m_Destination->getSelectedFilterWidgets();
//  for (int i = 0; i < selected.size(); i++)
//  {
//    m_Selections.insert(m_Destination->indexOfFilterWidget(selected[i]), selected[i]->getSelectionModifiers());
//  }

  // Paste the filter widgets
  int insertIndex = m_StartIndex;
  for (int i = 0; i < m_TotalFiltersPasted; i++)
  {
    PipelineFilterWidget* filterWidget = new PipelineFilterWidget(container[i], NULL, m_Destination);
    m_Destination->addFilterWidget(filterWidget, insertIndex);

    if (insertIndex >= 0)
    {
      insertIndex++;
    }
  }

//  // Clear the selected filter widgets
//  m_Destination->clearSelectedFilterWidgets();

//  // Re-select the previously recorded filter widgets
//  if (m_Selections.size() <= 0 && m_Destination->filterCount() > 0)
//  {
//    m_Destination->setSelectedFilterWidget(m_Destination->filterWidgetAt(0), Qt::NoModifier);
//  }
//  else
//  {
//    QMapIterator<int, Qt::KeyboardModifiers> iter(m_Selections);
//    if (iter.hasNext())
//    {
//      iter.next();

//      m_Destination->setSelectedFilterWidget(m_Destination->filterWidgetAt(iter.key()), Qt::NoModifier);
//    }
//    while (iter.hasNext())
//    {
//      iter.next();

//      m_Destination->setSelectedFilterWidget(m_Destination->filterWidgetAt(iter.key()), Qt::ShiftModifier);
//    }
//  }

  m_Destination->preflightPipeline();
  emit m_Destination->pipelineChanged();
}



