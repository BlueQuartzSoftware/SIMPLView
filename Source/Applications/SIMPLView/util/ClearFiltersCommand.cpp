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

#include "ClearFiltersCommand.h"

#include <QtCore/QObject>

#include "Applications/SIMPLView/SIMPLView_UI.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ClearFiltersCommand::ClearFiltersCommand(SIMPLView_UI* instance, QUndoCommand* parent) :
QUndoCommand(parent),
m_Instance(instance),
m_PipelineView(instance->getPipelineViewWidget())
{
  setText(QObject::tr("\"Clear All Filters\""));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ClearFiltersCommand::~ClearFiltersCommand()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ClearFiltersCommand::undo()
{
  int selectionsIndex = 0;
  for (int i = 0; i < m_CopiedFilterWidgets.size(); i++)
  {
    m_PipelineView->addFilterWidget(m_CopiedFilterWidgets[i]);
    if (m_Selections.contains(i))
    {
      m_PipelineView->setSelectedFilterWidget(m_CopiedFilterWidgets[i], m_Selections.value(selectionsIndex, Qt::NoModifier));
      selectionsIndex++;
    }
  }

  m_Instance->setWindowModified(m_Modified);

  m_CopiedFilterWidgets.clear();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ClearFiltersCommand::redo()
{
  int filterCount = m_PipelineView->filterCount() - 1;
  for (int i = 0; i < filterCount; i++)
  {
    m_CopiedFilterWidgets.push_back(m_PipelineView->filterWidgetAt(i)->deepCopy());
  }

  QList<PipelineFilterWidget*> selected = m_PipelineView->getSelectedFilterWidgets();
  for (int i = 0; i < selected.size(); i++)
  {
    m_Selections.insert(m_PipelineView->indexOfFilterWidget(selected[i]), selected[i]->getSelectionModifiers());
  }

  m_Modified = m_Instance->isWindowModified();

  // Clear the filter input widget
  m_Instance->clearFilterInputWidget();

  m_Instance->getPipelineViewWidget()->clearWidgets();
  m_Instance->setWindowModified(true);
}



