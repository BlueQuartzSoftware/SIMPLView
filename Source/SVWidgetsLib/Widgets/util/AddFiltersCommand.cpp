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

#include "AddFiltersCommand.h"

#include <QtCore/QObject>

#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/CoreFilters/Breakpoint.h"


#include "SVWidgetsLib/Workspace/PipelineFilterWidget.h"
#include "SVWidgetsLib/Workspace/PipelineViewWidget.h"
#include "SVWidgetsLib/Widgets/BreakpointFilterWidget.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFiltersCommand::AddFiltersCommand(AbstractFilter::Pointer filter, PipelineViewWidget* destination, QString actionText, int startIndex, QUndoCommand* parent) :
  QUndoCommand(parent),
  m_ActionText(actionText),
  m_Destination(destination),
  m_StartIndex(startIndex)
{
  if (m_StartIndex < -1)
  {
    m_StartIndex = -1;
  }

  FilterPipeline::Pointer pipeline = FilterPipeline::New();
  pipeline->pushBack(filter);

  m_JsonString = JsonFilterParametersWriter::WritePipelineToString(pipeline, "Pipeline");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFiltersCommand::AddFiltersCommand(QList<AbstractFilter::Pointer> filters, PipelineViewWidget* destination, QString actionText, int startIndex, QUndoCommand* parent) :
  QUndoCommand(parent),
  m_ActionText(actionText),
  m_Destination(destination),
  m_StartIndex(startIndex)
{
  if (m_StartIndex < -1)
  {
    m_StartIndex = -1;
  }

  FilterPipeline::Pointer pipeline = FilterPipeline::New();
  for (int i=0; i<filters.size(); i++)
  {
    pipeline->pushBack(filters[i]);
  }

  m_JsonString = JsonFilterParametersWriter::WritePipelineToString(pipeline, "Pipeline");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFiltersCommand::AddFiltersCommand(PipelineFilterWidget* filterWidget, PipelineViewWidget* destination, QString actionText, int startIndex, QUndoCommand* parent) :
  QUndoCommand(parent),
  m_ActionText(actionText),
  m_Destination(destination),
  m_StartIndex(startIndex)
{
  if (m_StartIndex < -1)
  {
    m_StartIndex = -1;
  }

  FilterPipeline::Pointer pipeline = FilterPipeline::New();
  pipeline->pushBack(filterWidget->getFilter());
  delete filterWidget;

  m_JsonString = JsonFilterParametersWriter::WritePipelineToString(pipeline, "Pipeline");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFiltersCommand::AddFiltersCommand(QList<PipelineFilterWidget*> filterWidgets, PipelineViewWidget* destination, QString actionText, int startIndex, QUndoCommand* parent) :
  QUndoCommand(parent),
  m_ActionText(actionText),
  m_Destination(destination),
  m_StartIndex(startIndex)
{
  if (m_StartIndex < -1)
  {
    m_StartIndex = -1;
  }

  FilterPipeline::Pointer pipeline = FilterPipeline::New();
  for (int i=0; i<filterWidgets.size(); i++)
  {
    pipeline->pushBack(filterWidgets[i]->getFilter());
    delete filterWidgets[i];
  }

  m_JsonString = JsonFilterParametersWriter::WritePipelineToString(pipeline, "Pipeline");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFiltersCommand::AddFiltersCommand(const QString &jsonString, PipelineViewWidget* destination, QString actionText, int startIndex, QUndoCommand* parent) :
  QUndoCommand(parent),
  m_JsonString(jsonString),
  m_ActionText(actionText),
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
AddFiltersCommand::~AddFiltersCommand()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFiltersCommand::undo()
{
  int index = m_StartIndex + m_TotalFiltersPasted - 1;

  for (int i = 0; i < m_TotalFiltersPasted; i++)
  {
    m_Destination->removeFilterWidget(m_Destination->filterWidgetAt(index), false);
    index--;
  }

  m_Destination->preflightPipeline();
  emit m_Destination->pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFiltersCommand::redo()
{
  FilterPipeline::Pointer pipeline = JsonFilterParametersReader::ReadPipelineFromString(m_JsonString);
  FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();

  m_TotalFiltersPasted = container.size();

  if (m_TotalFiltersPasted == 1)
  {
    setText(QObject::tr("\"%1 '%2'\"").arg(m_ActionText).arg(container[0]->getHumanLabel()));
  }
  else
  {
    setText(QObject::tr("\"%1 %2 Filter Widgets\"").arg(m_ActionText).arg(m_TotalFiltersPasted));
  }

  // Paste the filter widgets
  if (m_StartIndex < 0)
  {
    m_StartIndex = m_Destination->filterCount();
  }

  int insertIndex = m_StartIndex;

  for (int i = 0; i < m_TotalFiltersPasted; i++)
  {
    if (std::dynamic_pointer_cast<Breakpoint>(container[i]))
    {
      BreakpointFilterWidget* filterWidget = new BreakpointFilterWidget(container[i], NULL, m_Destination);
      m_Destination->addFilterWidget(filterWidget, insertIndex, false);
    }
    else
    {
      PipelineFilterWidget* filterWidget = new PipelineFilterWidget(container[i], NULL, m_Destination);
      m_Destination->addFilterWidget(filterWidget, insertIndex, false);
    }
    insertIndex++;
  }

  m_Destination->setSelectedFilterWidget(m_Destination->filterWidgetAt(m_StartIndex), Qt::NoModifier);

  m_Destination->preflightPipeline();
}



