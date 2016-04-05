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

#include "MoveFilterCommand.h"

#include <QtCore/QObject>

#include "SIMPLViewWidgetsLib/Widgets/PipelineFilterWidget.h"
#include "SIMPLViewWidgetsLib/Widgets/PipelineViewWidget.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MoveFilterCommand::MoveFilterCommand(PipelineFilterWidget* filterWidget, int originIndex, int destIndex, PipelineViewWidget* pipelineView, QUndoCommand* parent) :
  QUndoCommand(parent),
  m_PipelineView(pipelineView),
  m_FilterWidget(filterWidget),
  m_OriginIndex(originIndex),
  m_DestinationIndex(destIndex)
{
  m_FilterWidgetCopy = m_FilterWidget->deepCopy();

  setText(QObject::tr("\"Move '%1'\"").arg(m_FilterWidget->getFilter()->getHumanLabel()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MoveFilterCommand::~MoveFilterCommand()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MoveFilterCommand::undo()
{
  m_FilterWidget = m_PipelineView->filterWidgetAt(m_DestinationIndex);

  m_FilterWidgetCopy = m_FilterWidget->deepCopy();

  m_PipelineView->addFilterWidget(m_FilterWidget, m_OriginIndex);
  m_PipelineView->setSelectedFilterWidget(m_FilterWidget);

  m_PipelineView->preflightPipeline();

  emit m_PipelineView->pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MoveFilterCommand::redo()
{
  m_PipelineView->removeFilterWidget(m_FilterWidget);

  m_PipelineView->addFilterWidget(m_FilterWidgetCopy, m_DestinationIndex);
  m_PipelineView->setSelectedFilterWidget(m_FilterWidgetCopy);

  m_PipelineView->preflightPipeline();

  emit m_PipelineView->pipelineChanged();
}



