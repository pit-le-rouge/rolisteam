#include "selectconnectionprofiledialog.h"
#include "ui_selectconnectionprofiledialog.h"

#include <QDebug>

/// ConnectionProfile
///
///
ConnectionProfile::ConnectionProfile()
{

}

ConnectionProfile::~ConnectionProfile()
{

}

void ConnectionProfile::setTitle(QString str)
{
    m_title = str;
}
void ConnectionProfile::setName(QString str)
{
    m_name = str;
}

void ConnectionProfile::setAddress(QString str)
{
    m_address = str;
}
void ConnectionProfile::setPort(int i)
{
    m_port = i;
}

void ConnectionProfile::setServerMode(bool b)
{
    m_server = b;
}

void ConnectionProfile::setPlayer(Player* p)
{
    m_player = p;
}

void ConnectionProfile::setGm(bool b)
{
    m_isGM = b;
}


QString ConnectionProfile::getTitle()const
{
    return m_title;
}
QString ConnectionProfile::getName() const
{
    return m_name;
}
QString ConnectionProfile::getAddress() const
{
    return m_address;
}
int ConnectionProfile::getPort() const
{
    return m_port;
}
bool ConnectionProfile::isServer() const
{
    return m_server;
}
Player* ConnectionProfile::getPlayer() const
{
    return m_player;
}
bool    ConnectionProfile::isGM() const
{
    return m_isGM;
}







////////////
//model
////////////
ProfileModel::ProfileModel()
{

}

ProfileModel::~ProfileModel()
{

}
QVariant ProfileModel::headerData(int section, Qt::Orientation orientation, int role)
{
    return QVariant();
}

int ProfileModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        return m_connectionProfileList.size();
    }
    else
    {
        return 0;
    }
}
QVariant ProfileModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(Qt::DisplayRole == role)
    {

        return m_connectionProfileList.at(index.row())->getTitle();
    }

    return QVariant();
}

void ProfileModel::appendProfile()
{
    beginInsertRows(QModelIndex(),m_connectionProfileList.size(),m_connectionProfileList.size());
    m_connectionProfileList.append(new ConnectionProfile());
    endInsertRows();
}
void ProfileModel::readSettings(QSettings & settings)
{
    settings.beginGroup("ConnectionProfiles");
    int size = settings.beginReadArray("ConnectionProfilesArray");
    for(int i = 0; i< size;++i)
    {
        settings.setArrayIndex(i);
        ConnectionProfile* profile = new ConnectionProfile();
        profile->setAddress(settings.value("address").toString());
        profile->setName( settings.value("name").toString());
        profile->setTitle( settings.value("title").toString());
        profile->setPort(settings.value("port").toInt());
        profile->setServerMode(settings.value("server").toBool());
        profile->setGm(settings.value("gm").toBool());
        QColor color = settings.value("PlayerColor").value<QColor>();
        Player* player = new Player(profile->getName(),color,profile->isGM());
        profile->setPlayer(player);

        beginInsertRows(QModelIndex(),m_connectionProfileList.size(),m_connectionProfileList.size());
        m_connectionProfileList.append(profile);
        endInsertRows();
    }
    settings.endArray();
    settings.endGroup();



    if(size==0)
    {
        ConnectionProfile* profile = new ConnectionProfile();
        profile->setName(tr("New Player"));
        profile->setTitle(tr("Default"));
        profile->setPort(6660);
        profile->setServerMode(true);
        QColor color = Qt::black;
        Player* player = new Player(profile->getName(),color,profile->isGM());
        profile->setPlayer(player);
        beginInsertRows(QModelIndex(),m_connectionProfileList.size(),m_connectionProfileList.size());
        m_connectionProfileList.append(profile);
        endInsertRows();
    }
}
ConnectionProfile* ProfileModel::getProfile(const QModelIndex& index)
{

    if((!m_connectionProfileList.isEmpty())&&(m_connectionProfileList.size()>index.row()))
    {
        return m_connectionProfileList.at(index.row());
    }
    return NULL;
}

void ProfileModel::writeSettings(QSettings & settings)
{
    settings.beginGroup("ConnectionProfiles");
    settings.beginWriteArray("ConnectionProfilesArray",m_connectionProfileList.size());
    for(int i = 0; i< m_connectionProfileList.size();++i)
    {
        settings.setArrayIndex(i);
        ConnectionProfile* profile = m_connectionProfileList.at(i);
        Player* player = profile->getPlayer();

        settings.setValue("address",profile->getAddress());
        settings.setValue("name",profile->getName());
        settings.setValue("title",profile->getTitle());
        settings.setValue("server",profile->isServer());
        settings.setValue("gm",profile->isGM());
        settings.setValue("PlayerColor",player->getColor());

    }
    settings.endArray();
    settings.endGroup();
}

Qt::ItemFlags ProfileModel::flags(const QModelIndex & index)
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

///SelectConnectionProfileDialog
///
///
///
///

SelectConnectionProfileDialog::SelectConnectionProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectConnectionProfileDialog),m_currentProfile(NULL)
{
    ui->setupUi(this);
    QSettings settings("rolisteam","rolisteam");

    m_model = new ProfileModel();
    readSettings(settings);
    ui->m_profileList->setModel(m_model);

    connect(ui->m_profileList,SIGNAL(clicked(QModelIndex)),this,SLOT(setCurrentProfile(QModelIndex)));
    connect(ui->m_profileList,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(setCurrentProfile(QModelIndex)));

    ui->m_profileList->setCurrentIndex(m_model->index(0, 0));
    setCurrentProfile(ui->m_profileList->currentIndex());


    connect(ui->m_addProfile,SIGNAL(clicked()),m_model,SLOT(appendProfile()));
    connect(ui->m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
    connect(ui->m_connect,SIGNAL(clicked()),this,SLOT(accept()));
}

SelectConnectionProfileDialog::~SelectConnectionProfileDialog()
{
    delete ui;
}

ConnectionProfile* SelectConnectionProfileDialog::getSelectedProfile()
{
    updateProfile();
    return m_currentProfile;
}
void SelectConnectionProfileDialog::setCurrentProfile(QModelIndex index)
{
    m_currentProfile = m_model->getProfile(index);
    updateGUI();
}
void SelectConnectionProfileDialog::updateGUI()
{
    if(NULL!=m_currentProfile)
    {
        ui->m_addresseLineEdit->setText(m_currentProfile->getAddress());
        ui->m_name->setText(m_currentProfile->getName());
        ui->m_profileTitle->setText(m_currentProfile->getTitle());
        ui->m_port->setValue(m_currentProfile->getPort());
        ui->m_isServerCheckbox->setChecked(m_currentProfile->isServer());
        ui->m_isGmCheckbox->setChecked(m_currentProfile->isGM());
        ui->m_colorBtn->setColor(m_currentProfile->getPlayer()->getColor());
    }
}

void SelectConnectionProfileDialog::updateProfile()
{
    if(NULL!=m_currentProfile)
    {
        m_currentProfile->setAddress(ui->m_addresseLineEdit->text());
        m_currentProfile->setName(ui->m_name->text());
        m_currentProfile->setPort(ui->m_port->value());
        m_currentProfile->setServerMode(ui->m_isServerCheckbox->isChecked());
        m_currentProfile->setTitle(ui->m_profileTitle->text());
        m_currentProfile->setGm(ui->m_isGmCheckbox->isChecked());
        Person* person = m_currentProfile->getPlayer();
        person->setColor(ui->m_colorBtn->color());
        person->setName(ui->m_name->text());
    }
}
void SelectConnectionProfileDialog::readSettings(QSettings & settings)
{
    m_model->readSettings(settings);
}

void SelectConnectionProfileDialog::writeSettings(QSettings & settings)
{
    m_model->writeSettings(settings);
}