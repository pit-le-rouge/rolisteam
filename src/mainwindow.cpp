/*************************************************************************
 *        Copyright (C) 2007 by Romain Campioni                          *
 *        Copyright (C) 2009 by Renaud Guezennec                         *
 *        Copyright (C) 2010 by Berenger Morel                           *
 *        Copyright (C) 2010 by Joseph Boudou                            *
 *                                                                       *
 *        http://www.rolisteam.org/                                      *
 *                                                                       *
 *   rolisteam is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published   *
 *   by the Free Software Foundation; either version 2 of the License,   *
 *   or (at your option) any later version.                              *
 *                                                                       *
 *   This program is distributed in the hope that it will be useful,     *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
 *   GNU General Public License for more details.                        *
 *                                                                       *
 *   You should have received a copy of the GNU General Public License   *
 *   along with this program; if not, write to the                       *
 *   Free Software Foundation, Inc.,                                     *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           *
 *************************************************************************/

#include <QApplication>
#include <QBitmap>
#include <QBuffer>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QStringBuilder>
#include <QTime>
#include <QUrl>
#include <QUuid>
#include <QStatusBar>
#include <QCommandLineParser>


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "toolsbar.h"
#include "map/mapframe.h"
#include "map/map.h"
#include "chat/chatlistwidget.h"
#include "network/networkmanager.h"
#include "Image.h"
#include "network/networkmessagewriter.h"

#include "data/persons.h"
#include "userlist/playersList.h"
#include "userlist/playersListWidget.h"
#include "preferences/preferencesdialog.h"
#include "services/updatechecker.h"
#include "improvedworkspace.h"
#include "data/mediacontainer.h"
#include "network/receiveevent.h"
#include "textedit.h"
#include "variablesGlobales.h"

#ifndef NULL_PLAYER
#include "audioPlayer.h"
#endif

// singleton to the mainwindow
MainWindow* MainWindow::m_singleton= NULL;

MainWindow::MainWindow()
    : QMainWindow(),m_networkManager(NULL),m_localPlayerId(QUuid::createUuid().toString()),m_ui(new Ui::MainWindow),m_resetSettings(false)
{
    setAcceptDrops(true);
    m_supportedCharacterSheet=tr("Character Sheets files (*.xml)");
    m_pdfFiles=tr("Pdf File (*.pdf)");
    m_supportedNotes=tr("Supported Text files (*.html *.txt)");
    m_supportedMap=tr("Supported Map files (*.pla )");


    m_ui->setupUi(this);
    m_shownProgress=false;

    m_preferences = PreferencesManager::getInstance();
    m_downLoadProgressbar = new QProgressBar();
    m_downLoadProgressbar->setRange(0,100);

    m_downLoadProgressbar->setVisible(false);

    //m_mapWizzard = new MapWizzard(this);
    m_networkManager = new NetworkManager(m_localPlayerId);

    connect(m_networkManager,SIGNAL(notifyUser(QString)),this,SLOT(notifyUser(QString)));
    m_ipChecker = new IpChecker(this);
    //G_NetworkManager = m_networkManager;
    m_mapAction = new QMap<MediaContainer*,QAction*>();


}

MainWindow::~MainWindow()
{
    delete m_dockLogUtil;
}
void MainWindow::aboutRolisteam()
{
    QMessageBox::about(this, tr("About Rolisteam"),
                       tr("<h1>Rolisteam v%1</h1>"
                          "<p>Rolisteam helps you to manage a tabletop role playing game with remote friends/players. "
                          "It provides many features to share maps, pictures and it also includes tool to communicate with your friends/players. "
                          "The goal is to make Rolisteam-managed RPG games as good as RPG games around your table. To achieve it, we are working hard to provide you more and more features. "
                          "Existing features : Map sharing (with permission management), Image sharing, background music, dice roll and so on. Rolisteam is written in Qt5").arg(m_version) %
                       tr("<p>You may modify and redistribute the program under the terms of the GPL (version 2 or later)."
                          "A copy of the GPL is contained in the 'COPYING' file distributed with Rolisteam."
                          "Rolisteam is copyrighted by its contributors.  See the 'COPYRIGHT' file for the complete list of contributors.  We provide no warranty for this program.</p>") %
                       tr("<p><h3>Web Sites :</h3>"
                          "<ul>"
                          "<li><a href=\"http://www.rolisteam.org/\">Official Rolisteam Site</a></li> "
                          "<li><a href=\"https://github.com/Rolisteam/rolisteam/issues\">Bug Tracker</a></li> "
                          "</ul></p>"
						  "<p><h3>Current developers :</h3>"
						  "<ul>"
						  "<li><a href=\"http://www.rolisteam.org/contact\">Renaud Guezennec</a></li>"
						  "</ul></p> "
						   "<p><h3>Contributors :</h3>"
                          "<ul>"
                          "<li><a href=\"mailto:joseph.boudou@matabio.net\">Joseph Boudou</a></li>"
                          "<li><a href=\"mailto:rolistik@free.fr\">Romain Campioni</a> (rolistik)  </li>"
                          "</ul></p>"
                          "<p><h3>Translators</h3>"
						  "<table>"
						  "<tr><td><a href=\"https://www.transifex.com/accounts/profile/Le_Sage/\">Renaud Guezennec</a></td><td>(English & French)</td></tr>"
						  "<tr><td><a href=\"https://www.transifex.com/accounts/profile/lorrampi/\">Lorram Lomeu de Souza Rampi</a></td><td> (Portuguese)</td></tr>"
						  "<tr><td><a href=\"https://www.transifex.com/accounts/profile/JuAlves/\">Juliana Alves de Sousa Rampi</a></td><td> (Portuguese)</td></tr>"
						  "<tr><td><a href=\"https://www.transifex.com/accounts/profile/KrekoG/\">Gergely Krekó</a></td><td> (Hungarian)</td></tr>"
						  "<tr><td><a href=\"https://www.transifex.com/accounts/profile/kayazeren/\">Kaya Zeren</a></td><td> (Turkish)</td></tr>"
						  "<tr><td><a href=\"https://www.transifex.com/accounts/profile/IGrumoI/\">Alexia Béné</a></td><td> (French & German)</td></tr>"
						  "</table>"));
}
void MainWindow::addMediaToMdiArea(MediaContainer* mediac)
{
    QAction *action = m_ui->m_menuSubWindows->addAction(mediac->getTitle());
    action->setCheckable(true);
    action->setChecked(true);

    mediac->setAction(action);
    setLatestFile(mediac->getCleverUri());

    m_mdiArea->addContainerMedia(mediac);


    if(mediac->getTitle().isEmpty())
    {
        mediac->setTitle(tr("Unknown Map"));
    }

    m_mapAction->insert(mediac,action);
    mediac->setVisible(true);
    mediac->setFocus();
}
void  MainWindow::closeConnection()
{
    if(NULL!=m_networkManager)
    {
        m_networkManager->disconnectAndClose();
        m_ui->m_connectionAction->setEnabled(true);
        m_ui->m_disconnectAction->setEnabled(false);
    }
}
void MainWindow::closeAllImagesAndMaps()
{

    foreach(QMdiSubWindow* tmp,m_pictureList)
    {
        if(NULL!=tmp)
        {
            Image* imageWindow = dynamic_cast<Image*>(tmp->widget());

            if(NULL!=imageWindow)
            {
                removePictureFromId(imageWindow->getImageId());
            }
        }
    }

    foreach(MapFrame* tmp,m_mapWindowMap)
    {
        if(NULL!=tmp)
        {
            removeMapFromId(tmp->getMapId());
        }
    }
}

void MainWindow::closeMapOrImage()
{

    QMdiSubWindow* subactive = m_mdiArea->currentSubWindow();
    QWidget* active = subactive;
    MapFrame* bipMapWindow = NULL;

    if (NULL!=active)
    {
		QAction* action=NULL;

        Image*  imageFenetre = dynamic_cast<Image*>(active);

        QString mapImageId;
        QString mapImageTitle;
        mapImageTitle = active->windowTitle();
        bool image=false;
        //it is image
        if(NULL!=imageFenetre)
        {
            m_pictureList.removeOne(imageFenetre);

            mapImageId = imageFenetre->getImageId();
            image = true;
            action = imageFenetre->getAction();
        }
        else//it is a map
        {
            bipMapWindow= dynamic_cast<MapFrame*>(active);
            if(NULL!=bipMapWindow)
            {
                mapImageId = bipMapWindow->getMapId();
                action = bipMapWindow->getAction();

            }
            else// it is undefined
            {
                return;
            }
        }

        QMessageBox msgBox(this);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel );
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.move(QPoint(width()/2, height()/2) + QPoint(-100, -50));
        Qt::WindowFlags flags = msgBox.windowFlags();
        msgBox.setWindowFlags(flags ^ Qt::WindowSystemMenuHint);


        if (!image)
        {
            msgBox.setWindowTitle(tr("Close Map"));
        }
        else
        {
            msgBox.setWindowTitle(tr("Close Picture"));
        }
        msgBox.setText(tr("Do you want to close %1 %2?\nIt will be closed for everybody").arg(mapImageTitle).arg(image?tr(""):tr("(Map)")));

        msgBox.exec();
        if (msgBox.result() != QMessageBox::Yes)
            return;

        if (!image)
        {
            NetworkMessageWriter msg(NetMsg::MapCategory,NetMsg::CloseMap);
            msg.string8(mapImageId);
            msg.sendAll();

            m_mapWindowMap.remove(mapImageId);
            m_playersListWidget->model()->changeMap(NULL);
            m_toolBar->changeMap(NULL);

        }
        else
        {
            NetworkMessageWriter msg(NetMsg::PictureCategory,NetMsg::DelPictureAction);
            msg.string8(mapImageId);
            msg.sendAll();
        }
        delete action;
        delete subactive;
    }
}
void MainWindow::checkUpdate()
{
    if(m_preferences->value("MainWindow_MustBeChecked",true).toBool())
    {
        m_updateChecker = new UpdateChecker();
        m_updateChecker->startChecking();
        connect(m_updateChecker,SIGNAL(checkFinished()),this,SLOT(updateMayBeNeeded()));
    }
}
void MainWindow::changementFenetreActive(QMdiSubWindow *subWindow)
{
	bool localPlayerIsGM = PlayersList::instance()->localPlayer()->isGM();
    if((NULL!=subWindow) )
    {
        if (subWindow->objectName() == QString("MapFrame") && (localPlayerIsGM))
        {
            m_ui->m_closeAction->setEnabled(true);
            m_ui->m_saveAction->setEnabled(true);
            subWindow->setFocus();
        }
        else if(subWindow->objectName() == QString("Image") && ((localPlayerIsGM)))
        {
            m_ui->m_closeAction->setEnabled(true);
            m_ui->m_saveAction->setEnabled(false);
        }
    }
    else
    {
        m_ui->m_closeAction->setEnabled(false);
        m_ui->m_saveAction->setEnabled(false);
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(mayBeSaved())
    {
        //emit closing();
        if(NULL!=m_playerList)
            m_playerList->sendDelLocalPlayer();
        writeSettings();
        if(NULL!=m_noteEditor)
            m_noteEditor->close();
        event->accept();
    }
    else
    {
        event->ignore();
    }

}

void MainWindow::changementNatureUtilisateur()
{
    m_toolBar->updateUi();
    updateUi();
    updateWindowTitle();
}


void MainWindow::displayMinutesEditor(bool visible, bool isCheck)
{

    //m_noteEditor->setVisible(visible);
    if(NULL!=m_noteEditorSub)
    {
        m_noteEditorSub->setVisible(visible);
        m_noteEditor->setVisible(visible);
    }
    if (isCheck)
    {
        m_ui->m_showMinutesEditorAction->setChecked(visible);
    }

}

NetworkManager* MainWindow::getNetWorkManager()
{
    return m_networkManager;
}
MainWindow* MainWindow::getInstance()
{
    if(NULL==m_singleton)
    {
        m_singleton = new MainWindow();
    }
    return m_singleton;
}
Map::PermissionMode MainWindow::getPermission(int id)
{
    switch(id)
    {
    case 0:
        return Map::GM_ONLY;
    case 1:
        return Map::PC_MOVE;
    case 2:
        return Map::PC_ALL;
    default:
        return Map::GM_ONLY;
    }

}

void MainWindow::receiveData(quint64 readData,quint64 size)
{
    if(size==0)
    {
        m_downLoadProgressbar->setVisible(false);
        if(m_shownProgress)
        {
            statusBar()->removeWidget(m_downLoadProgressbar);
        }
        m_shownProgress=false;
        statusBar()->setVisible(false);
    }
    else if(readData!=size)
    {
        statusBar()->setVisible(true);
        statusBar()->addWidget(m_downLoadProgressbar);
        m_downLoadProgressbar->setVisible(true);
        quint64 i = (size-readData)*100/size;

        m_downLoadProgressbar->setValue(i);
        m_shownProgress=true;
    }

}


void MainWindow::createNotificationZone()
{
    m_dockLogUtil = new QDockWidget(tr("Notification Zone"), this);
    m_dockLogUtil->setObjectName("dockLogUtil");
    m_dockLogUtil->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_dockLogUtil->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_notifierDisplay = new QTextEdit(m_dockLogUtil);
    m_notifierDisplay->setReadOnly(true);
    m_dockLogUtil->setWidget(m_notifierDisplay);
    m_dockLogUtil->setMinimumWidth(125);
}

void MainWindow::linkActionToMenu()
{
    // file menu
    connect(m_ui->m_newMapAction, SIGNAL(triggered(bool)), this, SLOT(newMap()));
    connect(m_ui->m_newChatAction, SIGNAL(triggered(bool)), m_chatListWidget, SLOT(createPrivateChat()));
    connect(m_ui->m_newNoteAction, SIGNAL(triggered(bool)), this, SLOT(newNoteDocument()));
    connect(m_ui->m_openPictureAction, SIGNAL(triggered(bool)), this, SLOT(openContent()));
    connect(m_ui->m_openOnlinePictureAction, SIGNAL(triggered(bool)), this, SLOT(openContent()));
    connect(m_ui->m_openMapAction, SIGNAL(triggered(bool)), this, SLOT(openContent()));
    m_ui->m_recentFileMenu->setVisible(false);

    connect(m_ui->m_openStoryAction, SIGNAL(triggered(bool)), this, SLOT(openStory()));
    connect(m_ui->m_openNoteAction, SIGNAL(triggered(bool)), this, SLOT(openNote()));
    connect(m_ui->m_closeAction, SIGNAL(triggered(bool)), this, SLOT(closeMapOrImage()));
    connect(m_ui->m_saveAction, SIGNAL(triggered(bool)), this, SLOT(saveMap()));
    connect(m_ui->m_saveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveMap()));
    connect(m_ui->m_saveScenarioAction, SIGNAL(triggered(bool)), this, SLOT(saveStory()));
    //connect(m_ui->m_saveMinuteAct, SIGNAL(triggered(bool)), this, SLOT(saveMinutes()));
    connect(m_ui->m_preferencesAction, SIGNAL(triggered(bool)), m_preferencesDialog, SLOT(show()));

    // close
    connect(m_ui->m_quitAction, SIGNAL(triggered(bool)), this, SLOT(close()));

    // network
    connect(m_networkManager,SIGNAL(stopConnectionTry()),this,SLOT(stopReconnection()));
    connect(m_ui->m_disconnectAction,SIGNAL(triggered()),this,SLOT(closeConnection()));
    connect(m_ui->m_connectionAction,SIGNAL(triggered()),this,SLOT(startReconnection()));

    // Windows managing
    connect(m_ui->m_cascadeViewAction, SIGNAL(triggered(bool)), m_mdiArea, SLOT(cascadeSubWindows()));
    connect(m_ui->m_tabViewAction,SIGNAL(triggered(bool)),m_mdiArea,SLOT(setTabbedMode(bool)));
    connect(m_ui->m_tileViewAction, SIGNAL(triggered(bool)), m_mdiArea, SLOT(tileSubWindows()));

    // Help
    connect(m_ui->m_aboutAction, SIGNAL(triggered()), this, SLOT(aboutRolisteam()));
    connect(m_ui->m_onlineHelpAction, SIGNAL(triggered()), this, SLOT(helpOnLine()));


    //Note Editor
    connect(m_ui->m_showMinutesEditorAction, SIGNAL(triggered(bool)), this, SLOT(displayMinutesEditor(bool)));

}
void MainWindow::prepareMap(MapFrame* mapFrame)
{
    m_mapWindowMap.insert(mapFrame->getMapId(),mapFrame);
    Map* map = mapFrame->getMap();
    map->setPointeur(m_toolBar->getCurrentTool());
    map->setLocalIsPlayer(m_preferences->value("isPlayer",false).toBool());

    connect(m_toolBar,SIGNAL(currentToolChanged(ToolsBar::SelectableTool)),map,SLOT(setPointeur(ToolsBar::SelectableTool)));

    connect(m_toolBar,SIGNAL(currentNpcSizeChanged(int)),map,SLOT(setCharacterSize(int)));
    connect(m_toolBar,SIGNAL(currentPenSizeChanged(int)),map,SLOT(setPenSize(int)));
    connect(m_toolBar,SIGNAL(currentTextChanged(QString)),map,SLOT(setCurrentText(QString)));
    connect(m_toolBar,SIGNAL(currentNpcNameChanged(QString)),map,SLOT(setCurrentNpcName(QString)));
    connect(m_toolBar,SIGNAL(currentNpcNumberChanged(int)),map,SLOT(setCurrentNpcNumber(int)));

    connect(map, SIGNAL(changeCurrentColor(QColor)), m_toolBar, SLOT(changeCurrentColor(QColor)));
    connect(map, SIGNAL(incrementeNumeroPnj()), m_toolBar, SLOT(incrementNpcNumber()));
    connect(map, SIGNAL(mettreAJourPnj(int, QString)), m_toolBar, SLOT(updateNpc(int,QString)));

    connect(m_ui->m_showPcNameAction, SIGNAL(triggered(bool)), map, SIGNAL(afficherNomsPj(bool)));
    connect(m_ui->m_showNpcNameAction, SIGNAL(triggered(bool)), map, SIGNAL(afficherNomsPnj(bool)));
    connect(m_ui->m_showNpcNumberAction, SIGNAL(triggered(bool)), map, SIGNAL(afficherNumerosPnj(bool)));

    connect(m_ui->m_showNpcNameAction, SIGNAL(triggered(bool)), map, SLOT(setNpcNameVisible(bool)));
    connect(m_ui->m_showPcNameAction, SIGNAL(triggered(bool)), map, SLOT(setPcNameVisible(bool)));
    connect(m_ui->m_showNpcNumberAction,SIGNAL(triggered(bool)),map,SLOT(setNpcNumberVisible(bool)));

    map->setNpcNameVisible(m_ui->m_showNpcNameAction->isChecked());
    map->setPcNameVisible(m_ui->m_showPcNameAction->isChecked());
    map->setNpcNumberVisible(m_ui->m_showNpcNumberAction->isChecked());
    map->setCurrentNpcNumber(m_toolBar->getCurrentNpcNumber());
    map->setPenSize(m_toolBar->getCurrentPenSize());


    // new PlayersList connection
    connect(mapFrame, SIGNAL(activated(Map *)), m_playersListWidget->model(), SLOT(changeMap(Map *)));
    connect(mapFrame, SIGNAL(activated(Map *)), m_toolBar, SLOT(changeMap(Map *)));
}
void MainWindow::prepareImage(Image* imageFrame)
{
    m_pictureList.append(imageFrame);
    connect(m_toolBar,SIGNAL(currentToolChanged(ToolsBar::SelectableTool)),imageFrame,SLOT(setCurrentTool(ToolsBar::SelectableTool)));
    imageFrame->setCurrentTool(m_toolBar->getCurrentTool());
}


void MainWindow::updateWorkspace()
{
    QMdiSubWindow* active = m_mdiArea->currentSubWindow();
    if (NULL!=active)
    {
        changementFenetreActive(active);
    }
}
void MainWindow::newMap()
{
    MapFrame* mapFrame = new MapFrame(NULL, m_mdiArea);
    if(!mapFrame->createMap())
    {
        delete mapFrame;
    }
    else
    {
        prepareMap(mapFrame);
        addMediaToMdiArea(mapFrame);
        mapFrame->setVisible(true);

    }
}
void MainWindow::newNoteDocument()
{
    m_noteEditor->fileNew();
    displayMinutesEditor(true,true);
}
void MainWindow::sendOffAllMaps(NetworkLink * link)
{
    QMapIterator<QString, MapFrame*> i(m_mapWindowMap);
    while (i.hasNext())
    {
        i.next();
        i.value()->getMap()->setHasPermissionMode(m_playerList->everyPlayerHasFeature("MapPermission"));
        i.value()->getMap()->emettreCarte(i.value()->windowTitle(), link);
        i.value()->getMap()->emettreTousLesPersonnages(link);
    }

}
void MainWindow::sendOffAllImages(NetworkLink * link)
{
    NetworkMessageWriter message = NetworkMessageWriter(NetMsg::PictureCategory, NetMsg::AddPictureAction);

    foreach(Image* sub, m_pictureList)
    {
        //Image* img = dynamic_cast<Image*>(sub->widget());
        if(NULL!=sub)
        {
            sub->fill(message);
            message.sendTo(link);
        }
    }
}

void MainWindow::removeMapFromId(QString idMap)
{
        MapFrame* mapWindow = m_mapWindowMap.value(idMap);
        m_playersListWidget->model()->changeMap(NULL);
        m_toolBar->changeMap(NULL);
        if(NULL!=mapWindow)
        {
            delete m_mapAction->value(mapWindow);
            m_mapWindowMap.remove(idMap);
            delete mapWindow;
            //delete tmp;
        }
}
Map* MainWindow::findMapById(QString idMap)
{
    MapFrame* mapframe = m_mapWindowMap.value(idMap);
    if(NULL!=mapframe)
    {
        return mapframe->getMap();
    }
    else
    {
        return NULL;
    }
}
bool MainWindow::mayBeSaved(bool perteConnexion)
{
    // Creation de la boite d'alerte
    QMessageBox msgBox(this);
    QAbstractButton *boutonSauvegarder         = msgBox.addButton(QMessageBox::Save);
    QAbstractButton *boutonQuitter                 = msgBox.addButton(tr("Quit"), QMessageBox::RejectRole);
    //msgBox.move(QPoint(width()/2, height()/2) + QPoint(-100, -50));
    // On supprime l'icone de la barre de titre
    Qt::WindowFlags flags = msgBox.windowFlags();
    msgBox.setWindowFlags(flags ^ Qt::WindowSystemMenuHint);

    // Creation du message
    QString message;
    QString msg = m_preferences->value("Application_Name","rolisteam").toString();

    // S'il s'agit d'une perte de connexion
    if (perteConnexion)
    {
        message = tr("Connection has been lost. %1 will be close").arg(msg);
        // Icone de la fenetre
        msgBox.setIcon(QMessageBox::Critical);
        // M.a.j du titre et du message
        msgBox.setWindowTitle(tr("Connection lost"));
    }
    else
    {
        // Icone de la fenetre
        msgBox.setIcon(QMessageBox::Information);
        // Ajout d'un bouton
        msgBox.addButton(QMessageBox::Cancel);
        // M.a.j du titre et du message
        msgBox.setWindowTitle(tr("Quit %1 ").arg(msg));
    }

    // Si l'utilisateur est un joueur
    if (!PlayersList::instance()->localPlayer()->isGM())
    {
        message += tr("Do you want to save your minutes before to quit %1?").arg(msg);
    }
    // Si l'utilisateut est un MJ
    else
    {
        message += tr("Do you want to save your scenario before to quit %1?").arg(msg);

    }

    //M.a.j du message de la boite de dialogue
    msgBox.setText(message);
    // Ouverture de la boite d'alerte
    msgBox.exec();

    // Quit without saving
    if (msgBox.clickedButton() == boutonQuitter)
    {
        return true;
    }
    else if (msgBox.clickedButton() == boutonSauvegarder) //saving
    {
        bool ok;
        // Si l'utilisateur est un joueur, on sauvegarde les notes
        if (!PlayersList::instance()->localPlayer()->isGM())
            ok = saveMinutes();
        // S'il est MJ, on sauvegarde le scenario
        else
            ok = saveStory();

        if (ok || perteConnexion)
        {
            return true;
        }
    }
    return false;
}
void MainWindow::removePictureFromId(QString idImage)
{
    QMdiSubWindow* tmp = m_mdiArea->getSubWindowFromId(idImage);
    if(NULL!=tmp)
    {
        Image* imageWindow = dynamic_cast<Image*>(tmp->widget());

        if(NULL!=imageWindow)
        {
            m_pictureList.removeOne(imageWindow);
            delete imageWindow;
        }
        delete tmp;
    }
}
QWidget* MainWindow::registerSubWindow(QWidget * subWindow,QAction* action)
{
    return m_mdiArea->addWindow(subWindow,action);
}

void MainWindow::openNote()
{
    // open file name.
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Minutes"), m_preferences->value("MinutesDirectory",QDir::homePath()).toString(), m_noteEditor->getFilter());

    if (!filename.isEmpty())
    {
        QFileInfo fi(filename);
        m_preferences->registerValue("MinutesDirectory",fi.absolutePath() +"/");
        m_noteEditor->load(filename);
        displayMinutesEditor(true, true);
    }
}

void MainWindow::openStory()
{
    QString fichier = QFileDialog::getOpenFileName(this, tr("Open scenario"), m_preferences->value("StoryDirectory",QDir::homePath()).toString(), tr("Scenarios (*.sce)"));

    if (fichier.isNull())
        return;

    int dernierSlash = fichier.lastIndexOf("/");
    m_preferences->registerValue("StoryDirectory",fichier.left(dernierSlash));


    QFile file(fichier);
    if (!file.open(QIODevice::ReadOnly))
    {
        notifyUser("Cannot be read (ouvrirScenario - MainWindow.cpp)");
        return;
    }
    QDataStream in(&file);


    int nbrCartes;
    in >> nbrCartes;


    for (int i=0; i<nbrCartes; ++i)
    {
        QPoint posWindow;
        in >> posWindow;

        QSize sizeWindow;
        in >> sizeWindow;

        MapFrame* mapFrame = new MapFrame();


        if(mapFrame->readMapAndNpc(in))
        {
            prepareMap(mapFrame);
            addMediaToMdiArea(mapFrame);
            mapFrame->setGeometry(posWindow.x(),posWindow.y(),sizeWindow.width(),sizeWindow.height());
        }
        else
        {
            delete mapFrame;
        }
    }
    int nbrImages;
    in >>nbrImages;
    // in >>On lit toutes les images presentes dans le fichier
    for (int i=0; i<nbrImages; ++i)
    {
        readImageFromStream(in);
    }
    //reading minutes
    m_noteEditor->readFromBinary(in);
    bool tempbool;
    in >> tempbool;
    m_noteEditor->setVisible(tempbool);
    m_ui->m_showMinutesEditorAction->setChecked(tempbool);

    // closing file
    file.close();
}
////////////////////////////////////////////////////
// Save data
////////////////////////////////////////////////////

void MainWindow::saveMap()
{
    bool saveAs = false;
    QAction* action = qobject_cast<QAction*>(sender());
    if(m_ui->m_saveAsAction==action)
    {
        saveAs = true;
    }
    QWidget *active = m_mdiArea->activeWindow();
    MapFrame* mapWindow = static_cast<MapFrame*>(active);
    if ((NULL==mapWindow) || (mapWindow->objectName() != "MapFrame"))
    {
        notifyUser("Not a map (saveMap - mainwindow.cpp)");
        return;
    }
    QString fileName;
    bool hasCorrectPath= mapWindow->isUriEndWith(".pla");

    if(saveAs||!hasCorrectPath)
    {
        fileName= QFileDialog::getSaveFileName(this, tr("Save Map"), m_preferences->value("MapDirectory",QDir::homePath()).toString(), tr("Map (*.pla)"));
    }
    else
    {
        //no needs to check if cleverUri is valid, if not hasCorrectPath is false.
        fileName = mapWindow->getCleverUri()->getUri();
    }

    if (fileName.isNull())
        return;

    if (!fileName.endsWith(".pla"))
    {
        fileName += ".pla";
    }

    int dernierSlash = fileName.lastIndexOf("/");
    m_preferences->registerValue("MapDirectory",fileName.left(dernierSlash));

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        notifyUser("could not open file for writting (saveMap - mainwindow.cpp)");
        return;
    }
    QDataStream out(&file);
    mapWindow->getMap()->saveMap(out);
    file.close();
}
bool MainWindow::saveMinutes()
{
    return m_noteEditor->fileSave();
}
bool MainWindow::saveStory()
{
    // open file
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Scenarios"), m_preferences->value("StoryDirectory",QDir::homePath()).toString(), tr("Scenarios (*.sce)"));


    if (filename.isNull())
    {
        return false;
    }
    if(!filename.endsWith(".sce"))
    {
        filename += ".sce";
    }

    int dernierSlash = filename.lastIndexOf("/");
    m_preferences->registerValue("StoryDirectory",filename.left(dernierSlash));

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        notifyUser("cannot be open (sauvegarderScenario - MainWindow.cpp)");
        return false;
    }

    QDataStream out(&file);
    saveAllMap(out);
    saveAllImages(out);
    m_noteEditor->saveFileAsBinary(out);
    out << m_noteEditor->isVisible();
    file.close();
    return true;
}
void MainWindow::saveAllMap(QDataStream &out)
{
    out << m_mapWindowMap.size();
    QMapIterator<QString, MapFrame*> i(m_mapWindowMap);
    while (i.hasNext())
    {
        i.next();
        QPoint pos2 = i.value()->mapFromParent(i.value()->pos());
        out << pos2;
        out << i.value()->size();
        i.value()->getMap()->saveMap(out, i.value()->windowTitle());
    }
}

void MainWindow::saveAllImages(QDataStream &out)
{
    out << m_pictureList.size();
    foreach(Image* sub, m_pictureList)
    {
        if(NULL!=sub)
        {
            sub->saveImageToFile(out);
        }
    }
}
void MainWindow::stopReconnection()
{
    m_ui->m_connectionAction->setEnabled(true);
    m_ui->m_disconnectAction->setEnabled(false);
}


void MainWindow::startReconnection()
{
    if (PreferencesManager::getInstance()->value("isClient",true).toBool())
    {
        closeAllImagesAndMaps();
    }
    if(m_networkManager->startConnection())
    {
        m_ui->m_connectionAction->setEnabled(false);
        m_ui->m_disconnectAction->setEnabled(true);
    }
    else
    {
        m_ui->m_connectionAction->setEnabled(true);
        m_ui->m_disconnectAction->setEnabled(false);
    }
}
void MainWindow::showIp(QString ip)
{
    notifyUser(tr("Server Ip Address:%1\nPort:%2").arg(ip).arg(m_networkManager->getPort()));
}
void MainWindow::setUpNetworkConnection()
{
    if (PreferencesManager::getInstance()->value("isClient",true).toBool())
    {
        // We want to know if the server refuses local player to be GM
        connect(m_playerList, SIGNAL(localGMRefused()), this, SLOT(changementNatureUtilisateur()));
        // We send a message to del local player when we quit
        connect(this, SIGNAL(closing()), m_playerList, SLOT(sendDelLocalPlayer()));
    }
    else
    {
        //connect(m_networkManager, SIGNAL(linkAdded(NetworkLink *)), this, SLOT(updateSessionToNewClient(NetworkLink*)));
    }
    connect(m_networkManager, SIGNAL(dataReceived(quint64,quint64)), this, SLOT(receiveData(quint64,quint64)));

}
void MainWindow::readImageFromStream(QDataStream &file)
{
    QString title;
    QByteArray baImage;
    QPoint topleft;
    QSize size;

    file >> title;
    file >>topleft;
    file >> size;
    file >> baImage;

    QImage img;
    if (!img.loadFromData(baImage, "jpg"))
    {
        notifyUser(tr("Image compression error (readImageFromStream - MainWindow.cpp)"));
    }

    // Creation de l'identifiant
    QString idImage = QUuid::createUuid().toString();

    // Creation de la fenetre image
    Image *imageFenetre = new Image(m_mdiArea);
    imageFenetre->setTitle(title);
    imageFenetre->setLocalPlayerId(m_localPlayerId);
    imageFenetre->setImage(img);

    prepareImage(imageFenetre);
    addMediaToMdiArea(imageFenetre);

    connect(m_toolBar,SIGNAL(currentToolChanged(ToolsBar::SelectableTool)),imageFenetre,SLOT(setCurrentTool(ToolsBar::SelectableTool)));
    imageFenetre->setCurrentTool(m_toolBar->getCurrentTool());

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    if (!img.save(&buffer, "jpg", 70))
    {
        notifyUser(tr("Image compression error (readImageFromStream - MainWindow.cpp)"));
    }

    NetworkMessageWriter msg(NetMsg::PictureCategory,NetMsg::AddPictureAction);
    msg.string16(title);
    msg.string8(idImage);
    msg.string8(m_localPlayerId);
    msg.byteArray32(byteArray);
    msg.sendAll();
}
void MainWindow::helpOnLine()
{
    if (!QDesktopServices::openUrl(QUrl("http://wiki.rolisteam.org/")))
    {
        QMessageBox * msgBox = new QMessageBox(
                    QMessageBox::Information,
                    tr("Help"),
                    tr("Documentation of %1 can be found online at :<br> <a href=\"http://wiki.rolisteam.org\">http://wiki.rolisteam.org/</a>").arg(m_preferences->value("Application_Name","rolisteam").toString()),
                    QMessageBox::Ok
                    );
        msgBox->exec();
    }

}
void MainWindow::updateUi()
{
    /// @todo hide diametrePNj for players.
    m_toolBar->updateUi();
#ifndef NULL_PLAYER
    m_audioPlayer->updateUi();
#endif
    if(!PlayersList::instance()->localPlayer()->isGM())
    {
        m_ui->m_newMapAction->setEnabled(false);
        m_ui->m_openMapAction->setEnabled(false);
        m_ui->m_openStoryAction->setEnabled(false);
        m_ui->m_closeAction->setEnabled(false);
        m_ui->m_saveAction->setEnabled(false);
        m_ui->m_saveScenarioAction->setEnabled(false);
    }
	/*if(m_networkManager->isServer())
    {
        m_ui->m_connectionAction->setEnabled(false);
        m_ui->m_disconnectAction->setEnabled(true);
    }
    else
	{*/
        m_ui->m_connectionAction->setEnabled(false);
        m_ui->m_disconnectAction->setEnabled(true);
	//}
    updateRecentFileActions();
}
void MainWindow::updateMayBeNeeded()
{
    if(m_updateChecker->mustBeUpdated())
    {
        QMessageBox::information(this,tr("Update Monitor"),tr("The %1 version has been released. Please take a look at <a href=\"http://www.rolisteam.org/download\">Download page</a> for more information").arg(m_updateChecker->getLatestVersion()));
    }
    m_updateChecker->deleteLater();
}
void MainWindow::networkStateChanged(bool state)
{
    if(state)
    {
        m_ui->m_connectionAction->setEnabled(false);
        m_ui->m_disconnectAction->setEnabled(true);

        if((m_networkManager->isConnected())&&(m_networkManager->isServer())&&(NULL!=m_ipChecker))
        {
            m_ipChecker->startCheck();
        }
    }
    else
    {
        m_ui->m_connectionAction->setEnabled(true);
        m_ui->m_disconnectAction->setEnabled(false);
    }
}

void MainWindow::notifyAboutAddedPlayer(Player * player) const
{
    notifyUser(tr("%1 just joins the game.").arg(player->name()));
    if(player->getUserVersion().compare(m_version)!=0)
    {
        notifyUser(tr("%1 has not the right version: %2.").arg(player->name()).arg(player->getUserVersion()),Error);
    }
}

void MainWindow::notifyAboutDeletedPlayer(Player * player) const
{
    notifyUser(tr("%1 just leaves the game.").arg(player->name()));
}

void MainWindow::updateSessionToNewClient(Player* player)
{
    if(NULL!=player)
    {
        sendOffAllMaps(player->link());
        sendOffAllImages(player->link());
        m_preferencesDialog->sendOffAllDiceAlias(player->link());
    }
}

void MainWindow::setNetworkManager(NetworkManager* tmp)
{
    m_networkManager = tmp;
    m_networkManager->setParent(this);
}
void MainWindow::readSettings()
{
    QSettings settings("rolisteam",QString("rolisteam_%1/preferences").arg(m_version));

    if(m_resetSettings)
    {
        settings.clear();
    }

    restoreState(settings.value("windowState").toByteArray());
    bool  maxi = settings.value("Maximized", false).toBool();
    if(!maxi)
    {
        restoreGeometry(settings.value("geometry").toByteArray());
    }

    m_preferences->readSettings(settings);

    /**
      * management of recentFileActs
      * */
    m_maxSizeRecentFile = m_preferences->value("recentfilemax",5).toInt();
    for (int i = 0; i < m_maxSizeRecentFile; ++i)
    {
        m_recentFileActs.insert(i,new QAction(m_ui->m_recentFileMenu));
        m_recentFileActs[i]->setVisible(false);
        connect(m_recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));

        m_ui->m_recentFileMenu->addAction(m_recentFileActs[i]);
    }
    updateRecentFileActions();
    m_preferencesDialog->initializePostSettings();
}
void MainWindow::writeSettings()
{
    QSettings settings("rolisteam",QString("rolisteam_%1/preferences").arg(m_version));
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("Maximized", isMaximized());
    m_preferences->writeSettings(settings);
}
void MainWindow::parseCommandLineArguments(QStringList list)
{

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption port(QStringList() << "p"<< "port", tr("Set rolisteam to use <port> for the connection"),"port");
    QCommandLineOption hostname(QStringList() << "s"<< "server", tr("Set rolisteam to connect to <server>."),"server");
    QCommandLineOption role(QStringList() << "r"<< "role", tr("Define the <role>: gm or pc"),"role");
    QCommandLineOption reset(QStringList() << "reset-settings", tr("Erase the settings and use the default parameters"));
    QCommandLineOption user(QStringList() << "u"<<"user", tr("Define the <username>"),"username");
    QCommandLineOption translation(QStringList() << "t"<<"translation", QObject::tr("path to the translation file: <translationfile>"),"translationfile");

    parser.addOption(port);
    parser.addOption(hostname);
    parser.addOption(role);
    parser.addOption(reset);
    parser.addOption(user);
    parser.addOption(translation);

    parser.process(list);

    bool hasPort = parser.isSet(port);
    bool hasHostname = parser.isSet(hostname);
    bool hasRole = parser.isSet(role);
    bool hasUser = parser.isSet(user);
    m_resetSettings = parser.isSet(reset);

    QString portValue;
    QString hostnameValue;
    QString roleValue;
    QString username;
    if(hasPort)
    {
        portValue = parser.value(port);
    }
    if(hasHostname)
    {
        hostnameValue = parser.value(hostname);
    }
    if(hasRole)
    {
        roleValue = parser.value(role);
    }
    if(hasUser)
    {
        username = parser.value(user);
    }
    if(!(roleValue.isNull()&&hostnameValue.isNull()&&portValue.isNull()&&username.isNull()))
    {
        m_networkManager->setValueConnection(portValue,hostnameValue,username,roleValue);
    }
}
NetWorkReceiver::SendType MainWindow::processMessage(NetworkMessageReader* msg)
{
    NetWorkReceiver::SendType type;
    switch(msg->category())
    {
    case NetMsg::PictureCategory:
        processImageMessage(msg);
        type = NetWorkReceiver::AllExceptMe;
        break;
    case NetMsg::MapCategory:
        processMapMessage(msg);
        type = NetWorkReceiver::AllExceptMe;
        break;
    case NetMsg::NPCCategory:
        processNpcMessage(msg);
        type = NetWorkReceiver::AllExceptMe;
        break;
    case NetMsg::DrawCategory:
        processPaintingMessage(msg);
        type = NetWorkReceiver::ALL;
        break;
    case NetMsg::CharacterCategory:
        processCharacterMessage(msg);
        type = NetWorkReceiver::AllExceptMe;
        break;
    case NetMsg::ConnectionCategory:
        processConnectionMessage(msg);
        type = NetWorkReceiver::NONE;
        break;
    case NetMsg::CharacterPlayerCategory:
        processCharacterPlayerMessage(msg);
        type = NetWorkReceiver::AllExceptMe;
        break;
    }
    return type;//NetWorkReceiver::AllExceptMe;
}
void MainWindow::processConnectionMessage(NetworkMessageReader* msg)
{
    if(msg->action() == NetMsg::EndConnectionAction)
    {
        notifyUser(tr("End of the connection process"));
        updateWorkspace();
    }
}
void MainWindow::notifyUser(QString message, MessageType type) const
{
    static bool alternance = false;
    QColor color;
    alternance = !alternance;

    if (alternance)
        color = Qt::darkBlue;
    else
        color = Qt::darkRed;

    switch (type)
    {
    case Information:

        break;
    case Error:
        color = Qt::red;
        message.prepend(tr("Error:"));
        break;
    case Warning:
        color = Qt::darkRed;
        message.prepend(tr("Warning:"));
        break;
    case Notice:

        break;

    }

    QString time = QTime::currentTime().toString("hh:mm:ss") + " - ";
    m_notifierDisplay->moveCursor(QTextCursor::End);
    m_notifierDisplay->setTextColor(Qt::darkGreen);
    m_notifierDisplay->append(time);
    m_notifierDisplay->setTextColor(color);
    m_notifierDisplay->insertPlainText(message);
}
bool  MainWindow::showConnectionDialog()
{
    // Get a connection
    bool result = m_networkManager->configAndConnect(m_version);
#ifndef NULL_PLAYER
    m_audioPlayer->updateUi();
#endif
    return result;
}
void MainWindow::setupUi()
{
    // Initialisation de la liste des BipMapWindow, des Image et des Tchat
    m_mapWindowMap.clear();
    m_pictureList.clear();
    m_version=tr("unknown");
#ifdef VERSION_MINOR
    #ifdef VERSION_MAJOR
        #ifdef VERSION_MIDDLE
            m_version = QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MIDDLE).arg(VERSION_MINOR);
        #endif
    #endif
#endif
    setAnimated(false);
    m_mdiArea = new ImprovedWorkspace();
    setCentralWidget(m_mdiArea);
    connect(m_mdiArea, SIGNAL(subWindowActivated ( QMdiSubWindow * )), this, SLOT(changementFenetreActive(QMdiSubWindow *)));

    m_toolBar = new ToolsBar();
    addDockWidget(Qt::LeftDockWidgetArea, m_toolBar);
    m_ui->m_menuSubWindows->insertAction(m_ui->m_toolBarAct,m_toolBar->toggleViewAction());
    m_ui->m_menuSubWindows->removeAction(m_ui->m_toolBarAct);

    createNotificationZone();
    m_ui->m_menuSubWindows->insertAction(m_ui->m_notificationAct,m_dockLogUtil->toggleViewAction());
    m_ui->m_menuSubWindows->removeAction(m_ui->m_notificationAct);
    addDockWidget(Qt::RightDockWidgetArea, m_dockLogUtil);


    m_chatListWidget = new ChatListWidget(this);
    ReceiveEvent::registerNetworkReceiver(NetMsg::SharePreferencesCategory,m_chatListWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_chatListWidget);
    m_ui->m_menuSubWindows->insertAction(m_ui->m_chatListAct,m_chatListWidget->toggleViewAction());
    m_ui->m_menuSubWindows->removeAction(m_ui->m_chatListAct);


    ///////////////////
    //PlayerList
    ///////////////////
    m_playersListWidget = new PlayersListWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, m_playersListWidget);
    setWindowIcon(QIcon(":/logo.png"));
    m_ui->m_menuSubWindows->insertAction(m_ui->m_characterListAct,m_playersListWidget->toggleViewAction());
    m_ui->m_menuSubWindows->removeAction(m_ui->m_characterListAct);

    ///////////////////
    //Audio Player
    ///////////////////
#ifndef NULL_PLAYER
    m_audioPlayer = AudioPlayer::getInstance(this);
    m_networkManager->setAudioPlayer(m_audioPlayer);
    addDockWidget(Qt::RightDockWidgetArea,m_audioPlayer );
    m_ui->m_menuSubWindows->insertAction(m_ui->m_audioPlayerAct,m_audioPlayer->toggleViewAction());
    m_ui->m_menuSubWindows->removeAction(m_ui->m_audioPlayerAct);
#endif
    //readSettings();
    m_preferencesDialog = new PreferencesDialog(this);
    linkActionToMenu();
    // Creation de l'editeur de notes
    m_noteEditor= new TextEdit(this);
#ifdef Q_OS_MAC
    m_noteEditor->menuBar()->setNativeMenuBar(false);
#endif


    m_noteEditorSub  = static_cast<QMdiSubWindow*>(m_mdiArea->addWindow(m_noteEditor,m_ui->m_showMinutesEditorAction));
    if(NULL!=m_noteEditorSub)
    {
        m_noteEditorSub->setWindowTitle(tr("Minutes Editor[*]"));
        m_noteEditorSub->setWindowIcon(QIcon(":/notes.png"));
        m_noteEditorSub->hide();
    }
    connect(m_noteEditor,SIGNAL(showed(bool)),m_ui->m_showMinutesEditorAction,SLOT(setChecked(bool)));
    connect(m_noteEditor,SIGNAL(showed(bool)),m_noteEditorSub,SLOT(setVisible(bool)));
    // Initialisation des etats de sante des PJ/PNJ (variable declarees dans DessinPerso.cpp)
    m_playerList = PlayersList::instance();
    connect(m_playerList, SIGNAL(playerAdded(Player *)), this, SLOT(notifyAboutAddedPlayer(Player *)));
    connect(m_playerList, SIGNAL(playerAddedAsClient(Player*)), this, SLOT(updateSessionToNewClient(Player*)));
    connect(m_playerList, SIGNAL(playerDeleted(Player *)), this, SLOT(notifyAboutDeletedPlayer(Player *)));
    connect(m_networkManager,SIGNAL(connectionStateChanged(bool)),this,SLOT(updateWindowTitle()));
    connect(m_networkManager,SIGNAL(connectionStateChanged(bool)),this,SLOT(networkStateChanged(bool)));
    connect(m_ipChecker,SIGNAL(finished(QString)),this,SLOT(showIp(QString)));
}
void MainWindow::processMapMessage(NetworkMessageReader* msg)
{
    if(msg->action() == NetMsg::CloseMap)
    {
        QString idMap = msg->string8();
        removeMapFromId(idMap);
    }
    else
    {
        MapFrame* mapFrame = new MapFrame(NULL, m_mdiArea);
        if(!mapFrame->processMapMessage(msg,m_preferences->value("isPlayer",false).toBool()))
        {
            delete mapFrame;
        }
        else
        {
            //addMap(bipMapWindow,bipMapWindow->getTitle());
            prepareMap(mapFrame);
            addMediaToMdiArea(mapFrame);
            mapFrame->setVisible(true);
        }
    }
}

void MainWindow::processImageMessage(NetworkMessageReader* msg)
{
    if(msg->action() == NetMsg::AddPictureAction)
    {
        QString title = msg->string16();
        QString idImage = msg->string8();
        QString idPlayer = msg->string8();
        QByteArray dataImage = msg->byteArray32();

        QImage *img = new QImage;
        if (!img->loadFromData(dataImage, "jpg"))
        {
            notifyUser("Cannot read received image (receptionMessageImage - NetworkLink.cpp)");
        }
        Image* image = new Image();
        image->setTitle(title);
        image->setIdImage(idImage);
        image->setIdOwner(idPlayer);
        image->setImage(*img);

        //addImage(image, title);
        prepareImage(image);
        addMediaToMdiArea(image);
        image->setVisible(true);


    }
    else if(msg->action() == NetMsg::DelPictureAction)
    {
        QString idImage = msg->string8();
        removePictureFromId(idImage);
    }
}
void MainWindow::processNpcMessage(NetworkMessageReader* msg)
{
    QString idMap = msg->string8();
    if(msg->action() == NetMsg::addNpc)
    {
        Map* map = findMapById(idMap);
        extractCharacter(map,msg);

    }
    else if(msg->action() == NetMsg::delNpc)
    {
        QString idNpc = msg->string8();
        Map* map = findMapById(idMap);
        if(NULL!=map)
        {
            map->eraseCharacter(idNpc);
        }
    }
}
void MainWindow::processPaintingMessage(NetworkMessageReader* msg)
{
    if(msg->action() == NetMsg::penPainting)
    {
        QString idPlayer = msg->string8();
        QString idMap = msg->string8();

        quint32 pointNumber = msg->uint32();

        QList<QPoint> pointList;
        QPoint point;
        for (int i=0; i<pointNumber; i++)
        {
            quint16 pointX = msg->uint16();
            quint16 pointY = msg->uint16();
            point = QPoint(pointX, pointY);
            pointList.append(point);
        }
        quint16 zoneX = msg->uint16();
        quint16 zoneY = msg->uint16();
        quint16 zoneW = msg->uint16();
        quint16 zoneH = msg->uint16();

        QRect zoneToRefresh(zoneX,zoneY,zoneW,zoneH);

        quint8 diameter = msg->uint8();
        quint8 colorType = msg->uint8();
        QColor color(msg->rgb());

        Map* map = findMapById(idMap);

        if(NULL!=map)
        {
            SelectedColor selectedColor;
            selectedColor.color = color;
            selectedColor.type = (ColorKind)colorType;
            map->paintPenLine(&pointList,zoneToRefresh,diameter,selectedColor,idPlayer==m_localPlayerId);
        }
    }
    else if(msg->action() == NetMsg::textPainting)
    {
        QString idMap = msg->string8();
        QString text = msg->string16();
        quint16 posx = msg->uint16();
        quint16 posy = msg->uint16();
        QPoint pos(posx,posy);

        quint16 zoneX = msg->uint16();
        quint16 zoneY = msg->uint16();
        quint16 zoneW = msg->uint16();
        quint16 zoneH = msg->uint16();

        QRect zoneToRefresh(zoneX,zoneY,zoneW,zoneH);
        quint8 colorType = msg->uint8();
        QColor color(msg->rgb());

        Map* map = findMapById(idMap);

        if(NULL!=map)
        {
            SelectedColor selectedColor;
            selectedColor.color = color;
            selectedColor.type = (ColorKind)colorType;

            map->paintText(text,pos,zoneToRefresh,selectedColor);

        }
    }
    else if(msg->action() == NetMsg::handPainting)
    {

    }
    else
    {
        QString idMap = msg->string8();
        quint16 posx = msg->uint16();
        quint16 posy = msg->uint16();
        QPoint startPos(posx,posy);

        quint16 endposx = msg->uint16();
        quint16 endposy = msg->uint16();
        QPoint endPos(endposx,endposy);


        quint16 zoneX = msg->uint16();
        quint16 zoneY = msg->uint16();
        quint16 zoneW = msg->uint16();
        quint16 zoneH = msg->uint16();

        QRect zoneToRefresh(zoneX,zoneY,zoneW,zoneH);

        quint8 diameter = msg->uint8();
        quint8 colorType = msg->uint8();
        QColor color(msg->rgb());
        Map* map = findMapById(idMap);

        if(NULL!=map)
        {
            SelectedColor selectedColor;
            selectedColor.color = color;
            selectedColor.type = (ColorKind)colorType;

            map->paintOther(msg->action(),startPos,endPos,zoneToRefresh,diameter,selectedColor);
        }


    }
}
void MainWindow::extractCharacter(Map* map,NetworkMessageReader* msg)
{
    if(NULL!=map)
    {
        QString npcName = msg->string16();
        QString npcId = msg->string8();
        quint8 npcType = msg->uint8();
        quint8 npcNumber = msg->uint8();
        quint8 npcDiameter = msg->uint8();
        QColor npcColor(msg->rgb());
        quint16 npcXpos = msg->uint16();
        quint16 npcYpos = msg->uint16();

        qint16 npcXorient = msg->int16();
        qint16 npcYorient = msg->int16();

        QColor npcState(msg->rgb());
        QString npcStateName = msg->string16();
        quint16 npcStateNum = msg->uint16();

        quint8 npcVisible = msg->uint8();
        quint8 npcVisibleOrient = msg->uint8();

        QPoint orientation(npcXorient, npcYorient);

        QPoint npcPos(npcXpos, npcYpos);

        bool showNumber=(npcType == DessinPerso::pnj)?m_ui->m_showNpcNumberAction->isChecked():false;
        bool showName=(npcType == DessinPerso::pnj)? m_ui->m_showNpcNameAction->isChecked():m_ui->m_showPcNameAction->isChecked();

        DessinPerso* npc = new DessinPerso(map, npcId, npcName, npcColor, npcDiameter, npcPos, (DessinPerso::typePersonnage)npcType,showNumber,showName, npcNumber);

        if((npcVisible)||(npcType == DessinPerso::pnj && !m_preferences->value("isPlayer",false).toBool()))
        {
            npc->afficherPerso();
        }
        npc->nouvelleOrientation(orientation);
        if(npcVisibleOrient)
        {
            npc->afficheOuMasqueOrientation();
        }

        DessinPerso::etatDeSante health;
        health.couleurEtat = npcState;
        health.nomEtat = npcStateName;
        npc->nouvelEtatDeSante(health, npcStateNum);
        map->afficheOuMasquePnj(npc);

    }
}

void MainWindow::processCharacterMessage(NetworkMessageReader* msg)
{
    if(NetMsg::addCharacterList == msg->action())
    {
        QString idMap = msg->string8();
        quint16 characterNumber = msg->uint16();
        Map* map = findMapById(idMap);

        if(NULL!=map)
        {
            for(int i=0;i<characterNumber;++i)
            {
                extractCharacter(map,msg);
            }
        }
    }
    else if(NetMsg::moveCharacter == msg->action())
    {
        QString idMap = msg->string8();
        QString idCharacter = msg->string8();
        quint32 pointNumber = msg->uint32();


        QList<QPoint> moveList;
        QPoint point;
        for (int i=0; i<pointNumber; i++)
        {
            quint16 posX = msg->uint16();
            quint16 posY = msg->uint16();
            point = QPoint(posX, posY);
            moveList.append(point);
        }
        Map* map=findMapById(idMap);
        if(NULL!=map)
        {
            map->lancerDeplacementPersonnage(idCharacter,moveList);
        }
    }
    else if(NetMsg::changeCharacterState == msg->action())
    {
        QString idMap = msg->string8();
        QString idCharacter = msg->string8();
        quint16 stateNumber = msg->uint16();
        Map* map=findMapById(idMap);
        if(NULL!=map)
        {
            DessinPerso* character = map->trouverPersonnage(idCharacter);
            if(NULL!=character)
            {
                character->changerEtatDeSante(stateNumber);
            }
        }
    }
    else if(NetMsg::changeCharacterOrientation == msg->action())
    {
        QString idMap = msg->string8();
        QString idCharacter = msg->string8();
        qint16 posX = msg->int16();
        qint16 posY = msg->int16();
        QPoint orientation(posX, posY);
        Map* map=findMapById(idMap);
        if(NULL!=map)
        {
            DessinPerso* character = map->trouverPersonnage(idCharacter);
            if(NULL!=character)
            {
                character->nouvelleOrientation(orientation);
            }
        }
    }
    else if(NetMsg::showCharecterOrientation == msg->action())
    {
        QString idMap = msg->string8();
        QString idCharacter = msg->string8();
        quint8 showOrientation = msg->uint8();
        Map* map=findMapById(idMap);
        if(NULL!=map)
        {
            DessinPerso* character = map->trouverPersonnage(idCharacter);
            if(NULL!=character)
            {
                character->showOrientation(showOrientation);
            }
        }
    }
}
void MainWindow::processCharacterPlayerMessage(NetworkMessageReader* msg)
{
    if(msg->action() == NetMsg::ToggleViewPlayerCharacterAction)
    {
        QString idMap = msg->string8();
        QString idCharacter = msg->string8();
        quint8 display = msg->uint8();
        Map* map=findMapById(idMap);
        if(NULL!=map)
        {
            map->showPc(idCharacter,display);
        }
    }
    else if(msg->action() == NetMsg::ChangePlayerCharacterSizeAction)
    {
        /// @warning overweird
        QString idMap = msg->string8();
        QString idCharacter = msg->string8();
        quint8 size = msg->uint8();
        Map* map=findMapById(idMap);
        if(NULL!=map)
        {
            map->selectCharacter(idCharacter);
            map->changePjSize(size + 11);
        }
    }
}
CleverURI* MainWindow::contentToPath(CleverURI::ContentType type,bool save)
{
    QString filter;
    QString folder;
    QString title;
    switch(type)
    {
    case CleverURI::PICTURE:
        filter = m_supportedImage;
        title = tr("Open Picture");
        folder = m_preferences->value(QString("PicturesDirectory"),".").toString();
        break;
    case CleverURI::MAP:
        filter = m_supportedMap;
        title = tr("Open Map");
        folder = m_preferences->value(QString("MapsDirectory"),".").toString();
        break;
    case CleverURI::TEXT:
        filter = m_supportedNotes;
        title = tr("Open Minutes");
        folder = m_preferences->value(QString("MinutesDirectory"),".").toString();
        break;
    default:
        break;
    }
    if(!filter.isNull())
    {
        QString filepath;
        if(save)
            filepath= QFileDialog ::getSaveFileName(this,title,folder,filter);
        else
            filepath= QFileDialog::getOpenFileName(this,title,folder,filter);

        return new CleverURI(filepath,type);
    }
    return NULL;
}
void MainWindow::openContent()
{
    QAction* action=static_cast<QAction*>(sender());
    CleverURI::ContentType type;
    if(action == m_ui->m_openMapAction)
    {
        type=CleverURI::MAP;
    }
    else if(action == m_ui->m_openPictureAction)
    {
        type=CleverURI::PICTURE;

    }
    else if(action == m_ui->m_openStoryAction)
    {
        type = CleverURI::SCENARIO;
    }
    else if(action == m_ui->m_openNoteAction)
    {
        type = CleverURI::TEXT;
    }
    else if(action == m_ui->m_openOnlinePictureAction)
    {
        type = CleverURI::ONLINEPICTURE;
    }
#ifdef WITH_PDF
    else if(action == m_openPDFAct)
    {
        type = CleverURI::PDF;
    }
#endif
    else
    {
        return;
    }
    openContentFromType(type);
}
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        CleverURI uri = action->data().value<CleverURI>();
        openCleverURI(&uri);
    }
}
void MainWindow::updateRecentFileActions()
{
    QVariant var(CleverUriList);
    CleverUriList files = m_preferences->value("recentFileList",var).value<CleverUriList >();

    int numRecentFiles = qMin(files.size(), m_maxSizeRecentFile);

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(files[i].getShortName());
        m_recentFileActs[i]->setText(text);
        QVariant var;
        var.setValue(files[i]);
        m_recentFileActs[i]->setData(var);
        m_recentFileActs[i]->setIcon(QIcon(files[i].getIcon()));
        m_recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < m_recentFileActs.size() ; ++j)
    {
        m_recentFileActs[j]->setVisible(false);
    }

    m_ui->m_recentFileMenu->setEnabled(numRecentFiles > 0);
}
void MainWindow::setLatestFile(CleverURI* fileName)
{
    // no online picture because they are handled in a really different way.
    if((NULL!=fileName)&&(fileName->getType()!=CleverURI::ONLINEPICTURE))
    {
        QVariant var(CleverUriList());

        CleverUriList files = m_preferences->value("recentFileList",var).value<CleverUriList >();

        files.removeAll(*fileName);
        files.prepend(*fileName);
        while (files.size() > m_maxSizeRecentFile)
        {
            files.removeLast();
        }
        QVariant var3;
        var3.setValue(files);

        m_preferences->registerValue("recentFileList", var3,true);


        updateRecentFileActions();
    }
}
void MainWindow::openCleverURI(CleverURI* uri)
{
    MediaContainer* tmp=NULL;
    switch(uri->getType())
    {
    case CleverURI::MAP:
        tmp = new MapFrame();
        break;
    case CleverURI::PICTURE:
        tmp = new Image();
        break;
    case CleverURI::SCENARIO:
        break;
    default:
        break;
    }
    if(tmp!=NULL)
    {
        tmp->setCleverUri(uri);
        if(tmp->readFileFromUri())
        {
            if(uri->getType()==CleverURI::MAP)
            {
				prepareMap(static_cast<MapFrame*>(tmp));
            }
            else if(uri->getType()==CleverURI::PICTURE)
            {
				prepareImage(static_cast<Image*>(tmp));
            }
            addMediaToMdiArea(tmp);
            tmp->setVisible(true);
        }
    }
}
void MainWindow::openContentFromType(CleverURI::ContentType type)
{
    QString filter = CleverURI::getFilterForType(type);


    if(!filter.isEmpty())
    {
       QString folder = m_preferences->value(QString("ImageDirectory"),".").toString();
       QString title = tr("Open Picture");
       QStringList filepath = QFileDialog::getOpenFileNames(this,title,folder,filter);
       QStringList list = filepath;
       QStringList::Iterator it = list.begin();
       while(it != list.end())
       {

           openCleverURI(new CleverURI(*it,type));
           ++it;
       }

    }
    else
    {
		MediaContainer* tmp=NULL;
        switch(type)
        {
        case CleverURI::MAP:
            filter=CleverURI::getFilterForType(type);
            tmp = new MapFrame();
            break;
        case CleverURI::PICTURE:
        case CleverURI::ONLINEPICTURE:
            tmp = new Image();
            break;
        case CleverURI::SCENARIO:
            break;
        default:
            break;
        }

        if(tmp!=NULL)
        {
            tmp->setCleverUriType(type);
            if(tmp->openMedia())
            {
                if(tmp->readFileFromUri())
                {
                    if(type==CleverURI::MAP)
                    {
						prepareMap(static_cast<MapFrame*>(tmp));
                    }
                    else if((type==CleverURI::PICTURE)||(type==CleverURI::ONLINEPICTURE))
                    {
						prepareImage(static_cast<Image*>(tmp));
                    }
                    addMediaToMdiArea(tmp);
                    tmp->setVisible(true);
                }
            }
            //addToWorkspace(tmp);
        }
    }
}
void MainWindow::updateWindowTitle()
{
    setWindowTitle(tr("%1[*] - v%2 - %3 - %4 - %5").arg(m_preferences->value("applicationName","Rolisteam").toString())
                   .arg(m_version)
                   .arg(m_networkManager->isConnected() ? tr("Connected") : tr("Not Connected"))
                   .arg(m_networkManager->isServer() ? tr("Server") : tr("Client")).arg(m_playerList->localPlayer()->isGM() ? tr("GM") : tr("Player")));
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }

}
CleverURI::ContentType MainWindow::getContentType(QString str)
{
    QImage imag(str);
    if(str.endsWith(".pla"))
    {
        return CleverURI::MAP;
    }
    else if(!imag.isNull())
    {
        return CleverURI::PICTURE;
    }
    else if(str.endsWith(".m3u"))
    {
        return CleverURI::SONGLIST;
    }
    else if(str.endsWith(".sce"))
    {
        return CleverURI::SCENARIO;
    }
    else
    {
        QStringList list = m_preferences->value("AudioFileFilter","*.wav *.mp2 *.mp3 *.ogg *.flac").toString().split(' ');
        //QStringList list = audioFileFilter.split(' ');
        int i=0;
        while(i<list.size())
        {
            QString filter = list.at(i);
            filter.replace("*","");
            if(str.endsWith(filter))
            {
                return CleverURI::SONG;
            }
            ++i;
        }
    }
    return CleverURI::NONE;
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* data = event->mimeData();
    if(data->hasUrls())
    {
        QList<QUrl> list = data->urls();
        for(int i = 0; i< list.size();++i)
        {
            CleverURI::ContentType type= getContentType(list.at(i).toLocalFile());
            MediaContainer* tmp=NULL;
            switch(type)
            {
            case CleverURI::MAP:
                tmp = new MapFrame();
				prepareMap(static_cast<MapFrame*>(tmp));
                tmp->setCleverUri(new CleverURI(list.at(i).toLocalFile(),CleverURI::MAP));
                tmp->readFileFromUri();
                addMediaToMdiArea(tmp);
                tmp->setVisible(true);
                break;
            case CleverURI::PICTURE:
                tmp = new Image();
                tmp->setCleverUri(new CleverURI(list.at(i).toLocalFile(),CleverURI::PICTURE));
                tmp->readFileFromUri();
				prepareImage(static_cast<Image*>(tmp));
                addMediaToMdiArea(tmp);
                tmp->setVisible(true);
                break;
            case CleverURI::SONG:
#ifndef NULL_PLAYER
                m_audioPlayer->openSong(list.at(i).toLocalFile());
#endif
                break;
            case CleverURI::SONGLIST:
#ifndef NULL_PLAYER
                m_audioPlayer->openSongList(list.at(i).toLocalFile());
#endif
                break;
            }
        }
        event->acceptProposedAction();
    }
}

