#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStyle>
#include <QTime>
#include <QMultimedia>
#include <QMediaMetaData>
#include <QMediaTimeInterval>
#include <QMediaTimeRange>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Buttons style:
    ui->pushButtonOpen->setIcon(style()->standardIcon(QStyle::SP_DriveDVDIcon));
    ui->pushButtonPrev->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pushButtonPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->pushButtonStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->pushButtonNext->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));

    //Player init:
    m_player=new QMediaPlayer(this);
    m_player->setVolume(70);
    ui->labelVolume->setText(QString("Volume: ").append(QString::number(m_player->volume())));
    ui->horizontalSliderVolume->setValue(m_player->volume());



    //Connect
    connect(m_player,&QMediaPlayer::durationChanged,this,&MainWindow::on_duration_changed);
     connect(m_player,&QMediaPlayer::positionChanged,this,&MainWindow::on_position_changed);

    // Playlist init:
     m_playlist_model=new QStandardItemModel(this);
   this->ui->tableViewPlaylist->setModel(m_playlist_model);
     m_playlist_model->setHorizontalHeaderLabels(QStringList()<<"Audio track"<<"File path"<<"Duration");
     this->ui->tableViewPlaylist->hideColumn(1);
     //this->ui->tableViewPlaylist->horizontalHeader()->setStretchLastSection(true);
     int c_duration_width=64;
     this->ui->tableViewPlaylist->setColumnWidth(2,c_duration_width);
     this->ui->tableViewPlaylist->setColumnWidth(0,this->ui->tableViewPlaylist->width()-c_duration_width*1.7);
     this->ui->tableViewPlaylist->setEditTriggers(QAbstractItemView::NoEditTriggers);
     this->ui->tableViewPlaylist->setSelectionBehavior(QAbstractItemView::SelectRows);

     m_playlist=new QMediaPlaylist(m_player);
     m_player->setPlaylist(m_playlist);

     connect(this->ui->pushButtonPrev,&QPushButton::clicked,this->m_playlist,&QMediaPlaylist::previous);
     connect(this->ui->pushButtonNext,&QPushButton::clicked,this->m_playlist,&QMediaPlaylist::next);
     connect(this->m_playlist,&QMediaPlaylist::currentIndexChanged,this->ui->tableViewPlaylist,&QTableView::selectRow);
     connect(this->ui->tableViewPlaylist,&QTableView::doubleClicked,
             [this](const QModelIndex& index){m_playlist->setCurrentIndex(index.row());this->m_player->play();});
     connect(this->m_playlist,&QMediaPlaylist::currentIndexChanged,
             [this](int position){setTitles();m_player->play();});

     QString filename=DEFAULT_PLAYLIST_LOCATION + "playlist.m3u";
     loadPlaylist(filename);
}

MainWindow::~MainWindow()
{
    QString filename=DEFAULT_PLAYLIST_LOCATION + "playlist.m3u";
   // savePlaylist(filename);
    delete m_playlist_model;
    delete m_playlist;
    delete  m_player;
    delete ui;
}

void MainWindow::setPlaybackMode()
{
    QMediaPlaylist::PlaybackMode mode = QMediaPlaylist::PlaybackMode::CurrentItemOnce;
    if(this->ui->checkBoxLoop->isChecked())mode =QMediaPlaylist::PlaybackMode::Loop;
    if(this->ui->checkBoxShuffle->isChecked())mode =QMediaPlaylist::PlaybackMode::Random;
    this->m_playlist->setPlaybackMode(mode);
}

void MainWindow::savePlaylist(QString filename)
{
    QString format=filename.split('.').last();
    QUrl url = QUrl::fromLocalFile(filename);
    bool result=m_playlist->save(url,format.toStdString().c_str());
}

void MainWindow::loadPlaylist(QString filename)
{
 QString format=filename.split('.').last();
    m_playlist->load(QUrl::fromLocalFile(filename),format.toStdString().c_str());
    for(int i=0;i<m_playlist->mediaCount();i++)
    {
       QString url= m_playlist->media(i).canonicalUrl().url();
       QList<QStandardItem*>items;
       items.append(new QStandardItem(QDir(url).dirName()));
       items.append(new QStandardItem(url));
       m_playlist_model->appendRow(items);

    }
}

void MainWindow::loadFileToPlaylist(QString filename)
{
    filename=filename.remove('\n');
    m_playlist->addMedia(QUrl(filename));
    QList<QStandardItem*> items;
    items.append(new QStandardItem(QDir(filename).dirName()));
    items.append(new QStandardItem(filename));
   // items.append(new QStandardItem(duration));
    m_playlist_model->appendRow(items);
}

void MainWindow::setTitles()
{
    QString title=m_playlist->currentMedia().canonicalUrl().toString();
    this->setWindowTitle(title.split('/').last());
    this->ui->labelFile->setText(title);
}

QVector<QString> MainWindow::loadPlaylistToArray(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QList<QString> lines;
    while(!file.atEnd())
    {
        QByteArray line=file.readLine();
        lines.append(line);
    }
    return lines.toVector();
}

void MainWindow::on_duration_changed(qint64 duration)
{
    this->ui->HorizontalSliderProgress->setMaximum(duration);
  QTime q_time=  QTime::fromMSecsSinceStartOfDay(duration);
  ui->labelDuration->setText(QString("Duration").append(q_time.toString("hh:mm:ss")));
  QVariant data=m_player->metaData(QMediaMetaData::AudioBitRate);
  ui->labelBitrate->setText("bitrate: "+QString::number(m_player->metaData(QMediaMetaData::AudioBitRate).toString().toDouble()/1000).append("kbps"));
  data=m_player->metaData(QMediaMetaData::SampleRate);
  ui->labelSampleRate->setText("sample rate "+QString::number(data.toDouble()/1000)+" khz ");
}

void MainWindow::on_position_changed(qint64 position)
{
    QTime q_time=QTime::fromMSecsSinceStartOfDay(position);
    this->ui->labelProgress->setText(QString(q_time.toString("hh:mm:ss")));
    this->ui->HorizontalSliderProgress->setSliderPosition(position);

}


void MainWindow::on_pushButtonOpen_clicked()
{
 /*   QString file=QFileDialog::getOpenFileName
            (
                this,
                "Open file",
                NULL,
                "Audio files(*.mp3 *.flac);;MP-3(*.mp3);;(*.flac)"
                );
this->ui->labelFile->setText("File:"+file);
    this->setWindowTitle("MediaPlayerPD_321-"+file.split('/').last());
    this->m_player->setMedia(QUrl::fromLocalFile(file));
   // this->ui->labelDuration->setText(QString("Duration:").append(QString::number(m_player->duration())));*/

    QStringList files=QFileDialog::getOpenFileNames
            (
                this,
                "Open file",
                "C:\\Music",
                "Audio files (*.mp3 *.flac);; MP-3 (*.mp3);; Flac (*.flac);;Playlist (*.m3u *.CUE)"

            );
    for(QString filesPath:files)
    {
      /* QList<QStandardItem*> items;
       items.append(new QStandardItem(QDir(filesPath).dirName()));
       items.append(new QStandardItem(filesPath));
       m_playlist_model->appendRow(items);
       m_playlist->addMedia(QUrl(filesPath));*/
        this->loadFileToPlaylist(filesPath);

    }
}


void MainWindow::on_horizontalSliderVolume_valueChanged(int value)
{
    m_player->setVolume(value);
    ui->labelVolume->setText(QString("Volume :").append(QString::number(m_player->volume())));
}


void MainWindow::on_pushButtonPlay_clicked()
{
    m_player->play();
}

void MainWindow::on_pushButtonPause_clicked()
{
    m_player->pause();
}

void MainWindow::on_HorizontalSliderProgress_sliderMoved(int position)
{
    this->m_player->setPosition(position);
}


void MainWindow::on_pushButtonMute_clicked()
{
    this->m_player->setMuted(this->ui->pushButtonMute->isChecked());
    this->ui->pushButtonMute->setIcon(style()->standardIcon(
                                          this->ui->pushButtonMute->isChecked()?
                                              QStyle::SP_MediaVolumeMuted:
                                            QStyle::SP_MediaVolume));
}


void MainWindow::on_checkBoxLoop_stateChanged(int arg1)
{
    this->setPlaybackMode();
}


void MainWindow::on_checkBoxShuffle_stateChanged(int arg1)
{
     this->setPlaybackMode();
}


void MainWindow::on_pushButtonClr_clicked()
{
  m_playlist ->clear();
  m_playlist_model->clear();
}


void MainWindow::on_pushButtonDel_clicked()
{
  QItemSelectionModel* selection= ui->tableViewPlaylist->selectionModel();
  QModelIndexList indexes=selection->selectedRows();
  for(QModelIndex i:indexes)
  {
    m_playlist_model->removeRows(i.row(),1);
    m_playlist->removeMedia(i.row());
  }
}

