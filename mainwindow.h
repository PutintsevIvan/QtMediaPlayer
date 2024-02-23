#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setPlaybackMode();
    void savePlaylist(QString filename);
    void loadPlaylist(QString filename);
    void loadFileToPlaylist(QString filename);
    void setTitles();
QString* loadPlaylistToArray(QString filename);

private slots:
    void on_duration_changed(qint64 duration);
    void on_position_changed(qint64 position);
    void on_pushButtonOpen_clicked();
    void on_horizontalSliderVolume_valueChanged(int value);
    void on_pushButtonPlay_clicked();
    void on_pushButtonPause_clicked();
    void on_HorizontalSliderProgress_sliderMoved(int position);
    void on_pushButtonMute_clicked();
    void on_checkBoxLoop_stateChanged(int arg1);
    void on_checkBoxShuffle_stateChanged(int arg1);


private:
    Ui::MainWindow *ui;
    QMediaPlayer* m_player;
    QMediaPlaylist* m_playlist;
    QStandardItemModel* m_playlist_model;

    const QString DEFAULT_PLAYLIST_LOCATION = "C:\\Users\\79283\\source\\Qt\\build-MediaPlayer-Desktop_Qt_5_12_12_MSVC2015_64bit-Debug";
};
#endif // MAINWINDOW_H
