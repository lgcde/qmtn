/* Qt Movie Thumbnailer - movie screencast generator

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QDropEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QToolBar>
#include <QPluginLoader>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
#include <QVersionNumber>
#include <QDebug>
#endif
#ifdef QT_WEBENGINEWIDGETS_LIB
#include <QtWebEngineWidgets>
#endif
#include "mainwindow.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"
#include "iconprovider.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    processingItems(0),    
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))

    AppVersion = QStringLiteral(VERSION_RELEASE);
    AppVersionGit = QStringLiteral(VERSION_FROM_GIT_TAG);
    setWindowTitle(QString("%1 (%2)").arg(
        qApp->applicationName(),
        QVersionNumber::fromString(AppVersion).toString()));
#else
    AppVersion = QStringLiteral(VERSION_RELEASE);
    setWindowTitle(QString("%1 (%2)").arg(qApp->applicationName()).arg(AppVersion));
#endif
    datamodel = new QStandardItemModel(this);
    datamodel->setColumnCount(4);

    profileModel = new ProfileModel(this);
    worker = new MtnWorker(profileModel);

    ui->treeView->hide();
    ui->treeView->setModel(datamodel);
    ui->treeView->header()->hideSection(columnItemNames::path);
    ui->treeView->header()->hideSection(columnItemNames::logtext);
    ui->treeView->header()->hideSection(columnItemNames::output);

    ui->imageViewer->setModel(ui->treeView->selectionModel());

    ui->mainToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->action_Settings->setIcon(IconProvider::settings());
    ui->action_Quit->setIcon(IconProvider::exit());
    ui->actionAbout->setIcon(IconProvider::help());
    ui->actionAboutQt->setIcon(IconProvider::qt());
    ui->TabOutput->setTabIcon(0, IconProvider::image());
    ui->TabOutput->setTabIcon(1, IconProvider::text());
    ui->actionOpenDirectory->setIcon(IconProvider::folder());
    ui->actionOpenFile->setIcon(IconProvider::video());
    ui->actionRefreshThumbnail->setIcon(IconProvider::refresh());
    ui->actionRemoveItemfromSidebar->setIcon(IconProvider::remove());

    connect(ui->mainToolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::toolbarContextMenuRequested);
    connect(ui->action_Quit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::currentRowChanged);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,  this, &MainWindow::selectionChanged);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::treeContextMenuRequest);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &MainWindow::treeItemDoubleClicked);
    connect(worker, &MtnWorker::changedProcessingItemsNumber, this, &MainWindow::changedProcessingItemsNumber);
    connect(worker, &MtnWorker::generatingSuccess, this, &MainWindow::updateItem);

    QStringList defaultVideoExtensions;
    defaultVideoExtensions
        << "3gp"  << "3g2"    << "asf"   << "avi" << "avs"  << "dat"  << "divx"
        << "dsm"  << "evo"    << "flv"   << "m1v" << "m2ts" << "m2v"  << "m4a"
        << "mj2"  << "mjpg"   << "mjpeg" << "mkv" << "mov"  << "moov" << "mp4"
        << "rmvb" << "mpeg"   << "mpv"   << "nut" << "ogg"  << "ogm"  << "xvid"
        << "mpg"  << "swf"    << "ts"    << "vob" << "webm" << "wmv"  << "qt"
        << "rm"   << "f4v";

    QSettings s;
    settingsFileName = s.fileName();
    restoreGeometry(s.value("mainform/geometry").toByteArray());
    restoreState(s.value("mainform/state").toByteArray());
    ui->splitter->restoreState(s.value("mainform/splitter").toByteArray());
    recentFiles = s.value("recentFiles").toStringList();
    maxRecentFiles = s.value("MaxRecentFiles", 5).toInt();
    pluginsLocation = s.value("plugins/location").toString();
    ui->mainToolBar->setToolButtonStyle(
                static_cast<Qt::ToolButtonStyle>(s.value("mainform/toolbarlabels", Qt::ToolButtonTextBesideIcon).toInt()));

    videoExtensions = s.value("VideoExtensions", defaultVideoExtensions).toStringList();

    createStatusBarWidgets();
    createRecentFiles();
    createRecentMenu();
    refreshStatusBar();
    updateActionState();
    loadPlugins();
}
/******************************************************************************************************/
MainWindow::~MainWindow()
{
    delete ui;
    delete worker;
}
/******************************************************************************************************/
void MainWindow::updateItem(QStandardItem *parent, int row)
{
    if(ui->treeView->currentIndex().parent() == parent->index() &&
       ui->treeView->currentIndex().row()    == row)
    {
        // update log
        currentRowChanged(ui->treeView->currentIndex(), QModelIndex());
        // update image
        ui->imageViewer->currentChanged(ui->treeView->currentIndex(), QModelIndex());
    }
}
/******************************************************************************************************/
void MainWindow::toolbarContextMenuRequested(const QPoint &pos)
{
    QMenu m;

    QAction *a = m.addAction(tr("Display labels"), this, SLOT(toggleToolbarLabels()));
    a->setCheckable(true);
    a->setChecked(ui->mainToolBar->toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

    m.exec(ui->mainToolBar->mapToGlobal(pos));
}
/******************************************************************************************************/
void MainWindow::toggleToolbarLabels()
{
    if(ui->mainToolBar->toolButtonStyle() == Qt::ToolButtonTextBesideIcon)
        ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    else
        ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}
/******************************************************************************************************/
void MainWindow::currentRowChanged(const QModelIndex &current, const QModelIndex &/*previous*/)
{
    QString log;

    log = current.sibling(current.row(), columnItemNames::logtext).data().toString();
    ui->logText->setPlainText(log);
}
/******************************************************************************************************/
void MainWindow::selectionChanged(const QItemSelection&, const QItemSelection&)
{
    updateActionState();
}
/******************************************************************************************************/
void MainWindow::treeContextMenuRequest(const QPoint &pos)
{
    auto treeContextMenu = new QMenu(this);

    treeContextMenu->addAction(IconProvider::folder(),  tr("Open &Directory"),      this, SLOT(treeOpenDirectory()));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    treeContextMenu->addAction(IconProvider::video(),   tr("Open &Movie"),          Qt::Key_F3/*to generate hint*/,     this, SLOT(treeOpenMovie()));
#else
    treeContextMenu->addAction(IconProvider::video(),   tr("Open &Movie"),          this, SLOT(treeOpenMovie()),        Qt::Key_F3);
#endif
    treeContextMenu->addAction(ui->actionRemoveItemfromSidebar);
    treeContextMenu->addAction(IconProvider::plus(),  tr("&Expand all"),          ui->treeView, SLOT(expandAll())     );
    treeContextMenu->addAction(IconProvider::minus(), tr("&Collapse all"),        ui->treeView, SLOT(collapseAll())   );
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    treeContextMenu->addAction(IconProvider::refresh(), tr("&Recreate Thumbnail"),  Qt::Key_F5/*to generate hint*/,     this, SLOT(recreateThumbnail()));
#else
    treeContextMenu->addAction(IconProvider::refresh(), tr("&Recreate Thumbnail"),  this, SLOT(recreateThumbnail()),    Qt::Key_F5/*to generate hint*/);
#endif

    foreach (auto a, pluginActions)
        treeContextMenu->addAction(a);

    treeContextMenu->exec(ui->treeView->mapToGlobal(pos));
}
/******************************************************************************************************/
void MainWindow::treeItemDoubleClicked(const QModelIndex &selIndex)
{
    QString movieFileName = selIndex.sibling(
                selIndex.row(),
                columnItemNames::path
                ).data().toString();

    if(!movieFileName.isEmpty())
    {
        QFileInfo f(movieFileName);
        if(f.exists() && f.isFile())
            QDesktopServices::openUrl(QUrl::fromLocalFile(movieFileName));
    }
}
/******************************************************************************************************/
void MainWindow::processUrls(QList<QUrl> urls)
{

    /* all dropped files */
    foreach (QUrl files, urls)
    {
        qDebug() << "Processing URL: " << files;
        QString localFile = files.toLocalFile();
        QFileInfo fi(localFile);
        addRecentFile(localFile);

        if(fi.isDir())
            dir2DirItem(QDir(fi.absoluteFilePath()), profileModel->getCurrentSettingsData().max_dir_depth, true);
        else
            if(fi.isFile())
                fileInfo2DirItem(fi);
    }

    qDebug() << "processing dirs: "<<processingDirs;

    updateRecentFileActions();

    /* all unique directories in tree */
    foreach (QStandardItem *d, processingDirs) {
       if(d && d->hasChildren())
           datamodel->appendRow(d);
    }

    /* all dropped files processed */
    processingDirs.clear();

    /* show tree if dropped video files */
    if(datamodel->rowCount()>0)
    {
        ui->placeholderLabel->hide();
        ui->treeView->show();
    }
}
/******************************************************************************************************/
void MainWindow::treeOpenDirectory()
{
    if(datamodel->rowCount()>0)
    {
        QModelIndex selIndex = ui->treeView->currentIndex();

        QString movieFileName = selIndex.sibling(
                    selIndex.row(),
                    columnItemNames::path
                    ).data().toString();

        // File Item
        if(!movieFileName.isEmpty())
        {
            QFileInfo f(movieFileName);
            if(f.exists())
            {
                if(f.isDir())
                    QDesktopServices::openUrl(QUrl::fromLocalFile(f.absoluteFilePath()));
                else
                    if(f.isFile())
                        QDesktopServices::openUrl(QUrl::fromLocalFile(f.absoluteDir().absolutePath()));
            }
        }
        else
        // Directory Item
        {
            QFileInfo f(selIndex.model()->index(0,columnItemNames::path, selIndex).data().toString());

            if(f.exists() && f.isFile())
                QDesktopServices::openUrl(QUrl::fromLocalFile(f.absoluteDir().absolutePath()));
        }
    }
}
/******************************************************************************************************/
void MainWindow::treeOpenMovie()
{
    if(datamodel->rowCount()>0)
        treeItemDoubleClicked(ui->treeView->currentIndex());
}
/******************************************************************************************************/
void MainWindow::changedProcessingItemsNumber(int delta)
{
    gardian.lock();
    processingItems += delta;
    gardian.unlock();

    refreshStatusBar();
    updateActionState();
}
/******************************************************************************************************/
void MainWindow::recreateThumbnail()
{
    auto selIdx = ui->treeView->selectionModel()->selectedRows();
    QList<QModelIndex> refreshList;

    foreach (auto ix, selIdx)
    {
        if(ix.parent().isValid())
            refreshList.append(ix);
        else
        {
            int i=0;
            QModelIndex fileIx;

            while( (fileIx = ix.model()->index(i++, 0, ix)).isValid())
            {
                if( !refreshList.contains(fileIx))
                    refreshList.append(fileIx);
            }
        }
    }

    QModelIndex pathCell;

    foreach(auto refreshIx, refreshList)
    {
        pathCell = refreshIx.sibling(
                    refreshIx.row(),
                    columnItemNames::path
                    );

        if(!pathCell.data().toString().isEmpty())
            worker->enqueue(
                        datamodel->itemFromIndex(refreshIx.parent()),
                        refreshIx.row());
    }
}
/******************************************************************************************************/
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::NoModifier)
    {
        switch (event->key()) {
        case Qt::Key_F3:
            this->treeOpenMovie();
            break;

        case Qt::Key_F5:
            this->recreateThumbnail();
                break;
        default:
            event->ignore();
            return;
        }
        event->accept();
    }
    event->ignore();
}
/******************************************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings s;

    s.setValue("mainform/geometry", saveGeometry());
    s.setValue("mainform/state", saveState());
    s.setValue("mainform/splitter", ui->splitter->saveState());
    s.setValue("mainform/toolbarlabels", static_cast<int>(ui->mainToolBar->toolButtonStyle()));

    s.setValue("recentFiles", recentFiles);
    s.setValue("MaxRecentFiles", maxRecentFiles);

    if(ui->imageViewer->isFullScreen())
        ui->imageViewer->close();

    QMainWindow::closeEvent(event);
}
/******************************************************************************************************/
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}
/******************************************************************************************************/
void MainWindow::dropEvent(QDropEvent *event)
{
    // All droped files and directories
    processUrls(event->mimeData()->urls());
}
/******************************************************************************************************/
QStandardItem* MainWindow::dir2DirItem(const QDir dir, const int recursion_depth, const bool topLevel)
{
    QStandardItem *iChildDir, *iDir=nullptr;
    QList<QStandardItem*> iChildren;

    if(recursion_depth > 0)
    {
        qDebug() << "Processing directory:" << dir.absolutePath();
        //Merge toplevel directories
        if(topLevel && processingDirs.contains(dir.absolutePath()))
            iDir = processingDirs[dir.absolutePath()];
        else
        {
            iDir = new QStandardItem(IconProvider::folder(), dir.dirName());
            iDir->setEditable(false);
        }
        //Folders
        foreach (QFileInfo fi, dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name))
        {
            iChildDir = dir2DirItem(QDir(fi.absoluteFilePath()), recursion_depth-1, false);
            if(iChildDir)
                iChildren.append(iChildDir);
        }
        if(!iChildren.isEmpty())
            iDir->appendRows(iChildren);
        iChildren.clear();

        //Files
        foreach(QFileInfo entry, dir.entryInfoList(QDir::Files, QDir::Name))
            fileInfo2FileItem(entry, iDir);

        if(iDir->hasChildren())
        {
            if(topLevel)
                processingDirs[dir.absolutePath()] = iDir;
        }
        else
        {
            delete iDir;
            iDir=Q_NULLPTR;
        }
    }
    return iDir;
}
/******************************************************************************************************/
QStandardItem* MainWindow::fileInfo2DirItem(QFileInfo file)
{
    QStandardItem *iDir;
    QString absDirPath = file.dir().absolutePath();

    if(processingDirs.contains(absDirPath))
        iDir = processingDirs[absDirPath];
    else
    {
        iDir = new QStandardItem(IconProvider::folder(), file.dir().dirName());
        iDir->setEditable(false);
        processingDirs[absDirPath] = iDir;
    }
    fileInfo2FileItem(file, iDir);

    return iDir;
}
/******************************************************************************************************/
/**
 * @return true if fileitem created, othewise false
 */
bool MainWindow::fileInfo2FileItem(QFileInfo file, QStandardItem *parent)
{
    QStandardItem *iFile, *iAbsFile, *iLog, *iOutputFile;

    qDebug() << "Processing file: "<<file.filePath();
    if(isVideoFile(file))
    {
        iFile = new QStandardItem(file.fileName());                 iFile->setEditable(false);
        iAbsFile = new QStandardItem(file.absoluteFilePath());      iAbsFile->setEditable(false);
        iLog = new QStandardItem();                                 iLog->setEditable(false);
        iOutputFile = new QStandardItem();                          iOutputFile->setEditable(false);

        QList<QStandardItem*> cols;
        cols << iFile << iAbsFile << iLog << iOutputFile;
        parent->appendRow(cols);

        worker->enqueue(parent, parent->rowCount()-1);

        return true;
    }
    return false;
}
/******************************************************************************************************/
bool MainWindow::isVideoFile(QFileInfo file)
{
    QString file_suff = file.suffix();

    /* based on extension */
    if(videoExtensions.contains(file_suff, Qt::CaseInsensitive))
        return true;
    else
    {
        qWarning().noquote()
            << tr("File extension") << file_suff << tr("not allowed!")
            << tr("Modify VideoExtension item in Application settings to enable it.")
            << QString("( %1 %2 )").arg(tr("see"), settingsFileName);
        return false;
    }

    /* based on mime type
    QMimeDatabase mimedb;
    QMimeType mType = mimedb.mimeTypeForFile(entry, QMimeDatabase::MatchContent);
    */
}
/******************************************************************************************************/
void MainWindow::createStatusBarWidgets()
{
    auto s = statusBar();
    qApp->setStyleSheet("QStatusBar::item { border-width: 0; }");

    sProfile = new QLabel(s);
    sColumns = new QLabel(s);
    sRows = new QLabel(s);
    sOutput = new QLabel(s);
    sStep = new QLabel(s);
    sSuffix = new QLabel(s);

    sItemsCnt = new QLabel(s);

    sOverwrite = new QCheckBox(QString("%1:").arg(tr("Overwrite")), s);
    sOverwrite->setLayoutDirection(Qt::RightToLeft);
    sOverwrite->setAttribute(Qt::WA_TransparentForMouseEvents);
    sOverwrite->setFocusPolicy(Qt::NoFocus);

    s->addWidget(sProfile);
    s->addWidget(sColumns);
    s->addWidget(sStep);
    s->addWidget(sRows);
    s->addWidget(sOutput);
    s->addWidget(sSuffix);
    s->addWidget(sOverwrite);
    s->addPermanentWidget(sItemsCnt);
}
/******************************************************************************************************/
void MainWindow::refreshStatusBar()
{
    auto d = profileModel->getCurrentSettingsData();

    sProfile->setText(QString("%1: %2 |").arg(tr("Profile"),d.settingsName));
    sColumns->setText(QString("%1: %2 |").arg(tr("Columns"), QString::number(d.columns)));

    sStep->setText(QString("%1: %2s |").arg(tr("Step")).arg(QString::number(d.step)));
    sStep->setVisible(d.rows<=0);

    sRows->setText(QString("%1: %2 |").arg(tr("Rows")).arg(d.rows));
    sRows->setVisible(d.rows>0);

    sOutput->setText(QString("%1: %2 |").arg(tr("Output"), d.output_directory));
    sOutput->setHidden(d.output_directory.isEmpty());

    sSuffix->setText(QString("%1: %2 |").arg(tr("Suffix"), d.suffix));
    sSuffix->setHidden(d.suffix.isEmpty());

    sOverwrite->setChecked(d.overwrite);

    if(processingItems == 0)
        sItemsCnt->clear();
    else
        sItemsCnt->setText(QString("%1: %2").arg(tr("Processing items")).arg(processingItems));
}
/******************************************************************************************************/
void MainWindow::on_action_Settings_triggered()
{
    if(profileModel == Q_NULLPTR)
        profileModel = new ProfileModel(this);
    SettingsDialog *dial = new SettingsDialog(this, /*worker.allSettings(), */profileModel);

    if(dial->exec() == QDialog::Accepted)
        refreshStatusBar();
}
/******************************************************************************************************/
void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
/******************************************************************************************************/
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About..."),
R"(
<html>
    <p>Movie Thumbnailer for creating thumbnails is frontend of CLI <a href="https://gitlab.com/movie_thumbnailer/mtn/wikis/home/">mtn</a>.</p>
    <p>More details at <a href="https://gitlab.com/movie_thumbnailer/qmtn/wikis/home/">HomePage</a>.</p>
    <p>Version:
)"
+AppVersionGit+
R"(</p
    <p>Features:
        <ul>
            <li>Drag&drop files and folders</li>
            <li>Recursive search for movie files</li>
            <li>Instant image making in background</li>
            <li>Image and output log preview</li>
            <li>Open image in external image viewer</li>
            <li>Recreate image with new settings</li>
            <li>Settings for managing mtn switches</li>
            <li>Extracting album art</li>
        </ul>
    </p>
    <p>
    <code>
            Qt Movie Thumbnailer comes with ABSOLUTELY NO WARRANTY.
            This is free software, and you are welcome
            to redistribute it under certain conditions; see <a href="https://www.gnu.org/licenses/gpl.html">GPLv3</a> for details.
    </code>
    </p>
</html>
            )"
);
}
/******************************************************************************************************/
void MainWindow::on_actionOpenFile_triggered()
{
    auto files = QFileDialog::getOpenFileUrls(this, qApp->applicationName());

    if(!files.isEmpty())
        processUrls(files);
}
/******************************************************************************************************/
void MainWindow::on_actionOpenDirectory_triggered()
{
    QUrl directory = QFileDialog::getExistingDirectoryUrl(this, qApp->applicationName(), QUrl(), QFileDialog::ReadOnly|QFileDialog::ShowDirsOnly);

    if(!directory.isEmpty())
        processUrls({directory});
}
/******************************************************************************************************/
void MainWindow::on_actionRefreshThumbnail_triggered()
{
    recreateThumbnail();
}
/*****************************************************************************************************/
void MainWindow::on_actionRemoveItemfromSidebar_triggered()
{
    auto treeModel = ui->treeView->model();
    auto selRows = ui->treeView->selectionModel()->selectedRows();
    QList<QModelIndex>::reverse_iterator sel;

    for (sel = selRows.rbegin(); sel != selRows.rend(); sel++)
    {
        treeModel->removeRow(sel->row(), sel->parent());

        if(sel->parent().isValid() && !treeModel->index(0, 0, sel->parent()).isValid())
            treeModel->removeRow(sel->parent().row());
    }
}
/******************************************************************************************************/
void MainWindow::updateActionState()
{
    bool allowed = (
        ui->treeView->selectionModel()->selectedIndexes().count() > 0
        &&
        processingItems == 0
    );

    ui->actionRemoveItemfromSidebar->setEnabled(allowed);
    ui->actionRefreshThumbnail->setEnabled(allowed);
}
/******************************************************************************************************/
void MainWindow::loadPlugins()
{
    if(pluginsLocation.isEmpty())
        return;

    QDir pluginsDir(pluginsLocation);

    if(pluginsDir.exists())
    {
        foreach(QString fileName, pluginsDir.entryList(QDir::Files))
        {
            QString pluginFileName = pluginsDir.absoluteFilePath(fileName);
            QPluginLoader loader(pluginFileName);

            QObject *plugin = loader.instance();

            if(plugin)
            {
                qDebug() << "plugin loaded:" << pluginFileName;

                ImageProcessInteface *uploader = qobject_cast<ImageProcessInteface *>(plugin);
                if(uploader)
                {
                    addPluginToMenu(plugin, uploader->Name());
                    connect(dynamic_cast<QObject*>(uploader),
                            SIGNAL(finished(bool)), this,
                            SLOT(pluginFinished(bool)));
                }
            }
            else
                qDebug() << loader.errorString();
        }
    }
}
/******************************************************************************************************/
void MainWindow::addPluginToMenu(QObject *plugin, QString text)
{
    if(pluginActions.count() == 0)
        pluginActions << ui->menu_Edit->addSection(QIcon(":icons/plug.svg"), "Plugins");

    QAction *action = new QAction(QIcon(":upload"), text, plugin);
    connect(action, &QAction::triggered, this, &MainWindow::pluginTriggered);

    ui->menu_Edit->addAction(action);
    pluginActions << action;
}
/******************************************************************************************************/
void MainWindow::showPluginResult(QWidget *parent, ImageProcessInteface *i)
{
    auto dlg = new QDialog(this);
    auto layout = new QHBoxLayout(dlg);
#ifdef QT_WEBENGINEWIDGETS_LIB
    auto webEngine = new QWebEngineView(dlg);
    layout->addWidget(webEngine);
    webEngine->load(i->getResultUrl());
#else
    auto webBrowser = new QTextEdit(this);
    webBrowser->setReadOnly(true);
    layout->addWidget(webBrowser);
    webBrowser->setHtml(i->getResultPage());
#endif
    dlg->setLayout(layout);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    dlg->setWindowTitle(i->Name());
    dlg->setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    dlg->resize(parent->size() * 0.8);
    dlg->show();
}
/******************************************************************************************************/
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        processUrls({QUrl::fromLocalFile(action->data().toString())});

}
/******************************************************************************************************/
void MainWindow::pluginTriggered()
{
    if(datamodel->rowCount()>0)
    {
        auto selectedIndex = ui->treeView->currentIndex();

        QString imageFileName = selectedIndex.sibling(
                    selectedIndex.row(),
                    columnItemNames::output
                    ).data().toString();

        if(!imageFileName.isEmpty())
        {
            auto pluginAction = qobject_cast<QAction*>(sender());
            auto iPlugin = qobject_cast<ImageProcessInteface*>(pluginAction->parent());
            if(iPlugin)
            {
                if(QMessageBox::question(this, tr("Question"),
                                     tr("Dou you want to upload file '%1' to '%2'?")
                                     .arg(imageFileName, iPlugin->Name())) == QMessageBox::Yes)
                {
                    qDebug() << "Running plugin:" << iPlugin->Name();
                    iPlugin->process(imageFileName);
                }
            }
        }
        else
            QMessageBox::information(this, tr("Information"), tr("Select a movie item"));
    }
    else
        QMessageBox::information(this, tr("Information"), tr("Nothing to upload"));

}
/******************************************************************************************************/
void MainWindow::pluginFinished(bool success)
{
    auto plugin = qobject_cast<ImageProcessInteface*>(sender());
    qDebug() << "success: " << success;

    if(success)
        showPluginResult(this, plugin);
    else
        QMessageBox::critical(this, plugin->Name(), plugin->getErrText());
}
/******************************************************************************************************/
void MainWindow::updateRecentFileActions()
{
    int numRecentFiles = qMin(recentFiles.size(), maxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = QString("&%1 %2").arg(i + 1).arg(strippedName(recentFiles[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(recentFiles[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}
/******************************************************************************************************/
void MainWindow::addRecentFile(QString fileName)
{
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);

    while(recentFiles.size() > maxRecentFiles)
        recentFiles.removeLast();
}
/******************************************************************************************************/
void MainWindow::createRecentFiles()
{
    for (int i = 0; i < maxRecentFiles; ++i) {
        recentFileActs.append(new QAction(this));
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }
}
/******************************************************************************************************/
void MainWindow::createRecentMenu()
{
    ui->menu_File->addSeparator();

    separatorAct = ui->menu_File->addSeparator();

    for (int i = 0; i < maxRecentFiles; ++i)
        ui->menu_File->addAction(recentFileActs[i]);

    updateRecentFileActions();
}
/******************************************************************************************************/
QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
/******************************************************************************************************/
