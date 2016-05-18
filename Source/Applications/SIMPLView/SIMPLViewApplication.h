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

#ifndef _simplviewapplication_h_
#define _simplviewapplication_h_

#include <QtCore/QSet>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QUndoStack>

#include "SVWidgetsLib/Dialogs/UpdateCheck.h"

#define dream3dApp (static_cast<SIMPLViewApplication *>(qApp))

class DSplashScreen;
class SIMPLView_UI;
class QPluginLoader;
class ISIMPLibPlugin;
class SIMPLViewToolbox;
class SVPipelineFilterWidget;
class SVPipelineViewWidget;

class SIMPLViewApplication : public QApplication
{
    Q_OBJECT

  public:
    SIMPLViewApplication(int& argc, char** argv);
    ~SIMPLViewApplication();

    enum PasteType
    {
      None,
      Cut,
      Copy
    };

     /**
     * @brief fillVersionData
     * @return
     */
    static UpdateCheck::SIMPLVersionData_t FillVersionData();

    bool initialize(int argc, char* argv[]);

    QList<SIMPLView_UI*> getSIMPLViewInstances();

    void registerSIMPLViewWindow(SIMPLView_UI* window);

    virtual void unregisterSIMPLViewWindow(SIMPLView_UI* window);

    SIMPLView_UI* getNewSIMPLViewInstance();

    SIMPLView_UI* getActiveWindow();
    void setActiveWindow(SIMPLView_UI* instance);

    bool isCurrentlyRunning(SIMPLView_UI* instance);

    /**
     * @brief event
     * @param event
     * @return
     */
    bool event(QEvent* event);

    QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*> getClipboard();

  public slots:
    void newInstanceFromFile(const QString& filePath, const bool& setOpenedFilePath, const bool& addToRecentFiles);

    void setClipboard(QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*> clipboard);

  protected:
    SIMPLViewToolbox*                           m_Toolbox;

    // This is a set of all SIMPLView instances currently available
    QList<SIMPLView_UI*>                        m_SIMPLViewInstances;

    // This is the set of SIMPLView instances that are currently running a pipeline
    QSet<SIMPLView_UI*>                         m_CurrentlyRunningInstances;

    // The currently active SIMPLView instance
    SIMPLView_UI*                               m_ActiveWindow;
    SIMPLView_UI*                               m_PreviousActiveWindow;

    QString                                   m_OpenDialogLastDirectory;

    bool                                      show_splash;
    DSplashScreen*                            Splash;

    QVector<QPluginLoader*>                   m_PluginLoaders;

    QVector<ISIMPLibPlugin*> loadPlugins();

  protected slots:
    void on_actionCloseToolbox_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionShowToolbox_triggered(bool visible);
    void on_actionShowIssues_triggered(bool visible);
    void on_actionShowStdOutput_triggered(bool visible);

    void on_actionAddBookmark_triggered();
    void on_actionNewFolder_triggered();
    void on_actionRenameBookmark_triggered();
    void on_actionRemoveBookmark_triggered();
    void on_actionShowBookmarkInFileSystem_triggered();
    void on_actionClearCache_triggered();
    void on_actionClearBookmarks_triggered();

    void on_actionCloseWindow_triggered();
    void on_actionExit_triggered();
    void on_actionShowSIMPLViewHelp_triggered();
    void on_actionCheckForUpdates_triggered();
    void on_actionPluginInformation_triggered();
    void on_actionAboutSIMPLView_triggered();

    void on_pipelineViewWidget_deleteKeyPressed(SVPipelineViewWidget* widget);
    void on_bookmarksDockContextMenuRequested(const QPoint&);
    void bookmarkSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

    void toPipelineRunningState();
    void toPipelineIdleState();

    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();

    void updatePasteState(bool canPaste);

    /**
    * @brief Updates the QMenu 'Recent Files' with the latest list of files. This
    * should be connected to the Signal QtSRecentFileList->fileListChanged
    * @param file The newly added file.
    */
    virtual void updateRecentFileList(const QString& file);

    virtual void dream3dWindowChanged(SIMPLView_UI* instance);
    virtual void toolboxWindowChanged();

    virtual void on_actionClearRecentFiles_triggered();

    // SIMPLView_UI slots
    void openRecentFile();

    void addFilter(const QString &className);

  private:
    QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*>                  m_Clipboard;

    QMenu*                                                                    m_ContextMenu;

    bool                                                                      m_ShowFilterWidgetDeleteDialog;

    void readSettings();
    void writeSettings();

    SIMPLViewApplication(const SIMPLViewApplication&); // Copy Constructor Not Implemented
    void operator=(const SIMPLViewApplication&); // Operator '=' Not Implemented
};

#endif /* _SIMPLViewApplication_H */

